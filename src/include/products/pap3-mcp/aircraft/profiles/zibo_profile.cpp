// src/include/products/pap3-mcp/aircraft/zibo_profile.cpp
#include "zibo_profile.h"
#include "../../device/pap3_device.h" 
#include <cmath>
#include <algorithm>
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

namespace pap3::aircraft {

static inline XPLMCommandRef Cmd(const char* s) { return XPLMFindCommand(s); }

static inline bool dr_is_on(XPLMDataRef r) {
    if (!r) return false;
    const XPLMDataTypeID ty = XPLMGetDataRefTypes(r);
    // Prefer native type when available; fall back sanely.
    if (ty & xplmType_Int)   return XPLMGetDatai(r) != 0;
    if (ty & xplmType_Float) return XPLMGetDataf(r) > 0.5f;
    if (ty & xplmType_Double)return XPLMGetDatad(r) > 0.5;
    // Last resort: try int
    return XPLMGetDatai(r) != 0;
}

ZiboPAP3Profile::ZiboPAP3Profile() {
    // MCP values
    _drSpd     = XPLMFindDataRef("laminar/B738/autopilot/mcp_speed_dial_kts_mach");
    _drHdg     = XPLMFindDataRef("laminar/B738/autopilot/mcp_hdg_dial");
    _drAlt     = XPLMFindDataRef("laminar/B738/autopilot/mcp_alt_dial");
    _drVvi     = XPLMFindDataRef("sim/cockpit2/autopilot/vvi_dial_fpm");
    _drVviShow = XPLMFindDataRef("laminar/B738/autopilot/vvi_dial_show");
    _drCrsCapt = XPLMFindDataRef("laminar/B738/autopilot/course_pilot");
    _drCrsFo   = XPLMFindDataRef("laminar/B738/autopilot/course_copilot");
    _drMcpBrightnessArr = XPLMFindDataRef("sim/cockpit2/electrical/instrument_brightness_ratio_manual");
    _drCockpitLightsArr = XPLMFindDataRef("laminar/B738/electric/panel_brightness");
    

    // MCP LCD "special digits"
    _drDigitA = XPLMFindDataRef("laminar/B738/mcp/digit_A");
    _drDigitB = XPLMFindDataRef("laminar/B738/mcp/digit_8");

    // LEDs
    _drLedN1     = XPLMFindDataRef("laminar/B738/autopilot/n1_status1");
    _drLedSpd    = XPLMFindDataRef("laminar/B738/autopilot/speed_status1");
    _drLedVnav   = XPLMFindDataRef("laminar/B738/autopilot/vnav_status1");
    _drLedLvlChg = XPLMFindDataRef("laminar/B738/autopilot/lvl_chg_status");
    _drLedHdgSel = XPLMFindDataRef("laminar/B738/autopilot/hdg_sel_status");
    _drLedLnav   = XPLMFindDataRef("laminar/B738/autopilot/lnav_status");
    _drLedVorLoc = XPLMFindDataRef("laminar/B738/autopilot/vorloc_status");
    _drLedApp    = XPLMFindDataRef("laminar/B738/autopilot/app_status");
    _drLedAltHld = XPLMFindDataRef("laminar/B738/autopilot/alt_hld_status");
    _drLedVs     = XPLMFindDataRef("laminar/B738/autopilot/vs_status");
    _drLedCmdA   = XPLMFindDataRef("laminar/B738/autopilot/cmd_a_status");
    _drLedCwsA   = XPLMFindDataRef("laminar/B738/autopilot/cws_a_status");
    _drLedCmdB   = XPLMFindDataRef("laminar/B738/autopilot/cmd_b_status");
    _drLedCwsB   = XPLMFindDataRef("laminar/B738/autopilot/cws_b_status");
    _drLedAtArm  = XPLMFindDataRef("laminar/B738/autopilot/autothrottle_status1");
    _drLedMaCapt = XPLMFindDataRef("laminar/B738/autopilot/master_capt_status");
    _drLedMaFo   = XPLMFindDataRef("laminar/B738/autopilot/master_fo_status");

    // --- Switch position refs ---
    _drFDCaptPos = XPLMFindDataRef("laminar/B738/autopilot/flight_director_pos");
    _drFDFoPos   = XPLMFindDataRef("laminar/B738/autopilot/flight_director_fo_pos");
    _drATArmPos  = XPLMFindDataRef("laminar/B738/autopilot/autothrottle_arm_pos");
    _drApDiscPos = XPLMFindDataRef("laminar/B738/autopilot/disconnect_pos");

    // --- Toggle commands ---
    _cmdFDCaptToggle = Cmd("laminar/B738/autopilot/flight_director_toggle");
    _cmdFDFoToggle   = Cmd("laminar/B738/autopilot/flight_director_fo_toggle");
    _cmdATArmToggle  = Cmd("laminar/B738/autopilot/autothrottle_arm_toggle");
    _cmdApDiscToggle = Cmd("laminar/B738/autopilot/disconnect_toggle");

    // Power-related refs
    _drHasApPower = XPLMFindDataRef("sim/cockpit2/autopilot/autopilot_has_power");
    _drDcBus1     = XPLMFindDataRef("laminar/B738/electric/dc_bus1_status");
    _drDcBus2     = XPLMFindDataRef("laminar/B738/electric/dc_bus2_status");

    // --- Bank angle (Zibo) ---
    _drBankIdx = XPLMFindDataRef("laminar/B738/autopilot/bank_angle_pos");
    if (!_drBankIdx) _drBankIdx = XPLMFindDataRef("laminar/B738/autopilot/bank_angle_sel"); // fallback ancien Zibo

    _cmdBankUp = Cmd("laminar/B738/autopilot/bank_angle_up");
    _cmdBankDn = Cmd("laminar/B738/autopilot/bank_angle_dn");

    // Button bindings (press-only, unless noted). Offsets/masks per device map.
    auto B = [&](uint8_t off, uint8_t mask, const char* press, const char* release=nullptr){
        _btns.push_back({off, mask, Cmd(press), release ? Cmd(release) : nullptr});
    };

    // 0x01
    B(0x01, 0x01, "laminar/B738/autopilot/n1_press");
    B(0x01, 0x02, "laminar/B738/autopilot/speed_press");
    B(0x01, 0x04, "laminar/B738/autopilot/vnav_press");
    B(0x01, 0x08, "laminar/B738/autopilot/lvl_chg_press");
    B(0x01, 0x10, "laminar/B738/autopilot/hdg_sel_press");
    B(0x01, 0x20, "laminar/B738/autopilot/lnav_press");
    B(0x01, 0x40, "laminar/B738/autopilot/vorloc_press");
    B(0x01, 0x80, "laminar/B738/autopilot/app_press");

    // 0x02
    B(0x02, 0x01, "laminar/B738/autopilot/alt_hld_press");
    B(0x02, 0x02, "laminar/B738/autopilot/vs_press");
    B(0x02, 0x04, "laminar/B738/autopilot/cmd_a_press");
    B(0x02, 0x08, "laminar/B738/autopilot/cws_a_press");
    B(0x02, 0x10, "laminar/B738/autopilot/cmd_b_press");
    B(0x02, 0x20, "laminar/B738/autopilot/cws_b_press");
    B(0x02, 0x40, "laminar/B738/autopilot/change_over_press");
    B(0x02, 0x80, "laminar/B738/autopilot/spd_interv");

    // 0x03
    B(0x03, 0x01, "laminar/B738/autopilot/alt_interv");
    // The rest of 0x03/0x04/0x05 are handled as encoders or dedicated logic below if needed.

    // Encoder bindings
    auto E = [&](uint8_t posOff, const char* inc, const char* dec, int step=1){
        _encs.push_back({posOff, Cmd(inc), Cmd(dec), step});
    };

    // CRS CAPT
    E(0x15, "laminar/B738/autopilot/course_pilot_up",
              "laminar/B738/autopilot/course_pilot_dn");
    // SPD 
    E(0x17, "sim/autopilot/airspeed_up",
              "sim/autopilot/airspeed_down");
    // HDG
    E(0x19, "laminar/B738/autopilot/heading_up",
              "laminar/B738/autopilot/heading_dn");
    // ALT
    E(0x1B, "laminar/B738/autopilot/altitude_up",
              "laminar/B738/autopilot/altitude_dn");
    // V/S
    E(0x1D, "sim/autopilot/vertical_speed_up",
              "sim/autopilot/vertical_speed_down");
    // CRS FO
    E(0x1F, "laminar/B738/autopilot/course_copilot_up",
              "laminar/B738/autopilot/course_copilot_dn");
}

ZiboPAP3Profile::~ZiboPAP3Profile() { stop(); }

bool ZiboPAP3Profile::isEligible() const {
    return _drSpd != nullptr;
}

void ZiboPAP3Profile::start(StateCallback onChanged) {
    if (_running) return;
    _cb = std::move(onChanged);
    _running = true;
}

void ZiboPAP3Profile::stop() {
    if (!_running) return;
    _running = false;
}

pap3::aircraft::State ZiboPAP3Profile::current() const { return _state; }

void ZiboPAP3Profile::tick() {
    if (!_running) return;
    poll();
}

void ZiboPAP3Profile::poll() {
    if (_drSpd)     _state.spd     = XPLMGetDataf(_drSpd);
    if (_drHdg)     _state.hdg     = XPLMGetDatai(_drHdg);
    if (_drAlt)     _state.alt     = XPLMGetDatai(_drAlt);
    if (_drVvi)     _state.vvi     = (XPLMGetDataf(_drVvi));
    if (_drVviShow) _state.vviVisible = XPLMGetDataf(_drVviShow) > 0.5f;
    if (_drCrsCapt) _state.crsCapt = XPLMGetDatai(_drCrsCapt);
    if (_drCrsFo)   _state.crsFo   = XPLMGetDatai(_drCrsFo);
    if (_drMcpBrightnessArr) {
    float v = 0.0f;
        XPLMGetDatavf(_drMcpBrightnessArr, &v, 15, 1);
        _state.mcpBrightness = std::clamp(v, 0.0f, 1.0f);
    }

    if (_drCockpitLightsArr) {
    float v = 0.0f;
        XPLMGetDatavf(_drCockpitLightsArr, &v, 0, 1);
        _state.cockpitLights = std::clamp(v, 0.0f, 1.0f);
        _state.ledsBrightness = std::max(v, 0.60f); // minimum 60% for LEDs
    }
    // LCD special digits (A / 8)
    if (_drDigitA) _state.digitA = XPLMGetDataf(_drDigitA) > 0.5f;
    if (_drDigitB) _state.digitB = XPLMGetDataf(_drDigitB) > 0.5f;

    _state.led.N1      = (_drLedN1     && XPLMGetDataf(_drLedN1)     > 0.5f);
    _state.led.SPEED   = (_drLedSpd    && XPLMGetDataf(_drLedSpd)    > 0.5f);
    _state.led.VNAV    = (_drLedVnav   && XPLMGetDataf(_drLedVnav)   > 0.5f);
    _state.led.LVL_CHG = (_drLedLvlChg && XPLMGetDataf(_drLedLvlChg) > 0.5f);
    _state.led.HDG_SEL = (_drLedHdgSel && XPLMGetDataf(_drLedHdgSel) > 0.5f);
    _state.led.LNAV    = (_drLedLnav   && XPLMGetDataf(_drLedLnav)   > 0.5f);
    _state.led.VORLOC  = (_drLedVorLoc && XPLMGetDataf(_drLedVorLoc) > 0.5f);
    _state.led.APP     = (_drLedApp    && XPLMGetDataf(_drLedApp)    > 0.5f);
    _state.led.ALT_HLD = (_drLedAltHld && XPLMGetDataf(_drLedAltHld) > 0.5f);
    _state.led.V_S     = (_drLedVs     && XPLMGetDataf(_drLedVs)     > 0.5f);
    _state.led.CMD_A   = (_drLedCmdA   && XPLMGetDataf(_drLedCmdA)   > 0.5f);
    _state.led.CWS_A   = (_drLedCwsA   && XPLMGetDataf(_drLedCwsA)   > 0.5f);
    _state.led.CMD_B   = (_drLedCmdB   && XPLMGetDataf(_drLedCmdB)   > 0.5f);
    _state.led.CWS_B   = (_drLedCwsB   && XPLMGetDataf(_drLedCwsB)   > 0.5f);
    _state.led.AT_ARM  = (_drLedAtArm  && XPLMGetDataf(_drLedAtArm)  > 0.5f);
    _state.led.MA_CAPT = (_drLedMaCapt && XPLMGetDataf(_drLedMaCapt) > 0.5f);
    _state.led.MA_FO   = (_drLedMaFo   && XPLMGetDataf(_drLedMaFo)   > 0.5f);

    if (_cb) _cb(_state);

    #if PAP3_PRIORITY_LOCK
        // Enforce maintained switches to follow hardware when priority is enabled.
        // Only toggle when sim position differs from desired hardware position.

        if (_haveHwSnapshot) { 
        // FD CAPT
        maybeToggle(_drFDCaptPos, _hwFDCaptOn, _cmdFDCaptToggle);

        // FD FO
        maybeToggle(_drFDFoPos, _hwFDFoOn, _cmdFDFoToggle);

        // A/T ARM
        // Debounce is not strictly required here because we only toggle on mismatch.
        // If you still want to limit rate, you can reuse debounce(_lastAtToggleTime).
        //maybeToggle(_drATArmPos, _hwATOn, _cmdATArmToggle);
        if (_drATArmPos) {
        const int simAT = XPLMGetDataf(_drATArmPos);   // 0 = DISARMED, 1 = ARMED

        // If simulator shows ARMED and hardware is ARMED at that moment,
        // pulse the solenoid (non-blocking OFF->ON after ~50ms).
        // Debounce avoids pulsing every tick.
        if (!simAT && _hwATOn) {
            _device->pulseATSolenoid(100);
        } else {
            // Otherwise, enforce hardware-priority on the sim switch.
            maybeToggle(_drATArmPos, _hwATOn, _cmdATArmToggle);
        }
    }

        // A/P DISCONNECT latch (ENGAGED vs DISENGAGED as captured in _hwApDiscEngaged)
        maybeToggle(_drApDiscPos, _hwApDiscEngaged, _cmdApDiscToggle);
}
    #endif
}

// --- Input hooks implementation ---------------------------------------------

void ZiboPAP3Profile::execOnce(XPLMCommandRef cmd) {
    if (!cmd) return;
    XPLMCommandOnce(cmd);
}

void ZiboPAP3Profile::repeatCmd(XPLMCommandRef inc, XPLMCommandRef dec, int8_t delta, int stepPerTick)
{
    if (delta == 0) return;
    const bool up = (delta > 0);
    const int reps = std::max(1, static_cast<int>(std::abs(static_cast<int>(delta))) * std::max(1, stepPerTick));
    XPLMCommandRef cmd = up ? inc : dec;
    if (!cmd) return;
    for (int i = 0; i < reps; ++i) XPLMCommandOnce(cmd);
}

bool ZiboPAP3Profile::debounce(float& lastTs, float minDeltaSec) const {
    const float now = XPLMGetElapsedTime();
    if (now - lastTs < minDeltaSec) return false;
    lastTs = now;
    return true;
}

void ZiboPAP3Profile::maybeToggle(XPLMDataRef posRef, bool desiredOn, XPLMCommandRef toggleCmd)
{
    // Compare desired hardware state to current sim state and toggle only if needed.
    if (!posRef || !toggleCmd) return;
    const bool simOn = dr_is_on(posRef);
    if (simOn != desiredOn) XPLMCommandOnce(toggleCmd);
}

void ZiboPAP3Profile::onButton(uint8_t off, uint8_t mask, bool pressed)
{
    // Maintained switches first

    // --- BANK ANGLE: 0x05 / {0x02,0x04,0x08,0x10,0x20} ---
    if (off == 0x05 && pressed) {
        int target = -1;
        switch (mask) {
            case 0x02: target = 0; break; // 10°
            case 0x04: target = 1; break; // 15°
            case 0x08: target = 2; break; // 20°
            case 0x10: target = 3; break; // 25°
            case 0x20: target = 4; break; // 30°
            default: break;
        }
        if (target >= 0) {
            nudgeBankAngleTo(target);
            return; // ne pas retomber dans le mapping boutons générique
        }
    }


    // FD CAPT: 0x04 / 0x08  ("FD CAPT OFF" line). Pressed == switch ON, Released == switch OFF.
    if (off == 0x04 && mask == 0x08) {
        _hwFDCaptOn = pressed;
        _haveHwSnapshot = true;
        maybeToggle(_drFDCaptPos, _hwFDCaptOn, _cmdFDCaptToggle);
        return;
    }

    // FD FO: 0x04 / 0x20 ("FD FO OFF" line). Pressed == switch ON, Released == switch OFF.
    if (off == 0x04 && mask == 0x20) {
        _hwFDFoOn = pressed;
        _haveHwSnapshot = true;
        maybeToggle(_drFDFoPos, _hwFDFoOn, _cmdFDFoToggle);
        return;
    }

    // A/T ARM: 0x06 / 0x01 (ARMED) and 0x06 / 0x02 (DISARMED).
    // ON state when ARMED=pressed or DISARMED=released.
    if (off == 0x06 && (mask == 0x01 || mask == 0x02)) {
        if (mask == 0x01) {            // ARMED line
            _hwATOn = pressed;         // pressed -> ON, released -> OFF
            _haveHwSnapshot = true;
        } else if (mask == 0x02) {     // DISARMED line
            _hwATOn = !pressed;        // pressed -> OFF, released -> ON
        }
        if (debounce(_lastAtToggleTime)) {
            maybeToggle(_drATArmPos, _hwATOn, _cmdATArmToggle);
        }
        return;
    }

    // A/P DISENGAGE two lines:
    //  - 0x04 / 0x80 : UP (ENGAGED) line
    //  - 0x05 / 0x01 : DOWN (DISENGAGED) line
    // ENGAGED when UP=pressed or DOWN=released.
    if ((off == 0x04 && mask == 0x80) || (off == 0x05 && mask == 0x01)) {
        if (off == 0x04 && mask == 0x80) {   // UP line
            _hwApDiscEngaged = !pressed;      // pressed -> DISENGAGED, released -> ENGAGED
        } else {                              // DOWN line
            _hwApDiscEngaged = pressed;     // pressed -> ENGAGED, released -> DISENGAGED
            _haveHwSnapshot = true;
        }
        if (debounce(_lastApDiscToggleTime)) {
            maybeToggle(_drApDiscPos, _hwApDiscEngaged, _cmdApDiscToggle);
        }
        return;
    }

    // All other momentary buttons use the existing mapping table
    for (const auto& b : _btns) {
        if (b.off == off && b.mask == mask) {
            execOnce(pressed ? b.press : b.release);
            return;
        }
    }
}

void ZiboPAP3Profile::onEncoderDelta(uint8_t posOff, int8_t delta) {
    for (const auto& e : _encs) {
        if (e.posOff == posOff) {
            repeatCmd(e.inc, e.dec, delta, e.stepPerTick);
            return;
        }
    }
}
uint8_t ZiboPAP3Profile::mcpPowerMask() const {
    uint8_t mask = 0;

    if (dr_is_on(_drHasApPower)) mask |= 0x01; // bit0 = autopilot has power
    if (dr_is_on(_drDcBus1))     mask |= 0x02; // bit1 = DC bus 1 alive
    if (dr_is_on(_drDcBus2))     mask |= 0x04; // bit2 = DC bus 2 alive

    return mask;
}

bool ZiboPAP3Profile::mcpHasPower() const {
    return (mcpPowerMask() & 0x01) != 0;
}

void ZiboPAP3Profile::syncSimToHardware() {
    if (!_haveHwSnapshot) return;  // <--- garde-fou

    maybeToggle(_drFDCaptPos, _hwFDCaptOn, _cmdFDCaptToggle);
    maybeToggle(_drFDFoPos,   _hwFDFoOn,   _cmdFDFoToggle);
    maybeToggle(_drApDiscPos, _hwApDiscEngaged, _cmdApDiscToggle);

    const bool havePower = (dr_is_on(_drHasApPower) && (dr_is_on(_drDcBus1) || dr_is_on(_drDcBus2)));
    if (havePower) {
        maybeToggle(_drATArmPos, _hwATOn, _cmdATArmToggle);
    }
}

static inline bool bit(const uint8_t* r, int len, uint8_t off, uint8_t mask) {
    return (off < (uint8_t)len) && ((r[off] & mask) != 0);
}

void ZiboPAP3Profile::syncSimToHardwareFromRaw(const std::uint8_t* r, int len)
{
    if (!r || len <= 0) return;

    // --- FD CAPT / FO ---
    // Lignes "OFF" (1 => OFF). ON <=> (OFF == 0)
    const bool hwFDCaptOn = !bit(r, len, 0x04, 0x08); // FD CAPT OFF
    const bool hwFDFoOn   = !bit(r, len, 0x04, 0x20); // FD FO   OFF
    maybeToggle(_drFDCaptPos, hwFDCaptOn, _cmdFDCaptToggle);
    maybeToggle(_drFDFoPos,   hwFDFoOn,   _cmdFDFoToggle);
    _hwFDCaptOn = hwFDCaptOn; // seed internes si tu les utilises
    _hwFDFoOn   = hwFDFoOn;

    // --- A/T ARM ---
    // 0x06/0x01 = "ARMED"; 0x06/0x02 = "DISARMED"
    // ON <=> (ARMED == 1) || (DISARMED == 0)
    const bool atArmed     =  bit(r, len, 0x06, 0x01);
    const bool atDisarmed  =  bit(r, len, 0x06, 0x02);
    const bool hwATOn      =  atArmed || !atDisarmed;

    // ---  BANK ANGLE snapshot ---
    auto bit = [](const std::uint8_t* rr, int l, std::uint8_t off, std::uint8_t mask){
        return (rr && l > off) && ((rr[off] & mask) != 0);
    };

    int targetBank = -1;
    if      (bit(r, len, 0x05, 0x02)) targetBank = 0; // 10°
    else if (bit(r, len, 0x05, 0x04)) targetBank = 1; // 15°
    else if (bit(r, len, 0x05, 0x08)) targetBank = 2; // 20°
    else if (bit(r, len, 0x05, 0x10)) targetBank = 3; // 25°
    else if (bit(r, len, 0x05, 0x20)) targetBank = 4; // 30°

    if (targetBank >= 0) {
        nudgeBankAngleTo(targetBank);
    }

    const bool havePower = (dr_is_on(_drHasApPower) &&
                           (dr_is_on(_drDcBus1) || dr_is_on(_drDcBus2)));
    if (havePower) {
        maybeToggle(_drATArmPos, hwATOn, _cmdATArmToggle);
        // Si SIM déjà ARMÉ et HW=ON au boot, pulse une fois proprement
        if (dr_is_on(_drATArmPos) && hwATOn && _device) {
            _device->pulseATSolenoid(100);
        }
    }
    _hwATOn = hwATOn;

    // --- A/P DISENGAGE latch ---
    // 0x04/0x80 = UP(ENGAGED), 0x05/0x01 = DOWN(DISENGAGED)
    // ENGAGED <=> (UP==1) || (DOWN==0)
    const bool upEng        = bit(r, len, 0x04, 0x80);
    const bool downDiseng   = bit(r, len, 0x05, 0x01);
    const bool hwApEngaged  = upEng || !downDiseng;
    maybeToggle(_drApDiscPos, hwApEngaged, _cmdApDiscToggle);
    _hwApDiscEngaged = hwApEngaged;

}

int ZiboPAP3Profile::readBankIndex() const
{
    if (!_drBankIdx) return -1;
    const XPLMDataTypeID ty = XPLMGetDataRefTypes(_drBankIdx);
    if (ty & xplmType_Int)    return std::clamp(XPLMGetDatai(_drBankIdx), 0, 4);
    if (ty & xplmType_Float)  return std::clamp((int)std::lround(XPLMGetDataf(_drBankIdx)), 0, 4);
    if (ty & xplmType_Double) return std::clamp((int)std::lround(XPLMGetDatad(_drBankIdx)), 0, 4);
    return -1;
}

void ZiboPAP3Profile::nudgeBankAngleTo(int target)
{
    if (target < 0 || target > 4) return;
    if (!_cmdBankUp && !_cmdBankDn) return;

    int cur = readBankIndex();
    if (cur < 0) return;

    // Chemin direct (sans wrap). Si un jour Zibo wrappe circulairement, on pourra optimiser.
    const int maxSteps = 10; // garde-fou anti-boucle
    int steps = 0;

    while (cur != target && steps++ < maxSteps)
    {
        const bool up = (target > cur);
        XPLMCommandRef cmd = up ? _cmdBankUp : _cmdBankDn;
        if (!cmd) break;

        XPLMCommandOnce(cmd);

        // Relis la position; Zibo met à jour très vite.
        int next = readBankIndex();
        if (next == cur) {
            // si pas de progression, insiste une fois
            XPLMCommandOnce(cmd);
            next = readBankIndex();
        }
        cur = next;
    }
}

} // namespace pap3::aircraft
