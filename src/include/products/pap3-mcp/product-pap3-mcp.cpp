#include "product-pap3-mcp.h"
#include "dataref.h"
#include "appstate.h"
#include "config.h"

// Aircraft profiles
#include "profiles/mcp-zibo-profile.h"

#include <XPLMMenus.h>
#include <XPLMUtilities.h>
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <utility>
#include <vector>
#include <cmath>
#include <climits>

// -------- Logging helper (X-Plane dev console + Log.txt) --------
#ifndef PAP3_LOG
#define PAP3_LOG 0
#endif

static XPLMMenuID g_pap3MenuId = nullptr;
static int g_pap3ItemIndex = -1;
static int g_pap3LabelsIndex = -1;
static bool g_showLabels = false;
extern XPLMMenuID mainMenuId;

static std::string getPap3ConfigPath()
{
    char path[512];
    XPLMGetSystemPath(path);
    std::string base(path);
    return base + "Output/preferences/winwing_pap3.ini";
}

void ProductPAP3MCP::initializeMenu()
{
    g_showLabels = false; // valeur par défaut

    // Charger depuis le fichier ini
    std::ifstream f(getPap3ConfigPath());
    if (f.is_open())
    {
        std::string line;
        while (std::getline(f, line))
        {
            if (line.find("show_labels=1") != std::string::npos)
            {
                g_showLabels = true;
                break;
            }
        }
        f.close();
    }
}

bool ProductPAP3MCP::getShowLcdLabels()
{
    return g_showLabels;
}

