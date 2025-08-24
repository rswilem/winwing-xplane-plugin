#include "mcp-zibo-profile.h"
#include "dataref.h"
#include "config.h"

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#include <algorithm>
#include <string>
#include <vector>

// -----------------------------------------------------------
// Eligibility
// -----------------------------------------------------------

bool ZiboMcpProfile::IsEligible()
{
    // Heuristic: Zibo 737 exposes these laminar datarefs.
    // On évite XPLMGetNthAircraftModel() pour rester simple/fiable.
    auto *drm = Dataref::getInstance();
    return drm->exists("laminar/B738/autopilot/mcp_hdg_dial") &&
           drm->exists("laminar/B738/autopilot/mcp_alt_dial");
}

// -----------------------------------------------------------
// Initialize: build LED & Display bindings, resolve XPLM handles
// -----------------------------------------------------------

void ZiboMcpProfile::initialize()
{
    _ledBindings.clear();
    _ledDatarefs.clear();
    _displayBindings.clear();

    // ----------------------------
    // LED bindings (Zibo MCP LEDs)
    // ----------------------------
    // Règle:
    //  - GT0 : ON si value > 0
    //  - EQ1 : ON si value == 1
    //  - GE1 : ON si value >= 1   (ex: A/T arm)
    //
    // Mapping fourni :
    //   N1                    laminar/B738/autopilot/n1_status              -> N1
    //   SPEED                 laminar/B738/autopilot/speed_status1          -> SPEED
    //   VNAV                  laminar/B738/autopilot/vnav_status1           -> VNAV
    //   LVL CHG               laminar/B738/autopilot/lvl_chg_status         -> LVL_CHG
    //   HDG SEL               laminar/B738/autopilot/hdg_sel_status         -> HDG_SEL
    //   LNAV                  laminar/B738/autopilot/lnav_status            -> LNAV
    //   VOR LOC               laminar/B738/autopilot/vorloc_status          -> VOR_LOC
    //   APP                   laminar/B738/autopilot/app_status             -> APP
    //   ALT HLD               laminar/B738/autopilot/alt_hld_status         -> ALT_HLD
    //   VS                    laminar/B738/autopilot/vs_status              -> VS
    //   CMD A                 laminar/B738/autopilot/cmd_a_status           -> CMD_A
    //   CWS A                 laminar/B738/autopilot/cws_a_status           -> CWS_A
    //   CMD B                 laminar/B738/autopilot/cmd_b_status           -> CMD_B
    //   CWS B                 laminar/B738/autopilot/cws_b_status           -> CWS_B
    //   A/T arm               laminar/B738/autopilot/autothrottle_status    -> AT_ARM
    //   MA captain            laminar/B738/autopilot/master_capt_status     -> MA_CAPT
    //   MA first officer      laminar/B738/autopilot/master_fo_status       -> MA_FO
    const struct LedInit
    {
        const char *ref;
        McpLogicalLed logical;
        McpLedRule rule;
    } ledInitList[] = {
        {"laminar/B738/autopilot/n1_status1", McpLogicalLed::N1, McpLedRule::GT0},
        {"laminar/B738/autopilot/speed_status1", McpLogicalLed::SPEED, McpLedRule::GT0},
        {"laminar/B738/autopilot/vnav_status1", McpLogicalLed::VNAV, McpLedRule::GT0},
        {"laminar/B738/autopilot/lvl_chg_status", McpLogicalLed::LVL_CHG, McpLedRule::GT0},
        {"laminar/B738/autopilot/hdg_sel_status", McpLogicalLed::HDG_SEL, McpLedRule::GT0},
        {"laminar/B738/autopilot/lnav_status", McpLogicalLed::LNAV, McpLedRule::GT0},
        {"laminar/B738/autopilot/vorloc_status", McpLogicalLed::VOR_LOC, McpLedRule::GT0},
        {"laminar/B738/autopilot/app_status", McpLogicalLed::APP, McpLedRule::GT0},
        {"laminar/B738/autopilot/alt_hld_status", McpLogicalLed::ALT_HLD, McpLedRule::GT0},
        {"laminar/B738/autopilot/vs_status", McpLogicalLed::VS, McpLedRule::GT0},
        {"laminar/B738/autopilot/cmd_a_status", McpLogicalLed::CMD_A, McpLedRule::GT0},
        {"laminar/B738/autopilot/cws_a_status", McpLogicalLed::CWS_A, McpLedRule::GT0},
        {"laminar/B738/autopilot/cmd_b_status", McpLogicalLed::CMD_B, McpLedRule::GT0},
        {"laminar/B738/autopilot/cws_b_status", McpLogicalLed::CWS_B, McpLedRule::GT0},
        {"laminar/B738/autopilot/autothrottle_status1", McpLogicalLed::AT_ARM, McpLedRule::GT0},
        {"laminar/B738/autopilot/master_capt_status", McpLogicalLed::MA_CAPT, McpLedRule::GT0},
        {"laminar/B738/autopilot/master_fo_status", McpLogicalLed::MA_FO, McpLedRule::GT0},
    };

    _ledBindings.reserve(std::size(ledInitList));
    _ledDatarefs.reserve(std::size(ledInitList));

    for (const auto &it : ledInitList)
    {
        McpLedBinding b;
        b.datarefName = it.ref;
        b.logical = it.logical;
        b.rule = it.rule;
        b.handle = XPLMFindDataRef(it.ref); // utile si jamais on veut faire des reads directs
        _ledBindings.push_back(b);
        _ledDatarefs.push_back(it.ref);
    }

    // ---------------------------------
    // Display (MCP dials / 7-seg values)
    // ---------------------------------
    // AUTOPILOT DISPLAY (read):
    //  MCP SPD                  sim/cockpit2/autopilot/airspeed_dial_kts_mach (float)
    //  MCP SPD DIGIT 8          laminar/B738/mcp/digit_8                      (int)
    //  MCP SPD DIGIT A          laminar/B738/mcp/digit_A                      (int)
    //  MCP HDG                  laminar/B738/autopilot/mcp_hdg_dial           (int)
    //  MCP ALT                  laminar/B738/autopilot/mcp_alt_dial           (int)
    //  MCP VVI                  sim/cockpit2/autopilot/vvi_dial_fpm           (float)
    //  MCP CRS captain          laminar/B738/autopilot/course_pilot           (int)
    //  MCP CRS first officer    laminar/B738/autopilot/course_copilot         (int)
    const struct DispInit
    {
        const char *ref;
        McpDisplayItem item;
        McpRefType type;
    } dispInitList[] = {
        {"sim/cockpit2/autopilot/airspeed_dial_kts_mach", McpDisplayItem::SPD, McpRefType::Float},
        {"laminar/B738/mcp/digit_8", McpDisplayItem::SPD_DIGIT_8, McpRefType::Int},
        {"laminar/B738/mcp/digit_A", McpDisplayItem::SPD_DIGIT_A, McpRefType::Int},
        {"laminar/B738/autopilot/mcp_hdg_dial", McpDisplayItem::HDG, McpRefType::Int},
        {"laminar/B738/autopilot/mcp_alt_dial", McpDisplayItem::ALT, McpRefType::Int},
        {"sim/cockpit2/autopilot/vvi_dial_fpm", McpDisplayItem::VVI, McpRefType::Float},
        {"laminar/B738/autopilot/course_pilot", McpDisplayItem::CRS_CAPT, McpRefType::Int},
        {"laminar/B738/autopilot/course_copilot", McpDisplayItem::CRS_FO, McpRefType::Int}};

    _displayBindings.reserve(std::size(dispInitList));
    for (const auto &d : dispInitList)
    {
        McpDisplayBinding b;
        b.datarefName = d.ref;
        b.item = d.item;
        b.type = d.type;
        b.handle = XPLMFindDataRef(d.ref); // pour un éventuel accès direct
        _displayBindings.push_back(b);
    }

    // -------------------------------------------------
    // Button actions (press = execute Zibo command)
    // -------------------------------------------------
    _buttonActions.clear();

    // Offsets 0x01..0x06 per ton mapping
    auto BA = [&](uint8_t off, uint8_t mask, const char *name, const char *pressCmd)
    {
        McpButtonAction a{off, mask, name, pressCmd, nullptr};
        _buttonActions.push_back(a);
    };

    // 0x01
    BA(0x01, 0x01, "N1", "laminar/B738/autopilot/n1_press");
    BA(0x01, 0x02, "SPEED", "laminar/B738/autopilot/speed_press");
    BA(0x01, 0x04, "VNAV", "laminar/B738/autopilot/vnav_press");
    BA(0x01, 0x08, "LVL CHG", "laminar/B738/autopilot/lvl_chg_press");
    BA(0x01, 0x10, "HDG SEL", "laminar/B738/autopilot/hdg_sel_press");
    BA(0x01, 0x20, "LNAV", "laminar/B738/autopilot/lnav_press");
    BA(0x01, 0x40, "VOR LOC", "laminar/B738/autopilot/vorloc_press");
    BA(0x01, 0x80, "APP", "laminar/B738/autopilot/app_press");

    // 0x02
    BA(0x02, 0x01, "ALT HLD", "laminar/B738/autopilot/alt_hld_press");
    BA(0x02, 0x02, "V/S", "laminar/B738/autopilot/vs_press");
    BA(0x02, 0x04, "CMD A", "laminar/B738/autopilot/cmd_a_press");
    BA(0x02, 0x08, "CWS A", "laminar/B738/autopilot/cws_a_press");
    BA(0x02, 0x10, "CMD B", "laminar/B738/autopilot/cmd_b_press");
    BA(0x02, 0x20, "CWS B", "laminar/B738/autopilot/cws_b_press");
    BA(0x02, 0x40, "C/O", "laminar/B738/autopilot/co_press");
    BA(0x02, 0x80, "SPD INTV", "laminar/B738/autopilot/spd_intv_press");

    // 0x03
    BA(0x03, 0x01, "ALT INTV", "laminar/B738/autopilot/alt_intv_press");

    // 0x04 : CAPT F/D OFF, FO F/D OFF, DISENGAGE UP

    // 0x05 : BANK ANGLE

    // 0x06 : A/T ARMED/DISARMED

    // -------------------------------------------------
    // Encoder actions (delta -> inc/dec commands)
    // -------------------------------------------------
    _encoderActions.clear();

    auto EA = [&](uint8_t posOff, uint8_t cycOff, const char *name,
                  const char *incCmd, const char *decCmd, int stepPerTick = 1)
    {
        McpEncoderAction a{posOff, cycOff, name, incCmd, decCmd, stepPerTick};
        _encoderActions.push_back(a);
    };

    // CAPT CRS (0x15 pos, 0x16 cycle)
    EA(0x15, 0x16, "CRS CAPT",
       "laminar/B738/autopilot/course_pilot_up",
       "laminar/B738/autopilot/course_pilot_dn");

    // SPD (0x17 pos, 0x18 cycle): Zibo a "airspeed_up/_dn" (ou /mcp_dial_speed_up/dn selon version). Celui-ci marche bien :
    EA(0x17, 0x18, "SPD",
       "sim/autopilot/airspeed_up",
       "sim/autopilot/airspeed_down");

    // HDG (0x19 pos, 0x1A cycle)
    EA(0x19, 0x1A, "HDG",
       "laminar/B738/autopilot/heading_up",
       "laminar/B738/autopilot/heading_dn");

    // ALT (0x1B pos, 0x1C cycle)
    EA(0x1B, 0x1C, "ALT",
       "laminar/B738/autopilot/altitude_up",
       "laminar/B738/autopilot/altitude_dn");

    // V/S (0x1D pos, 0x1E cycle)
    EA(0x1D, 0x1E, "V/S",
       "sim/autopilot/vertical_speed_up",
       "sim/autopilot/vertical_speed_down");

    // FO CRS (0x1F pos) (pas de cycle fourni dans ton dump, on met 0x00)
    EA(0x1F, 0x00, "CRS FO",
       "laminar/B738/autopilot/course_copilot_up",
       "laminar/B738/autopilot/course_copilot_dn");

    // -------------------------------------------------
    // Switch actions (écriture directe de datarefs INT)
    // -------------------------------------------------
    _switchActions.clear();

    // BANK ANGLE : sim/cockpit2/autopilot/bank_angle_mode = 2..6 (10..30)
    // _switchActions.push_back({0x05, 0x02, "BANK 10", "sim/cockpit2/autopilot/bank_angle_mode", 2});
    // _switchActions.push_back({0x05, 0x04, "BANK 15", "sim/cockpit2/autopilot/bank_angle_mode", 3});
    // _switchActions.push_back({0x05, 0x08, "BANK 20", "sim/cockpit2/autopilot/bank_angle_mode", 4});
    // _switchActions.push_back({0x05, 0x10, "BANK 25", "sim/cockpit2/autopilot/bank_angle_mode", 5});
    // _switchActions.push_back({0x05, 0x20, "BANK 30", "sim/cockpit2/autopilot/bank_angle_mode", 6});

    // DISENGAGE : laminar/B738/switches/autopilot/ap_disconnect
    //   DOWN (offset 0x05, mask 0x01) -> 1 (disengaged)
    _switchActions.push_back({0x04, 0x80, "AP DISENGAGE UP", "laminar/B738/switches/autopilot/ap_disconnect", 2, 1});

    // A/T ARM : laminar/B738/switches/autopilot/at_arm  (0/1)
    //   offset 0x06: 0x01=ARMED -> 1,  0x02=DISARMED -> 0
    _switchActions.push_back({0x06, 0x01, "A/T ARMED", "laminar/B738/switches/autopilot/at_arm", 1, 0});

    // F/D OFF : laminar/B738/switches/autopilot/fd_ca / fd_fo (0=OFF)
    //   offset 0x04: 0x08=FD CAPT OFF,  0x20=FD FO OFF
    _switchActions.push_back({0x04, 0x08, "FD CAPT OFF", "laminar/B738/switches/autopilot/fd_ca", 0, 1});
    _switchActions.push_back({0x04, 0x20, "FD FO OFF", "laminar/B738/switches/autopilot/fd_fo", 0, 1});
}

const std::vector<McpSwitchAction> &ZiboMcpProfile::switchActions() const
{
    static const std::vector<McpSwitchAction> kEmpty;
    return kEmpty;
}

void ZiboMcpProfile::startAutopilotPowerMonitor(std::function<void(bool)> onChange)
{
    // Zibo/XP: use sim/cockpit2/... as power truth
    const char *kAPPower = "sim/cockpit2/autopilot/autopilot_has_power";
    auto *drm = Dataref::getInstance();

    if (!drm->exists(kAPPower))
        return;

    // Monitor and notify product (product will handle PAP3 solenoid frames)
    drm->monitorExistingDataref<int>(kAPPower, [onChange](int v)
                                     {
        if (onChange) onChange(v != 0); });
}

bool ZiboMcpProfile::isAutothrottleDisarmedAtStartup()
{
    const char *kAT = "laminar/B738/autopilot/autothrottle_status1";
    auto *drm = Dataref::getInstance();
    if (!drm->exists(kAT))
        return false;
    // Zibo: 0 = disarmed
    return drm->get<int>(kAT) == 0;
}

void ZiboMcpProfile::registerSwitchPositionMonitors(std::function<const uint8_t *()> getHwState)
{
    _getHwState = std::move(getHwState);

    auto *drm = Dataref::getInstance();
    _swPosMonRefs.clear();

    struct PosRef
    {
        const char *posRef;    // read-only *_pos dataref
        const char *toggleCmd; // command that flips the position
        enum Kind
        {
            Bin_AT,
            Bin_FD_CAPT,
            Bin_FD_FO,
            Enum_Bank,
            Enum_APDisc12
        } kind;
    } refs[] = {
        // A/T: special – only toggle to DISARM if sim shows ARMED and HW shows DISARMED
        {"laminar/B738/autopilot/autothrottle_arm_pos", "laminar/B738/autopilot/autothrottle_arm_toggle", PosRef::Bin_AT},

        // FD Capt/Fo: off-bit on PAP3 means desired=0; else desired=1. If sim!=desired => toggle
        {"laminar/B738/autopilot/flight_director_pos", "laminar/B738/autopilot/flight_director_toggle", PosRef::Bin_FD_CAPT},
        {"laminar/B738/autopilot/flight_director_fo_pos", "laminar/B738/autopilot/flight_director_fo_toggle", PosRef::Bin_FD_FO},

        // Bank angle: pos 0..4 – we nudge using Zibo helper (no direct writes)
        {"laminar/B738/autopilot/bank_angle_pos", nullptr, PosRef::Enum_Bank},

        // AP disconnect: pos 0/1; PAP3 bit 0x05:0x01 “DISENGAGE OFF” => desired 1 else 0; if sim!=desired => toggle
        {"laminar/B738/autopilot/disconnect_pos", "laminar/B738/autopilot/disconnect_toggle", PosRef::Enum_APDisc12},
    };

    for (const auto &r : refs)
    {
        _swPosMonRefs.emplace_back(r.posRef);

        drm->monitorExistingDataref<int>(
            r.posRef,
            [this, r](int simPos)
            {
                if (!_getHwState)
                    return;
                const uint8_t *hw = _getHwState();
                if (!hw)
                    return;

                switch (r.kind)
                {
                case PosRef::Bin_AT:
                {
                    // PAP3 0x06: 0x01=ARMED, 0x02=DISARMED
                    const bool hwArmed = (hw[0x06] & 0x01) != 0;
                    const bool hwDis = (hw[0x06] & 0x02) != 0;
                    int desired = simPos;
                    if (hwArmed)
                        desired = 1;
                    else if (hwDis)
                        desired = 0;

                    // Only force DISARM (1->0) via toggle; ARM transitions are handled by solenoid logic.
                    if (desired == 0 && simPos == 1 && r.toggleCmd && *r.toggleCmd)
                    {
                        Dataref::getInstance()->executeCommand(r.toggleCmd);
                        debug_force("[PAP3][Zibo] AT ARM: sim=%d -> DISARM via %s\n", simPos, r.toggleCmd);
                    }
                    break;
                }
                case PosRef::Bin_FD_CAPT:
                {
                    // PAP3 0x04 bit 0x08 = "FD CAPT OFF": desired=0 if set, else 1.
                    const bool offBit = (hw[0x04] & 0x08) != 0;
                    const int desired = offBit ? 1 : 0;
                    if (desired != simPos && r.toggleCmd && *r.toggleCmd)
                    {
                        Dataref::getInstance()->executeCommand(r.toggleCmd);
                        debug_force("[PAP3][Zibo] FD CAPT: sim=%d -> %d via %s\n", simPos, desired, r.toggleCmd);
                    }
                    break;
                }
                case PosRef::Bin_FD_FO:
                {
                    // PAP3 0x04 bit 0x20 = "FD FO OFF": desired=0 if set, else 1.
                    const bool offBit = (hw[0x04] & 0x20) != 0;
                    const int desired = offBit ? 1 : 0;
                    if (desired != simPos && r.toggleCmd && *r.toggleCmd)
                    {
                        Dataref::getInstance()->executeCommand(r.toggleCmd);
                        debug_force("[PAP3][Zibo] FD FO: sim=%d -> %d via %s\n", simPos, desired, r.toggleCmd);
                    }
                    break;
                }
                case PosRef::Enum_APDisc12:
                {
                    // PAP3 0x05 bit 0x01 = "DISENGAGE OFF". desired=1 if set, else 0. Position ref is 0/1.
                    const bool offBit = (hw[0x05] & 0x01) != 0;
                    const int desired = offBit ? 1 : 0;
                    if (desired != simPos && r.toggleCmd && *r.toggleCmd)
                    {
                        Dataref::getInstance()->executeCommand(r.toggleCmd);
                        debug_force("[PAP3][Zibo] AP DISC: sim=%d -> %d via %s\n", simPos, desired, r.toggleCmd);
                    }
                    break;
                }
                case PosRef::Enum_Bank:
                {
                    int desired = -1;
                    const uint8_t b = hw[0x05];
                    if (b & 0x02)
                        desired = 0; // 10°
                    else if (b & 0x04)
                        desired = 1; // 15°
                    else if (b & 0x08)
                        desired = 2; // 20°
                    else if (b & 0x10)
                        desired = 3; // 25°
                    else if (b & 0x20)
                        desired = 4; // 30°
                    if (desired >= 0 && desired != simPos)
                    {
                        this->nudgeBankAngleTo(desired);
                        debug_force("[PAP3][Zibo] BANK: sim=%d -> %d via nudges\n", simPos, desired);
                    }
                    break;
                }
                }
            });
    }

    for (const auto &s : _swPosMonRefs)
        drm->exists(s.c_str());
    debug_force("[PAP3][Zibo] Registered %zu switch position monitors\n", _swPosMonRefs.size());
}