void ProductPAP3MCP::toggleShowLcdLabels()
{
    g_showLabels = !g_showLabels;
    if (g_pap3MenuId != nullptr && g_pap3LabelsIndex >= 0)
    {
        XPLMCheckMenuItem(g_pap3MenuId, g_pap3LabelsIndex,
                          g_showLabels ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    }
    // Sauvegarder l'état dans l'ini
    std::ofstream f(getPap3ConfigPath(), std::ios::trunc);
    if (f.is_open())
    {
        f << "show_labels=" << (g_showLabels ? "1" : "0") << "\n";
        f.close();
    }
}

void ProductPAP3MCP::updateMenuPresence()
{
    createSubmenuIfNeeded();
}

void ProductPAP3MCP::createSubmenuIfNeeded()
{
    if (g_pap3MenuId != nullptr)
        return;

    g_pap3ItemIndex = XPLMAppendMenuItem(mainMenuId, "PAP3", nullptr, 0);
    g_pap3MenuId = XPLMCreateMenu("PAP3", mainMenuId, g_pap3ItemIndex,
                                  pap3MenuCallback, nullptr);

    g_pap3LabelsIndex = XPLMAppendMenuItem(g_pap3MenuId, "Display LCD Labels",
                                           (void *)"toggle_labels", 0);

    XPLMCheckMenuItem(g_pap3MenuId, g_pap3LabelsIndex,
                      g_showLabels ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}

void ProductPAP3MCP::pap3MenuCallback(void * /*mRef*/, void *iRef)
{
    const char *action = reinterpret_cast<const char *>(iRef);
    if (!action)
        return;

    if (strcmp(action, "toggle_labels") == 0)
    {
        toggleShowLcdLabels();
    }
}

static void PAP3_LogF(const char *fmt, ...)
{
#if PAP3_LOG
    char buf[1024];
    int off = std::snprintf(buf, sizeof(buf), "[winwing][PAP3] ");
    if (off < 0)
        off = 0;
    if (off >= (int)sizeof(buf))
        off = (int)sizeof(buf) - 1;

    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf + off, sizeof(buf) - off, fmt, ap);
    va_end(ap);

    size_t len = std::strlen(buf);
    if (len + 1 < sizeof(buf) && (len == 0 || buf[len - 1] != '\n'))
    {
        buf[len] = '\n';
        buf[len + 1] = '\0';
    }
    XPLMDebugString(buf);
#else
    (void)fmt;
#endif
}
#define LOGP(...) PAP3_LogF(__VA_ARGS__)

static void dump_hex(const char *prefix, const std::vector<uint8_t> &data)
{
#if PAP3_LOG
    char line[1024];
    int n = std::snprintf(line, sizeof(line), "%s", prefix ? prefix : "");
    for (size_t i = 0; i < data.size() && n < (int)sizeof(line) - 4; ++i)
    {
        n += std::snprintf(line + n, sizeof(line) - n, "%02X ", data[i]);
    }
    if (n < (int)sizeof(line) - 1)
        line[n++] = '\n';
    line[n] = '\0';
    XPLMDebugString(line);
#else
    (void)prefix;
    (void)data;
#endif
}

// -------------------------------------------------------------
// Flight loop to drive Dataref::update() every frame (FCU-style)
// -------------------------------------------------------------
static bool g_pap3DatarefFLRegistered = false;

static float PAP3_DatarefUpdateFL(float, float, int, void *)
{
    Dataref::getInstance()->update(); // triggers monitors ASAP
    return -1.0f;                     // run again next frame
}

// -------------------------------------------------------------
// LCD helpers
// -------------------------------------------------------------

static inline void pad64(std::vector<uint8_t> &b)
{
    if (b.size() < 64)
        b.resize(64, 0x00);
}

// Header for 0x38 where payload MUST start at absolute offset 0x19.
// This array is bytes 0..0x18 (25 bytes). We will push payload bytes starting at 0x19.
static const uint8_t HEAD_38_PREFIX[] = {
    0xF0, 0x00, 0x00 /*seq*/, 0x38, 0x0F, 0xBF, 0x00, 0x00,
    0x02, 0x01, 0x00, 0x00, 0xDF, 0xA2, 0x50, 0x00,
    0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00};

// -------------------------------------------------------------
// Lifecycle
// -------------------------------------------------------------

ProductPAP3MCP::ProductPAP3MCP(HIDDeviceHandle hidDevice,
                               uint16_t vendorId,
                               uint16_t productId,
                               std::string vendorName,
                               std::string productName)
    : USBDevice(hidDevice, vendorId, productId, std::move(vendorName), std::move(productName))
{
    profile = nullptr;
    lastUpdateCycle = 0;
    pressedButtonIndices.clear();
    selectorPositions.clear();

    LOGP("Ctor (VID=0x%04X PID=0x%04X)", vendorId, productId);
    connect();
}

ProductPAP3MCP::~ProductPAP3MCP()
{
    LOGP("Dtor");
    disconnect();
}

const char *ProductPAP3MCP::classIdentifier()
{
    return "Product-PAP3-MCP";
}

bool ProductPAP3MCP::connect()
{
    LOGP("connect() called");
    if (!USBDevice::connect())
    {
        LOGP("USBDevice::connect() FAILED");
        return false;
    }

    setProfileForCurrentAircraft();
    monitorDatarefs();

    // LED monitors + initial sync
    registerLedMonitors();

    // MCP display monitors (read-only); we’ll push LCDs on change
    registerDisplayMonitors();

    // Switch position monitors (PAP3 is master for switches)
    if (profile)
    {
        static_cast<McpAircraftProfile *>(profile)->registerSwitchPositionMonitors(
            [this]() -> const uint8_t *
            { return _inp.hasLast ? _inp.last : nullptr; });
    }

    // --- A/P Power check for solénoïde A/T ---
    {
        if (profile)
        {
            static_cast<McpAircraftProfile *>(profile)->startAutopilotPowerMonitor(
                [this](bool hasPwr) { /* envoie trames OFF/ON solenoid comme tu le fais déjà */ });
        }
    }

    // LCD init (does not affect LEDs)
    initializeDisplays();

    // Register a per-frame input poller so HID inputs are read instantly
    if (!_inputFLRegistered)
    {
        XPLMRegisterFlightLoopCallback(ProductPAP3MCP::PAP3_InputPollFL, -1.0f, this);
        _inputFLRegistered = true;
        LOGP("Input FlightLoop registered (per-frame HID polling)");
    }

    // --- Align AT solenoid with sim state at startup ---
    if (profile && static_cast<McpAircraftProfile *>(profile)->isAutothrottleDisarmedAtStartup())
    {
        pulseATSolenoid();
    }

    LOGP("Connected: VID=0x%04X PID=0x%04X (%s %s)",
         vendorId, productId, vendorName.c_str(), productName.c_str());

    // Register the flightloop once for immediate monitor notifications
    if (!g_pap3DatarefFLRegistered)
    {
        XPLMRegisterFlightLoopCallback(PAP3_DatarefUpdateFL, -1.0f, nullptr);
        g_pap3DatarefFLRegistered = true;
        LOGP("FlightLoop registered for Dataref::update() @ every frame");
    }
    return true;
}

void ProductPAP3MCP::disconnect()
{
    LOGP("disconnect() called");

    unregisterLedMonitors();
    unregisterDisplayMonitors();

    if (profile)
    {
        static_cast<McpAircraftProfile *>(profile)->unregisterSwitchPositionMonitors();
        LOGP("Deleting profile: %s", profile->name());
        delete profile;
        profile = nullptr;
    }

    if (g_pap3DatarefFLRegistered)
    {
        XPLMUnregisterFlightLoopCallback(PAP3_DatarefUpdateFL, nullptr);
        g_pap3DatarefFLRegistered = false;
        LOGP("FlightLoop unregistered");
    }

    if (_inputFLRegistered)
    {
        XPLMUnregisterFlightLoopCallback(ProductPAP3MCP::PAP3_InputPollFL, this);
        _inputFLRegistered = false;
        LOGP("Input FlightLoop unregistered");
    }

    // Cancel pending one-shot solenoid callback bound to this instance
    XPLMUnregisterFlightLoopCallback(ProductPAP3MCP::PAP3_ATSolenoidOnFL, this);
    XPLMUnregisterFlightLoopCallback(ProductPAP3MCP::PAP3_ReevalVSFL, this);

    USBDevice::disconnect();
    LOGP("Disconnected");
}

void ProductPAP3MCP::update()
{
    if (!connected)
        return;

    USBDevice::update(); // pump HID I/O

    // LEDs are fully event-driven now; no polling
    updateDisplays(); // still a no-op for now
}

void ProductPAP3MCP::didReceiveData(int /*reportId*/, uint8_t *report, int reportLength)
{
    if (!connected || !report || reportLength <= 0)
        return;

    // On ne traite que les paquets d’INPUT (header 0x01).
    // Tout ce qui commence par 0xF0 etc. = nos trames LCD → on ignore.
    if (report[0] != kInputHeader)
        return;

    // Clamp à la taille attendue (0x20). Beaucoup de devices envoient plus large.
    const int n = std::min(reportLength, kInputBytes);

    // Buffer local normalisé (0x20 bytes)
    uint8_t in[kInputBytes] = {0};
    std::memcpy(in, report, n);

    handleInputReport(in, kInputBytes);
}

// -------------------------------------------------------------
// PAP3 Inputs: buttons (flags) and encoders (rolling positions)
// -------------------------------------------------------------

struct BtnDef
{
    uint8_t off;
    uint8_t mask;
    const char *name;
};

// Offsets 0x01..0x06 based on the mapping provided
static const BtnDef kBtnDefs[] = {
    // 0x01
    {0x01, 0x01, "N1"},
    {0x01, 0x02, "SPEED"},
    {0x01, 0x04, "VNAV"},
    {0x01, 0x08, "LVL CHG"},
    {0x01, 0x10, "HDG SEL"},
    {0x01, 0x20, "LNAV"},
    {0x01, 0x40, "VOR LOC"},
    {0x01, 0x80, "APP"},
    // 0x02
    {0x02, 0x01, "ALT HLD"},
    {0x02, 0x02, "V/S"},
    {0x02, 0x04, "CMD A"},
    {0x02, 0x08, "CWS A"},
    {0x02, 0x10, "CMD B"},
    {0x02, 0x20, "CWS B"},
    {0x02, 0x40, "IAS/MACH TOGGLE"},
    {0x02, 0x80, "SPD INTV"},
    // 0x03
    {0x03, 0x01, "ALT INTV"},
    {0x03, 0x02, "CRS CAPT -"},
    {0x03, 0x04, "CRS CAPT +"},
    {0x03, 0x08, "SPD -"},
    {0x03, 0x10, "SPD +"},
    {0x03, 0x20, "HDG -"},
    {0x03, 0x40, "HDG +"},
    {0x03, 0x80, "ALT -"},
    // 0x04
    {0x04, 0x01, "ALT +"},
    {0x04, 0x02, "CRS FO -"},
    {0x04, 0x04, "CRS FO +"},
    {0x04, 0x08, "FD CAPT OFF"},
    {0x04, 0x20, "FD FO OFF"},
    {0x04, 0x80, "AP DISENGAGE UP"},
    // 0x05
    {0x05, 0x01, "AP DISENGAGE DOWN"},
    {0x05, 0x02, "BANK 10"},
    {0x05, 0x04, "BANK 15"},
    {0x05, 0x08, "BANK 20"},
    {0x05, 0x10, "BANK 25"},
    {0x05, 0x20, "BANK 30"},
    {0x05, 0x40, "V/S -"},
    {0x05, 0x80, "V/S +"},
    // 0x06
    {0x06, 0x01, "A/T ARMED"},
    {0x06, 0x02, "A/T DISARMED"},
};

struct EncDef
{
    uint8_t posOff;   // rolling position offset
    const char *name; // for logging / mapping
};

// Encoders: use the "POSITION (rolling)" offsets
static const EncDef kEncDefs[] = {
    {0x15, "CRS CAPT"},
    {0x17, "SPD"},
    {0x19, "HDG"},
    {0x1B, "ALT"},
    {0x1D, "V/S"},
    {0x1F, "CRS FO"},
};

static inline uint8_t byteAt(const uint8_t *r, int len, uint8_t off)
{
    return (off < (uint8_t)len) ? r[off] : 0;
}

void ProductPAP3MCP::handleInputReport(const uint8_t *report, int len)
{

    if (!report || len != kInputBytes)
        return;

    const uint8_t *prev = _inp.hasLast ? _inp.last : nullptr;

    if (profile)
        static_cast<McpAircraftProfile *>(profile)->enforceSwitchPositionsFromHardware(report, prev);

    decodeButtons(report, len, prev);
    decodeEncoders(report, len, prev);
    // decodeSwitches(report, len); // still disabled: we rely on *_pos + toggles

    std::memcpy(_inp.last, report, kInputBytes);
    _inp.hasLast = true;
}

void ProductPAP3MCP::decodeButtons(const uint8_t *r, int len, const uint8_t *prev)
{
    if (len != kInputBytes)
        return;

    // Premier paquet → seed silencieux (pas de spam)
    if (!prev)
        return;

    for (const auto &d : kBtnDefs)
    {
        const uint8_t now = (d.off < len) ? r[d.off] : 0;
        const uint8_t was = prev[d.off];
        const uint8_t chg = uint8_t(now ^ was);
        if (!(chg & d.mask))
            continue;

        const bool pressed = (now & d.mask) != 0;
        LOGP("BTN %s: %s", d.name, pressed ? "DOWN" : "UP");

        // --- BANK ANGLE: Zibo Specific
        if (pressed && d.off == 0x05)
        {
            int target = -1;
            switch (d.mask)
            {
            case 0x02:
                target = 0;
                break; // 10°
            case 0x04:
                target = 1;
                break; // 15°
            case 0x08:
                target = 2;
                break; // 20°
            case 0x10:
                target = 3;
                break; // 25°
            case 0x20:
                target = 4;
                break; // 30°
            default:
                break;
            }
            if (target >= 0 && profile)
            {
                if (auto *z = dynamic_cast<ZiboMcpProfile *>(profile))
                {
                    z->nudgeBankAngleTo(target);
                    return;
                }
            }
        }

        if (!profile)
            continue;
        const auto &actions = static_cast<McpAircraftProfile *>(profile)->buttonActions();
        for (const auto &a : actions)
        {
            if (a.offset != d.off || a.mask != d.mask)
                continue;
            if (pressed)
            {
                if (a.pressCmd && *a.pressCmd)
                    Dataref::getInstance()->executeCommand(a.pressCmd);
            }
            else
            {
                if (a.releaseCmd && *a.releaseCmd)
                    Dataref::getInstance()->executeCommand(a.releaseCmd);
            }
            break;
        }
    }
}

void ProductPAP3MCP::decodeSwitches(const uint8_t *r, int len)
{
    if (len != kInputBytes || !profile)
        return;

    auto *drm = Dataref::getInstance();
    const auto &sw = static_cast<McpAircraftProfile *>(profile)->switchActions();
    if (sw.empty())
        return;

    for (const auto &s : sw)
    {
        // Read current bit state
        const uint8_t now = (s.off < len) ? r[s.off] : 0;
        const bool bitSet = (now & s.mask) != 0;

        // Compute desired value based on bit state
        const int desired = bitSet ? s.valueIfSet : s.valueIfClr;

        // Skip if dataref missing
        if (!drm->exists(s.datarefName))
            continue;

        // Anti-spam: only write if changed
        int current = drm->get<int>(s.datarefName);
        if (current != desired)
        {
            drm->set<int>(s.datarefName, desired);
            LOGP("SW %s: bit %s -> %s = %d", s.name, bitSet ? "SET" : "CLR", s.datarefName, desired);
        }
    }
}

void ProductPAP3MCP::decodeEncoders(const uint8_t *r, int len, const uint8_t *prev)
{
    if (len != kInputBytes || !prev)
        return;

    auto delta8 = [](uint8_t now, uint8_t was) -> int
    { return int(int8_t(now - was)); };

    if (!profile)
        return;
    const auto &encs = static_cast<McpAircraftProfile *>(profile)->encoderActions();

    auto findEnc = [&](uint8_t posOff) -> const McpEncoderAction *
    {
        for (const auto &e : encs)
            if (e.posOffset == posOff)
                return &e;
        return nullptr;
    };

    for (const auto &e : kEncDefs)
    {
        const uint8_t now = (e.posOff < len) ? r[e.posOff] : 0;
        const uint8_t was = prev[e.posOff];
        if (now == was)
            continue;

        const int d = delta8(now, was);
        LOGP("ENC %s: delta=%d (pos 0x%02X -> 0x%02X)", e.name, d, was, now);

        const McpEncoderAction *act = findEnc(e.posOff);
        if (!act)
            continue;

        const char *cmd = (d > 0 ? act->incCmd : act->decCmd);
        const int reps = std::abs(d) * std::max(1, act->stepPerTick);
        for (int i = 0; i < reps; ++i)
            Dataref::getInstance()->executeCommand(cmd);
    }
}

// -------------------------------------------------------------
// Displays (LCD) – initialization + transactions
// -------------------------------------------------------------

void ProductPAP3MCP::initializeDisplays()
{
    // Exact 64-byte LCD init packet (same as your Python script).
    // Hex:
    // f0 00 03 12 0f bf 00 00 04 01 00 00 26 cc 00 00
    // 00 01 00 00 00 03 00 00 + pad to 64
    std::vector<uint8_t> pkt = {
        0xF0, 0x00, 0x03, 0x12, 0x0F, 0xBF, 0x00, 0x00,
        0x04, 0x01, 0x00, 0x00, 0x26, 0xCC, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00};
    pad64(pkt);

    const bool ok = writeData(pkt);
    LOGP("initializeDisplays(): LCD init sent (write=%s)", ok ? "ok" : "FAIL");
}

void ProductPAP3MCP::updateDisplays()
{
    (void)lastUpdateCycle;
}

void ProductPAP3MCP::setLedBrightness(int /*logicalLedId*/, uint8_t /*brightness*/)
{
    // Not implemented for PAP3 (LEDs are on/off only)
}

// -------------------------------------------------------------
// Profiles & datarefs
// -------------------------------------------------------------

void ProductPAP3MCP::setProfileForCurrentAircraft()
{
    if (profile)
        return;

    if (ZiboMcpProfile::IsEligible())
    {
        profile = new ZiboMcpProfile();
        profile->initialize(); // resolves XPLM handles if needed
        LOGP("Using profile: %s (Zibo detected)", profile->name());
    }
    else
    {
        LOGP("No eligible MCP profile found; LEDs/LCD will remain idle");
    }
}

void ProductPAP3MCP::monitorDatarefs()
{
    if (!profile)
        return;

    auto *drm = Dataref::getInstance();
    for (const auto &name : profile->ledDatarefs())
    {
        drm->exists(name.c_str());
    }
    LOGP("monitorDatarefs(): touched %zu LED datarefs", profile->ledDatarefs().size());
}

// -------------------------------------------------------------
// LEDs
// -------------------------------------------------------------

void ProductPAP3MCP::sendLedRaw(uint8_t ledId, bool on)
{
    // Exact short frame (14 bytes). No report-id, no padding:
    // 02 0F BF 00 00 03 49 XX YY 00 00 00 00 00
    std::vector<uint8_t> frame = {
        0x02, 0x0F, 0xBF, 0x00, 0x00, 0x03, 0x49,
        ledId,
        static_cast<uint8_t>(on ? 0x01 : 0x00),
        0x00, 0x00, 0x00, 0x00, 0x00};
    const bool ok = writeData(frame);
    LOGP("LED 0x%02X <- %s (write=%s)", ledId, on ? "ON" : "OFF", ok ? "ok" : "FAIL");
}

void ProductPAP3MCP::registerLedMonitors()
{
    if (!profile)
        return;

    auto *drm = Dataref::getInstance();
    const auto &bindings = profile->ledBindings();
    _ledMonitoredRefs.clear();
    _ledMonitoredRefs.reserve(bindings.size());

    LOGP("Registering %zu LED monitors", bindings.size());

    for (const auto &b : bindings)
    {
        const std::string ref = b.datarefName;
        const uint8_t hwLedId = translateLogicalToPap3(b.logical);

        const auto logical = b.logical; // juste avant l'appel au monitor

        drm->monitorExistingDataref<int>(
            ref.c_str(),
            [this, hwLedId, rule = b.rule, name = ref, logical](int v)
            {
                bool on = false;
                switch (rule)
                {
                case McpLedRule::GT0:
                    on = (v > 0);
                    break;
                case McpLedRule::EQ1:
                    on = (v == 1);
                    break;
                case McpLedRule::GE1:
                    on = (v >= 1);
                    break;
                }

                const uint8_t desired = on ? 0x01 : 0x00;
                auto it = _lastLedValue.find(hwLedId);
                if (it == _lastLedValue.end() || it->second != desired)
                {
                    LOGP("LED change (monitor): DR='%s' val=%d -> LED 0x%02X %s",
                         name.c_str(), v, hwLedId, on ? "ON" : "OFF");

                    // Trigger solenoid ONLY when AT ARM goes from ON -> OFF
                    if (logical == McpLogicalLed::AT_ARM)
                    {
                        const uint8_t prev = (it == _lastLedValue.end()) ? 0xFF : it->second;
                        if (prev == 0x01 && desired == 0x00)
                        {
                            pulseATSolenoid();
                        }
                    }

                    // Force an LCD refresh when VS LED toggles (to show/hide V/S immediately)
                    if (logical == McpLogicalLed::VS)
                    {
                        const uint8_t prev = (it == _lastLedValue.end()) ? 0xFF : it->second;
                        if (prev != desired)
                        {
                            // Refresh immédiat pour afficher/masquer le label/signe tout de suite
                            forceLcdRefresh();

                            // Si la LED V/S vient de passer à ON, on re-lit VVI ~200ms plus tard
                            if (desired == 0x01)
                            {
                                XPLMUnregisterFlightLoopCallback(ProductPAP3MCP::PAP3_ReevalVSFL, this);
                                XPLMRegisterFlightLoopCallback(ProductPAP3MCP::PAP3_ReevalVSFL, 0.20f, this);
                            }
                        }
                    }

                    sendLedRaw(hwLedId, on);
                    _lastLedValue[hwLedId] = desired;
                }
            });

        _ledMonitoredRefs.push_back(ref);

        // Initial sync
        if (drm->exists(ref.c_str()))
        {
            int v = drm->get<int>(ref.c_str());
            bool on = false;
            switch (b.rule)
            {
            case McpLedRule::GT0:
                on = (v > 0);
                break;
            case McpLedRule::EQ1:
                on = (v == 1);
                break;
            case McpLedRule::GE1:
                on = (v >= 1);
                break;
            }
            const uint8_t desired = on ? 0x01 : 0x00;
            auto it = _lastLedValue.find(hwLedId);
            if (it == _lastLedValue.end() || it->second != desired)
            {
                LOGP("LED initial sync: DR='%s' val=%d -> LED 0x%02X %s",
                     ref.c_str(), v, hwLedId, on ? "ON" : "OFF");
                sendLedRaw(hwLedId, on);
                _lastLedValue[hwLedId] = desired;
                if (b.logical == McpLogicalLed::VS && on)
                {
                    // buildLcdPayloadFromCache() uses _lastLedValue to decide V/S visibility
                    forceLcdRefresh();
                }
            }
        }
        else
        {
            LOGP("WARN: dataref not found at init: '%s'", ref.c_str());
        }
    }
}

void ProductPAP3MCP::unregisterLedMonitors()
{
    auto *drm = Dataref::getInstance();
    for (const auto &ref : _ledMonitoredRefs)
    {
        drm->unbind(ref.c_str());
    }
    LOGP("Unregistered %zu LED monitors", _ledMonitoredRefs.size());
    _ledMonitoredRefs.clear();
}

float ProductPAP3MCP::PAP3_InputPollFL(float, float, int, void *inRefcon)
{
    auto *self = static_cast<ProductPAP3MCP *>(inRefcon);
    if (!self || !self->connected)
        return -1.0f;

    self->USBDevice::update();
    return -1.0f; // à chaque frame
}

// -------------------------------------------------------------
// Magnetic AT solenoid
// -------------------------------------------------------------
float ProductPAP3MCP::PAP3_ATSolenoidOnFL(float, float, int, void *inRefcon)
{
    auto *self = static_cast<ProductPAP3MCP *>(inRefcon);
    if (!self || !self->connected)
        return 0.0f;

    if (!self->_apHasPower)
    {
        // No A/P power -> retry a bit later
        LOGP("AT solenoid ON delayed: no A/P power");
        return 0.20f; // 200 ms
    }

    std::vector<uint8_t> frameOn = {
        0x02, 0x0F, 0xBF, 0x00, 0x00, 0x03, 0x49, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    self->writeData(frameOn);
    LOGP("AT solenoid ON sent (delayed)");
    return 0.0f; // one-shot when it succeeds
}

void ProductPAP3MCP::pulseATSolenoid()
{
    // Send OFF immediately
    std::vector<uint8_t> frameOff = {
        0x02, 0x0F, 0xBF, 0x00, 0x00, 0x03, 0x49,
        0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    writeData(frameOff);

    // Schedule ON a few ms later (no blocking sleep)
    const float delaySeconds = 0.06f; // ~60 ms pulse
    XPLMRegisterFlightLoopCallback(ProductPAP3MCP::PAP3_ATSolenoidOnFL, delaySeconds, this);

    LOGP("AT solenoid OFF sent, ON scheduled in %.0f ms", delaySeconds * 1000.0f);
}

// -------------------------------------------------------------
// MCP display monitors (read) -> trigger LCD push
// -------------------------------------------------------------

void ProductPAP3MCP::registerDisplayMonitors()
{
    if (!profile)
        return;

    auto *drm = Dataref::getInstance();
    _displayMonitoredRefs.clear();

    const auto &binds = profile->displayBindings();
    _displayMonitoredRefs.reserve(binds.size());

    for (const auto &b : binds)
    {
        _displayMonitoredRefs.push_back(b.datarefName);

        if (b.type == McpRefType::Float)
        {
            drm->monitorExistingDataref<float>(b.datarefName.c_str(),
                                               [this, item = b.item](float v)
                                               {
                                                   auto &st = _displayCache[item];
                                                   if (!st.hasFlt || std::fabs(st.lastFlt - v) > 1e-3f)
                                                   {
                                                       st.hasFlt = true;
                                                       st.lastFlt = v;

                                                       // ----- NEW: force refresh when IAS/MACH heuristic flips -----
                                                       if (item == McpDisplayItem::SPD)
                                                       {
                                                           const bool spdMachNow = (v > 0.f && v < 2.0f);
                                                           if (spdMachNow != _lastSpdMach)
                                                           {
                                                               _lastSpdMach = spdMachNow;
                                                               LOGP("SPD mode changed -> %s (re-read SPD + force LCD)", spdMachNow ? "MACH" : "IAS");
                                                               refreshSpdFromDatarefAndPush();
                                                               return; // stop here; we already pushed with the fresh value
                                                           }
                                                       }
                                                       // --------------------------------------------------------------

                                                       // Logging (inchangé)
                                                       switch (item)
                                                       {
                                                       case McpDisplayItem::SPD:
                                                           LOGP("MCP SPD change: %.3f", v);
                                                           break;
                                                       case McpDisplayItem::VVI:
                                                           LOGP("MCP VVI change: %.1f fpm", v);
                                                           break;
                                                       default:
                                                           LOGP("MCP (float) item=%d change: %.3f", (int)item, v);
                                                           break;
                                                       }

                                                       onDisplayChanged(); // envoie normal
                                                   }
                                               });
        }
        else
        {
            drm->monitorExistingDataref<int>(b.datarefName.c_str(),
                                             [this, item = b.item](int v)
                                             {
                                                 auto &st = _displayCache[item];
                                                 if (!st.hasInt || st.lastInt != v)
                                                 {
                                                     st.hasInt = true;
                                                     st.lastInt = v;

                                                     // Special logging for flags
                                                     switch (item)
                                                     {
                                                     case McpDisplayItem::HDG:
                                                         break;
                                                     case McpDisplayItem::ALT:
                                                         break;
                                                     case McpDisplayItem::CRS_CAPT:
                                                         break;
                                                     case McpDisplayItem::CRS_FO:
                                                         break;
                                                     case McpDisplayItem::SPD_DIGIT_8:
                                                         break;
                                                     case McpDisplayItem::SPD_DIGIT_A:
                                                         break;
                                                     default:
                                                         LOGP("MCP (int) item=%d change: %d", (int)item, v);
                                                         break;
                                                     }
                                                     onDisplayChanged();
                                                 }
                                             });
        }

        drm->exists(b.datarefName.c_str()); // touch
    }

    LOGP("Registered %zu MCP display monitors", binds.size());
}

void ProductPAP3MCP::unregisterDisplayMonitors()
{
    auto *drm = Dataref::getInstance();
    for (const auto &ref : _displayMonitoredRefs)
    {
        drm->unbind(ref.c_str());
    }
    _displayMonitoredRefs.clear();
    _displayCache.clear();
    LOGP("Unregistered MCP display monitors");
}

// -------------------------------------------------------------
// LCD TX (0x38 payload at 0x19 + 0x38 empty x2 + 0x2A commit)
// -------------------------------------------------------------

void ProductPAP3MCP::sendLcd38WithPayload(const std::vector<uint8_t> &payloadAfter0x19)
{
    std::vector<uint8_t> buf(std::begin(HEAD_38_PREFIX), std::end(HEAD_38_PREFIX));
    buf[2] = _lcdSeq; // set seq
    buf.insert(buf.end(), payloadAfter0x19.begin(), payloadAfter0x19.end());
    pad64(buf);

    const bool ok = writeData(buf);

    _lcdSeq++;
    if (_lcdSeq == 0)
        _lcdSeq = 1;
}

void ProductPAP3MCP::sendLcd38Empty()
{
    std::vector<uint8_t> buf = {0xF0, 0x00, _lcdSeq, 0x38};
    pad64(buf);

    const bool ok = writeData(buf);

    _lcdSeq++;
    if (_lcdSeq == 0)
        _lcdSeq = 1;
}

void ProductPAP3MCP::sendLcd2ACommit()
{
    std::vector<uint8_t> buf = {0xF0, 0x00, _lcdSeq, 0x2A};
    if (buf.size() < 0x1D)
        buf.resize(0x1D, 0x00);
    const std::vector<uint8_t> tail = {0x0F, 0xBF, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0xDF, 0xA2, 0x50, 0x00};
    buf.insert(buf.end(), tail.begin(), tail.end());
    pad64(buf);

    const bool ok = writeData(buf);

    _lcdSeq++;
    if (_lcdSeq == 0)
        _lcdSeq = 1;
}

void ProductPAP3MCP::sendLcdTransaction(const std::vector<uint8_t> &payloadAfter0x19)
{
    // Order matters
    sendLcd38WithPayload(payloadAfter0x19);
    sendLcd38Empty();
    sendLcd38Empty();
    sendLcd2ACommit();
}

void ProductPAP3MCP::onDisplayChanged()
{
    // Build payload from _displayCache and push immediately
    const auto payload = buildLcdPayloadFromCache();
    sendLcdTransaction(payload);
    _lcdDirty = false;
}

std::vector<uint8_t> ProductPAP3MCP::buildLcdPayloadFromCache() const
{
    // Payload for offsets 0x19..0x38 (inclusive) -> 0x20 bytes.
    std::vector<uint8_t> p(0x38 - 0x19 + 1, 0x00);

    auto OR = [&](uint8_t absOffset, uint8_t mask)
    {
        const int idx = int(absOffset) - 0x19;
        if (idx >= 0 && idx < (int)p.size())
            p[idx] |= mask;
    };
    auto getI = [&](McpDisplayItem it, int def = 0) -> int
    {
        auto itf = _displayCache.find(it);
        if (itf != _displayCache.end() && itf->second.hasInt)
            return itf->second.lastInt;
        return def;
    };
    auto getF = [&](McpDisplayItem it, float def = 0.f) -> float
    {
        auto itf = _displayCache.find(it);
        if (itf != _displayCache.end() && itf->second.hasFlt)
            return itf->second.lastFlt;
        return def;
    };

    // ------------ values ------------
    // SPD: heuristic for IAS vs MACH (digit_A is "Above", not Mach!)
    const float spdF_raw = getF(McpDisplayItem::SPD, 0.f);
    const bool spdMach = (spdF_raw > 0.f && spdF_raw < 2.0f);

    // In MACH, go back to rounding (was truncation); IAS still rounds as before.
    const int spd = std::clamp(
        spdMach ? (int)std::round(spdF_raw * 100.0f) // MACH -> 2 decimals, rounded
                : (int)std::round(spdF_raw),         // IAS  -> integer knots, rounded
        0, 9999);

    const bool spdBelow = (getI(McpDisplayItem::SPD_DIGIT_8, 0) != 0); // Below
    const bool spdAbove = (getI(McpDisplayItem::SPD_DIGIT_A, 0) != 0); // Above

    const int hdg = std::clamp(getI(McpDisplayItem::HDG, 0) % 360, 0, 359);

    const int altRaw = std::max(0, getI(McpDisplayItem::ALT, 0));
    const int alt = std::min(altRaw, 99999);

    // V/S read as float → round to int for display
    const int vviRaw = (int)std::round(getF(McpDisplayItem::VVI, 0.f));
    const int vviAbs = std::min(std::abs(vviRaw), 9999);
    const bool vsZero = (vviRaw == 0);
    bool vsEngaged = false; // consider engaged if VS LED last-sent state is ON
    {
        auto itVS = _lastLedValue.find(static_cast<uint8_t>(Pap3Led::VS));
        if (itVS != _lastLedValue.end())
            vsEngaged = (itVS->second != 0);
    }

    const int crsC = std::clamp(getI(McpDisplayItem::CRS_CAPT, 0) % 360, 0, 359);
    const int crsF = std::clamp(getI(McpDisplayItem::CRS_FO, 0) % 360, 0, 359);

    // ------------ labels/dots/signs -------------
    if (ProductPAP3MCP::getShowLcdLabels())
    {
        if (spdMach)
        {
            // MACH label (two halves)
            OR(0x32, 0x80);
            OR(0x2E, 0x80);
        }
        else
        {
            // IAS label
            OR(0x36, 0x80);
        }

        // HDG label (two halves). For TRK you would use 0x2E/0x2A instead.
        OR(0x36, 0x08);
        OR(0x32, 0x08);

        // V/S label only if VS ≠ 0 and engaged
        if (!vsZero && vsEngaged)
            OR(0x38, 0x80);
    }

    // Mach decimal dot for SPD (independent of labels)
    if (spdMach)
        OR(0x19, 0x04);

    // V/S sign only if V/S ≠ 0 and engaged
    if (!vsZero && vsEngaged)
    {
        if (vviRaw < 0)
        {
            // minus only
            OR(0x1F, 0x10);
        }
        else
        {
            // plus = minus + top + bottom bars
            OR(0x1F, 0x10);
            OR(0x28, 0x80);
            OR(0x2C, 0x80);
        }
    }

    // ------------ 7-segment helpers ------------
    struct SegGroup
    {
        uint8_t g, f, e, d, c, b, a;
    };
    const SegGroup G1{0x1D, 0x21, 0x25, 0x29, 0x2D, 0x31, 0x35}; // CPT_CRS & SPD
    const SegGroup G2{0x1E, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x36}; // HDG / ALT_HI
    const SegGroup G3{0x1F, 0x23, 0x27, 0x2B, 0x2F, 0x33, 0x37}; // ALT_LO / VSPD
    const SegGroup G4{0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38}; // FO_CRS

    auto segMaskForDigit = [](int d) -> uint8_t
    {
        switch (d)
        {
        case 0:
            return 0b01111110; // A B C D E F
        case 1:
            return 0b00001100; // B C
        case 2:
            return 0b10110110; // A B D E G
        case 3:
            return 0b10011110; // A B C D G
        case 4:
            return 0b11001100; // B C F G
        case 5:
            return 0b11011010; // A C D F G
        case 6:
            return 0b11111010; // A C D E F G
        case 7:
            return 0b00001110; // A B C
        case 8:
            return 0b11111110; // A B C D E F G
        case 9:
            return 0b11011110; // A B C D F G
        default:
            return 0;
        }
    };

    auto lightSegments = [&](const SegGroup &grp, uint8_t digitMask, int value)
    {
        const uint8_t pat = segMaskForDigit(value);
        if (pat & 0b00000010)
            OR(grp.a, digitMask); // A
        if (pat & 0b00000100)
            OR(grp.b, digitMask); // B
        if (pat & 0b00001000)
            OR(grp.c, digitMask); // C
        if (pat & 0b00010000)
            OR(grp.d, digitMask); // D
        if (pat & 0b00100000)
            OR(grp.e, digitMask); // E
        if (pat & 0b01000000)
            OR(grp.f, digitMask); // F
        if (pat & 0b10000000)
            OR(grp.g, digitMask); // G
    };

    auto split3 = [](int v)
    { return std::array<int, 3>{(v / 100) % 10, (v / 10) % 10, v % 10}; };
    auto split4 = [](int v)
    { return std::array<int, 4>{(v / 1000) % 10, (v / 100) % 10, (v / 10) % 10, v % 10}; };
    auto split5 = [](int v)
    { return std::array<int, 5>{(v / 10000) % 10, (v / 1000) % 10, (v / 100) % 10, (v / 10) % 10, v % 10}; };

    // ------------ SPD (Group 1) ------------
    {
        const auto d = split4(spd);
        if (d[0])
            lightSegments(G1, 0x08, d[0]); // kilo only if non-zero
        if (d[1])
            lightSegments(G1, 0x04, d[1]); // hundreds only if non-zero
        lightSegments(G1, 0x02, d[2]);     // tens
        lightSegments(G1, 0x01, d[3]);     // units

        // Inject Below/Above prefix into kilo digit (only in IAS mode).
        if (!spdMach)
        {
            if (spdAbove)
            {
                // Letter 'A' = A B C E F G (not D)
                OR(G1.a, 0x08);
                OR(G1.b, 0x08);
                OR(G1.c, 0x08);
                OR(G1.e, 0x08);
                OR(G1.f, 0x08);
                OR(G1.g, 0x08);
            }
            else if (spdBelow)
            {
                // Digit '8' full
                lightSegments(G1, 0x08, 8);
            }
        }
    }

    // ------------ Captain CRS (Group 1) ------------
    {
        const auto d = split3(crsC);
        lightSegments(G1, 0x80, d[0]); // hundreds
        lightSegments(G1, 0x40, d[1]); // tens
        lightSegments(G1, 0x20, d[2]); // units
    }

    // ------------ HDG (Group 2) ------------
    {
        const auto d = split3(hdg);
        lightSegments(G2, 0x40, d[0]); // hundreds
        lightSegments(G2, 0x20, d[1]); // tens
        lightSegments(G2, 0x10, d[2]); // units
    }

    // ------------ ALT (Group 2 high, Group 3 low) ------------
    // No leading zero on 10k → only light the 10k if non-zero.
    {
        const auto d = split5(alt); // [tenK, kilo, hundreds, tens, units]
        if (d[0])
            lightSegments(G2, 0x04, d[0]); // 10k ONLY if > 0
        lightSegments(G2, 0x02, d[1]);     // 1k
        lightSegments(G2, 0x01, d[2]);     // 100
        lightSegments(G3, 0x80, d[3]);     // 10
        lightSegments(G3, 0x40, d[4]);     // 1
    }

    // ------------ V/S (Group 3) ------------
    // If V/S == 0 or not engaged → hide everything for V/S (label, signs, digits).
    if (!vsZero && vsEngaged)
    {
        const auto d = split4(vviAbs);
        if (d[0])
            lightSegments(G3, 0x08, d[0]); // 1000s only if non-zero
        lightSegments(G3, 0x04, d[1]);     // 100s
        lightSegments(G3, 0x02, d[2]);     // 10s
        lightSegments(G3, 0x01, d[3]);     // 1s
    }

    // ------------ FO CRS (Group 4) ------------
    {
        const auto d = split3(crsF);
        lightSegments(G4, 0x40, d[0]); // hundreds
        lightSegments(G4, 0x20, d[1]); // tens
        lightSegments(G4, 0x10, d[2]); // units
    }

    return p;
}

void ProductPAP3MCP::forceLcdRefresh()
{
    // Rebuild from cache and push the 0x38 + two empties + 0x2A commit
    const auto payload = buildLcdPayloadFromCache();
    sendLcdTransaction(payload);
    LOGP("LCD refresh forced (labels=%s)", ProductPAP3MCP::getShowLcdLabels() ? "ON" : "OFF");
}

void ProductPAP3MCP::refreshSpdFromDatarefAndPush()
{
    // Resolve the SPD dataref name once from the profile bindings
    if (_spdRefName.empty() && profile)
    {
        for (const auto &b : profile->displayBindings())
        {
            if (b.item == McpDisplayItem::SPD)
            {
                _spdRefName = b.datarefName;
                break;
            }
        }
    }

    if (_spdRefName.empty())
    {
        LOGP("refreshSpdFromDatarefAndPush(): SPD ref not found in bindings");
        forceLcdRefresh(); // best effort
        return;
    }

    // Read fresh (bypass our cache update cadence)
    float fresh = Dataref::getInstance()->get<float>(_spdRefName.c_str());

    // Update our local cache immediately
    auto &st = _displayCache[McpDisplayItem::SPD];
    st.hasFlt = true;
    st.lastFlt = fresh;

    LOGP("SPD re-read on mode flip: %.4f (forcing LCD)");
    forceLcdRefresh();
}

float ProductPAP3MCP::PAP3_ReevalVSFL(float, float, int, void *inRefcon)
{
    auto *self = static_cast<ProductPAP3MCP *>(inRefcon);
    if (!self || !self->connected)
        return 0.0f;

    self->refreshVviFromDatarefAndPush();
    return 0.0f; // one-shot
}

void ProductPAP3MCP::refreshVviFromDatarefAndPush()
{
    // Résoudre le nom du dataref VVI une seule fois via les bindings du profil
    if (_vviRefName.empty() && profile)
    {
        for (const auto &b : profile->displayBindings())
        {
            if (b.item == McpDisplayItem::VVI)
            {
                _vviRefName = b.datarefName;
                break;
            }
        }
    }

    if (_vviRefName.empty())
    {
        LOGP("refreshVviFromDatarefAndPush(): VVI ref not found in bindings");
        forceLcdRefresh(); // best effort
        return;
    }

    // Lecture fraîche + mise à jour du cache
    float fresh = Dataref::getInstance()->get<float>(_vviRefName.c_str());
    auto &st = _displayCache[McpDisplayItem::VVI];
    st.hasFlt = true;
    st.lastFlt = fresh;

    LOGP("VVI re-read after VS ON: %.1f (forcing LCD)", fresh);
    forceLcdRefresh();
}