int ZiboMcpProfile::desiredBankFromHw(const uint8_t *hw) const
{
    if (!hw)
        return -1;
    const uint8_t b = hw[0x05];
    if (b & 0x02)
        return 0; // 10°
    if (b & 0x04)
        return 1; // 15°
    if (b & 0x08)
        return 2; // 20°
    if (b & 0x10)
        return 3; // 25°
    if (b & 0x20)
        return 4; // 30°
    return -1;
}

void ZiboMcpProfile::unregisterSwitchPositionMonitors()
{
    auto *drm = Dataref::getInstance();
    for (const auto &ref : _switchPosMonitoredRefs)
        drm->unbind(ref.c_str());
    _switchPosMonitoredRefs.clear();
    _getHwSnapshot = nullptr;
}

void ZiboMcpProfile::nudgeBankAngleTo(int targetPos)
{
    targetPos = std::clamp(targetPos, 0, 4);
    auto *drm = Dataref::getInstance();

    // Zibo : 0..4 (10,15,20,25,30)
    // pos RO
    static const char *kPosRef = "laminar/B738/autopilot/bank_angle_pos";
    // commandes +/- (une impulsion = un cran)
    static const char *kUpCmd = "laminar/B738/autopilot/bank_angle_up";
    static const char *kDnCmd = "laminar/B738/autopilot/bank_angle_dn";

    if (!drm->exists(kPosRef))
        return; // avion pas prêt
    int cur = drm->get<int>(kPosRef);

    if (cur == targetPos)
        return;

    int delta = targetPos - cur;
    const char *cmd = (delta > 0) ? kUpCmd : kDnCmd;
    int steps = std::abs(delta);
    steps = std::min(steps, 10); // sécurité

    for (int i = 0; i < steps; ++i)
    {
        drm->executeCommand(cmd);
    }
}

void ZiboMcpProfile::enforceSwitchPositionsFromHardware(const uint8_t *now, const uint8_t *prev)
{
    auto *drm = Dataref::getInstance();
    if (!now)
        return;

    auto exists = [&](const char *dr)
    { return drm->exists(dr); };
    auto geti = [&](const char *dr, int def = 0)
    { return exists(dr) ? drm->get<int>(dr) : def; };

    // Helpers to read current & previous bits safely
    auto bitNow = [&](uint8_t off, uint8_t mask) -> bool
    {
        return (now[off] & mask) != 0;
    };
    auto bitChanged = [&](uint8_t off, uint8_t mask) -> bool
    {
        if (!prev)
            return true; // first packet -> treat as changed once
        return ((now[off] ^ prev[off]) & mask) != 0;
    };

    // --- A/T ARM (pos: 0/1, toggle flips) ---
    // HW bits on 0x06: 0x01 = ARMED, 0x02 = DISARMED
    // Previously we only toggled to DISARM; now toggle both directions so PAP3 truly masters.
    {
        const char *posRef = "laminar/B738/autopilot/autothrottle_arm_pos";
        const char *togCmd = "laminar/B738/autopilot/autothrottle_arm_toggle";
        if (drm->exists(posRef))
        {
            const int simPos = drm->get<int>(posRef);
            const bool hwArmed = bitNow(0x06, 0x01);
            const bool hwDis = bitNow(0x06, 0x02);

            int desired = simPos;
            if (hwArmed)
                desired = 1;
            else if (hwDis)
                desired = 0;

            // Only act when one of the HW bits actually changed
            const bool changed = bitChanged(0x06, 0x01) || bitChanged(0x06, 0x02);
            if (changed && desired != simPos)
            {
                drm->executeCommand(togCmd);
                debug_force("AT ARM: sim=%d -> %d via %s", simPos, desired, togCmd);
            }
        }
    }

    // FD CAPT (pos 0/1, toggle flips)
    {
        const char *posRef = "laminar/B738/autopilot/flight_director_pos";
        const char *togCmd = "laminar/B738/autopilot/flight_director_toggle";
        if (drm->exists(posRef))
        {
            const int simPos = drm->get<int>(posRef);
            const bool onBit = bitNow(0x04, 0x08);
            const int desired = onBit ? 1 : 0; // bit = ON
            if (bitChanged(0x04, 0x08) && desired != simPos)
            {
                drm->executeCommand(togCmd);
                debug_force("FD CAPT: sim=%d -> %d via %s", simPos, desired, togCmd);
            }
        }
    }

    // FD FO (pos 0/1, toggle flips)
    {
        const char *posRef = "laminar/B738/autopilot/flight_director_fo_pos";
        const char *togCmd = "laminar/B738/autopilot/flight_director_fo_toggle";
        if (drm->exists(posRef))
        {
            const int simPos = drm->get<int>(posRef);
            const bool onBit = bitNow(0x04, 0x20);
            const int desired = onBit ? 1 : 0; // bit = ON
            if (bitChanged(0x04, 0x20) && desired != simPos)
            {
                drm->executeCommand(togCmd);
                debug_force("FD FO: sim=%d -> %d via %s", simPos, desired, togCmd);
            }
        }
    }

    // --- AP DISCONNECT (pos 1=disengaged, 2=engaged, toggle flips) ---
    // HW: 0x05/0x01 = DOWN (want 1), 0x04/0x80 = UP (want 2).
    // Debounce: only react on a change of those bits; if both changed, prefer the bit that is SET now.
    {
        const char *posRef = "laminar/B738/autopilot/disconnect_pos";
        const char *togCmd = "laminar/B738/autopilot/disconnect_toggle";
        if (drm->exists(posRef))
        {
            const int simPos = drm->get<int>(posRef); // 1=diseng, 2=engaged
            const bool onBit = bitNow(0x05, 0x01);    // Bouton enfoncé = DISENGAGED = 1
            const int desired = onBit ? 1 : 0;        // bit = ON
            if (bitChanged(0x05, 0x01) && desired != simPos)
            {
                drm->executeCommand(togCmd);
                debug_force("DISC: sim=%d -> %d via %s", simPos, desired, togCmd);
            }
        }
    }

    // --- BANK ANGLE (pos 0..4, use nudges; act only on change of bank bits) ---
    {
        const char *posRef = "laminar/B738/autopilot/bank_angle_pos";
        if (drm->exists(posRef))
        {
            const int simPos = drm->get<int>(posRef);

            const uint8_t maskAll = 0x02 | 0x04 | 0x08 | 0x10 | 0x20;
            const bool anyChg = bitChanged(0x05, maskAll);
            if (anyChg)
            {
                int desired = -1;
                const uint8_t b = now[0x05];
                if (b & 0x02)
                    desired = 0; // 10°
                else if (b & 0x04)
                    desired = 1; // 15°
                else if (b & 0x08)
                    desired = 2; // 20°
                else if (b & 0x10)
                    desired = 3; // 25°
                else if (b & 0x20)
                    desired = 4; // 30°

                if (desired >= 0 && desired != simPos)
                {
                    this->nudgeBankAngleTo(desired);
                    debug_force("[PAP3][Zibo] BANK: sim=%d -> %d via nudges\n", simPos, desired);
                }
            }
        }
    }
}
