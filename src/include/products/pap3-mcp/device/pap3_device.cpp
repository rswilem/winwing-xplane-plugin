// src/include/products/pap3-mcp/device/pap3_device.cpp
//
// PAP3 MCP - Device glue to transport, inputs, LCD compose, and aircraft profile.
// ------------------------------------------------------------------------------
// Responsibilities:
//  • Own the USB/HID connection and install the transport writer
//  • Run a minimal startup sequence (LCD init + payload + commit + dimming defaults)
//  • Bridge aircraft State → device (LEDs, LCD, dimming, solenoid)
//  • Poll HID IN reports and forward to Inputs decoder
//  • Forward input edges/encoders to the active aircraft profile

#include "pap3_device.h"
#include "pap3_demo.h"
#include "transport.h"
#include "../lcd/compose.h"
#include "../profiles/profile_factory.h"
#include "../aircraft/pap3_aircraft.h"
#include "../menu/pap3_menu.h"
#include "inputs.h"

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace {

/// Encoder human names by rolling-position offset (display-only).
struct EncDef { std::uint8_t posOff; const char* name; };

constexpr std::array<EncDef, 6> kEncDefs{{
    {0x15, "CRS CAPT"},
    {0x17, "SPD"},
    {0x19, "HDG"},
    {0x1B, "ALT"},
    {0x1D, "V/S"},
    {0x1F, "CRS FO"},
}};

inline const char* LookupEncoderName(std::uint8_t posOff)
{
    for (const auto& e : kEncDefs) {
        if (e.posOff == posOff) return e.name;
    }
    return nullptr;
}

} // anonymous namespace

namespace pap3::device {

using pap3::device::transport::DevicePtr;
using pap3::device::transport::sendATSolenoid;
using pap3::device::transport::sendDimming;
using pap3::device::transport::sendLcdCommit;
using pap3::device::transport::sendLcdEmptyFrame;
using pap3::device::transport::sendLcdInit;
using pap3::device::transport::sendLcdPayload;
using pap3::device::transport::setWriter;
using pap3::device::transport::writerUsbWriteData;

using pap3::aircraft::PAP3AircraftProfile;
using pap3::aircraft::ProfileFactory;

namespace lcdc = pap3::lcd::compose;

// -----------------------------------------------------------------------------
// Construction / destruction
// -----------------------------------------------------------------------------

/*
    Constructor behavior:
      - Install the transport writer.
      - Connect the HID device immediately (factory-style usage).
      - Run a minimal startup sequence that proves the path works:
          * LCD init (0x12)
          * LCD payload (32B zeros by default; profile will override later)
          * LCD commit (0x2A)
          * Set dimming defaults for Backlight/LCD/LEDs
*/
PAP3Device::PAP3Device(HIDDeviceHandle hidDevice,
                       std::uint16_t  vendorId,
                       std::uint16_t  productId,
                       const std::string& vendorName,
                       const std::string& productName)
: USBDevice(hidDevice, vendorId, productId, vendorName, productName)
, _seq(1)
{
    ensureWriterInstalled();
    debug_force("PAP3Device constructed - vendorId: 0x%04X, productId: 0x%04X\n", vendorId, productId);

    // Attempt connection. Keep object valid even if connect() fails.
    if (USBDevice::connect()) {
        debug_force("PAP3Device connected\n");
        runStartupSequence();
    }
}

PAP3Device::~PAP3Device()
{
    if (_inputFLRegistered) {
        XPLMUnregisterFlightLoopCallback(&PAP3Device::InputPollFL, this);

        _inputFLRegistered = false;
        debug_force("[PAP3] Input FlightLoop unregistered\n");
    }
    _inputs.onEncoders    = nullptr;
    _inputs.onButtons     = nullptr;
    _inputs.onSwitchEdges = nullptr;
    _inputs.onLightSensor = nullptr;
    _inputs.onRaw         = nullptr;

    // Stopper proprement le worker
    if (_ioRunning.exchange(false)) {
        _ioCv.notify_all();
        if (_ioThread.joinable()) _ioThread.join();
    }
    USBDevice::disconnect();
}

// -----------------------------------------------------------------------------
// Internal helpers
// -----------------------------------------------------------------------------

void PAP3Device::ensureWriterInstalled() const
{
    // Route transport.* writes to USBDevice::writeData(std::vector<uint8_t>).
    setWriter(&writerUsbWriteData);
}

void PAP3Device::applyState(const pap3::aircraft::State& st)
{
    // --- LED mapping: PAP3 hardware IDs --------------------------------------
    static constexpr std::uint8_t kLedId_N1      = 0x03;
    static constexpr std::uint8_t kLedId_SPEED   = 0x04;
    static constexpr std::uint8_t kLedId_VNAV    = 0x05;
    static constexpr std::uint8_t kLedId_LVL_CHG = 0x06;
    static constexpr std::uint8_t kLedId_HDG_SEL = 0x07;
    static constexpr std::uint8_t kLedId_LNAV    = 0x08;
    static constexpr std::uint8_t kLedId_VORLOC  = 0x09;
    static constexpr std::uint8_t kLedId_APP     = 0x0A;
    static constexpr std::uint8_t kLedId_ALT_HLD = 0x0B;
    static constexpr std::uint8_t kLedId_VS      = 0x0C;
    static constexpr std::uint8_t kLedId_CMD_A   = 0x0D;
    static constexpr std::uint8_t kLedId_CWS_A   = 0x0E;
    static constexpr std::uint8_t kLedId_CMD_B   = 0x0F;
    static constexpr std::uint8_t kLedId_CWS_B   = 0x10;
    static constexpr std::uint8_t kLedId_AT_ARM  = 0x11;
    static constexpr std::uint8_t kLedId_MA_CAPT = 0x12;
    static constexpr std::uint8_t kLedId_MA_FO   = 0x13;


    // Push LED states
    qSetLed(kLedId_N1,      st.led.N1);
    qSetLed(kLedId_SPEED,   st.led.SPEED);
    qSetLed(kLedId_VNAV,    st.led.VNAV);
    qSetLed(kLedId_LVL_CHG, st.led.LVL_CHG);
    qSetLed(kLedId_HDG_SEL, st.led.HDG_SEL);
    qSetLed(kLedId_LNAV,    st.led.LNAV);
    qSetLed(kLedId_VORLOC,  st.led.VORLOC);
    qSetLed(kLedId_APP,     st.led.APP);
    qSetLed(kLedId_ALT_HLD, st.led.ALT_HLD);
    qSetLed(kLedId_VS,      st.led.V_S);
    qSetLed(kLedId_CMD_A,   st.led.CMD_A);
    qSetLed(kLedId_CWS_A,   st.led.CWS_A);
    qSetLed(kLedId_CMD_B,   st.led.CMD_B);
    qSetLed(kLedId_CWS_B,   st.led.CWS_B);
    qSetLed(kLedId_AT_ARM,  st.led.AT_ARM);
    qSetLed(kLedId_MA_CAPT, st.led.MA_CAPT);
    qSetLed(kLedId_MA_FO,   st.led.MA_FO);

    // --- LCD: State → Snapshot → 32B payload --------------------------------
    lcdc::Snapshot s{};
    s.spd     = st.spd;
    s.hdg     = st.hdg;
    s.alt     = st.alt;
    s.vvi     = st.vvi;
    s.showVvi = st.vviVisible;
    s.crsCapt = st.crsCapt;
    s.crsFo   = st.crsFo;
    s.digitA  = st.digitA;
    s.digitB  = st.digitB;

    // Default labels/dots
    const bool show = pap3menu::GetShowLcdLabels();
    s.lblIAS = show;
    s.lblHDG = show;
    s.lblVS  = show;

    s.dotSpd = s.dotAlt = s.dotVvi = s.dotCrsCapt = s.dotCrsFo = s.dotHdg = false;

    // Build 32-byte payload and send LCD sequence.
    const lcdc::Payload payload = lcdc::build(s);
    std::vector<std::uint8_t> lcd32(payload.begin(), payload.end());
    qLcdPayload(lcd32);

    // Convert float [0..1] to int [0..100] and push as "aircraft percents"
    const int blPct  = std::clamp(static_cast<int>(std::lround(st.cockpitLights * 100.0f)), 0, 100);
    const int lcdPct = std::clamp(static_cast<int>(std::lround(st.mcpBrightness * 100.0f)), 0, 100);
    const int ledsPct = std::clamp(static_cast<int>(std::lround(st.ledsBrightness * 100.0f)), 0, 100);

    _illum.setAircraftPercents(blPct, lcdPct, ledsPct);

    const auto out = _illum.compute();
    if (out.changed[0]) setDimming(0, out.raw[0]); // Backlight
    if (out.changed[1]) setDimming(1, out.raw[1]); // LCD
    if (out.changed[2]) setDimming(2, out.raw[2]); // LEDs

    // Illumination / power gating based on profile power mask
    updatePower();
}

void PAP3Device::runStartupSequence()
{
    // 1) LCD init
    sendLcdInit(static_cast<DevicePtr>(this), _seq);

    // 2) Minimal 32B payload (zeros) + two empty frames
    std::vector<std::uint8_t> lcd32(32, 0x00);
    sendLcdPayload(static_cast<DevicePtr>(this), _seq, lcd32);
    sendLcdEmptyFrame(static_cast<DevicePtr>(this), _seq);
    sendLcdEmptyFrame(static_cast<DevicePtr>(this), _seq);

    // 3) Commit
    sendLcdCommit(static_cast<DevicePtr>(this), _seq);

    // 4) Dimming defaults
    sendDimming(static_cast<DevicePtr>(this), 0, 255); // Backlight
    sendDimming(static_cast<DevicePtr>(this), 1, 255); // LCD
    sendDimming(static_cast<DevicePtr>(this), 2, 255); // LEDs

    // Illumination engine defaults: Backlight driven by Aircraft percent,
    // LCD/LEDs fixed at 100%.
    _illumCfg.backlight.mode        = pap3::device::IllumMode::Aircraft;
    _illumCfg.backlight.fixedPercent= 100; // ignored in Aircraft mode
    _illumCfg.lcd.mode              = pap3::device::IllumMode::Aircraft;
    _illumCfg.lcd.fixedPercent      = 100;
    _illumCfg.leds.mode             = pap3::device::IllumMode::Aircraft;
    _illumCfg.leds.fixedPercent     = 100;
    _illum.setConfig(_illumCfg);

    // Initialize aircraft percents to user baseline
    _illum.setAircraftPercents(_userBacklightPct, 100, 100);
    _illum.setLightSensorPercent(100);
    _illum.setPowerAvailable(true);

    // Apply computed dimming (first frame → all changed)
    {
        auto out = _illum.compute();
        if (out.changed[0]) qSetDimming(0, out.raw[0]);
        if (out.changed[1]) qSetDimming(1, out.raw[1]);
        if (out.changed[2]) qSetDimming(2, out.raw[2]);
    }

     // Detect + start aircraft profile
    _profile = ProfileFactory::detect();
    if (_profile) {
        _profile->attachDevice(this);
        _profile->start([this](const pap3::aircraft::State& st) {
            this->applyState(st);
        });
    }

    // Inputs setup (only once)
    //setupInputCallbacks();

    // Poll HID inputs @ ~10 Hz
    if (!_inputFLRegistered) {
        XPLMRegisterFlightLoopCallback(&PAP3Device::InputPollFL, 0.04f, this);
        _inputFLRegistered = true;
        debug_force("[PAP3] Input FlightLoop registered @ ~10 Hz\n");
    }

    // Lancer le worker I/O si pas déjà démarré
    if (!_ioRunning.load()) {
        _ioRunning.store(true);
        _ioThread = std::thread([this]{ this->ioThreadMain(); });
    }

    updatePower();
    
}

// Optional demo harness -------------------------------------------------------
static pap3::device::PAP3Demo* g_demo = nullptr;

void StartPap3Demo(pap3::device::PAP3Device* dev)
{
    // Hardware LED IDs (example/demo set)
    std::vector<std::uint8_t> ledIds = {
        0x03, 0x04, 0x06, 0x05, 0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0E, 0x0D, 0x0F, 0x10
    };

    if (!g_demo) g_demo = new pap3::device::PAP3Demo(dev);
    g_demo->start(ledIds, 2.0f, 2.0f, 2.2f);
}

// -----------------------------------------------------------------------------
// Public helpers (transport delegates)
// -----------------------------------------------------------------------------

bool PAP3Device::lcdInit()
{
    return sendLcdInit(static_cast<DevicePtr>(this), _seq);
}

bool PAP3Device::lcdSendPayload(const std::vector<std::uint8_t>& lcd32)
{
    return sendLcdPayload(static_cast<DevicePtr>(this), _seq, lcd32);
}

bool PAP3Device::lcdSendEmpty()
{
    return sendLcdEmptyFrame(static_cast<DevicePtr>(this), _seq);
}

bool PAP3Device::lcdCommit()
{
    return sendLcdCommit(static_cast<DevicePtr>(this), _seq);
}

bool PAP3Device::setLed(std::uint8_t ledId, bool on)
{
    return transport::sendLed(static_cast<DevicePtr>(this), ledId, on);
}

bool PAP3Device::setDimming(std::uint8_t channel, std::uint8_t value)
{
    return sendDimming(static_cast<DevicePtr>(this), channel, value);
}

bool PAP3Device::setATSolenoid(bool on)
{
    debug_force("[PAP3] setATSolenoid(%s) called\n", on ? "ON" : "OFF");
    return sendATSolenoid(static_cast<DevicePtr>(this), on);
}

// -----------------------------------------------------------------------------
// Flight loop & HID IN handling
// -----------------------------------------------------------------------------

float PAP3Device::InputPollFL(float /*inElapsedSinceLastCall*/,
                              float /*inElapsedTimeSinceLastFlightLoop*/,
                              int   /*inCounter*/,
                              void* refcon)
{
    auto* self = static_cast<PAP3Device*>(refcon);
    if (!self) return 0.04f;

    // Pump HID; USBDevice::update() should invoke didReceiveData() for input reports.
    self->USBDevice::update();
    return 0.04f; // schedule again in ~100 ms
}

void PAP3Device::didReceiveData(int /*reportId*/, std::uint8_t* report, int reportLength)
{
    onHidInputReport(report, reportLength);
}

void PAP3Device::onHidInputReport(const std::uint8_t* report, int len)
{
    // ---- NOUVEAU : snapshot sync au tout premier paquet ----
    if (!_bootSwitchSyncDone && _profile && report && len > 0) {
        _bootSwitchSyncDone = true;
        debug_force("[PAP3] First HID report -> snapshot sync\n");
        _profile->syncSimToHardwareFromRaw(report, len);
    }

    // Inputs::decode() filtre et n’émet que sur changements (edges).
    _inputs.decode(report, len);
}

// -----------------------------------------------------------------------------
// Inputs wiring
// -----------------------------------------------------------------------------

void PAP3Device::setupInputCallbacks()
{
    using pap3::device::nameForButton; // reuse global lookup to avoid duplication

    _inputs.setWatchedSwitchOffsets({0x01, 0x02, 0x03, 0x04, 0x05, 0x06});
    _inputs.setLightSensorOffset(0x14);

    

    _inputs.onEncoders = [this](const Inputs::EncodersFrame& e) {
        struct Item { std::int8_t d; std::uint8_t off; };
        const Item items[] = {
            {e.d_crsC, 0x15}, {e.d_spd , 0x17}, {e.d_hdg , 0x19},
            {e.d_alt , 0x1B}, {e.d_vvi , 0x1D}, {e.d_crsF, 0x1F},
        };

        for (const auto& it : items) {
            if (it.d == 0) continue;

            if (const char* name = LookupEncoderName(it.off)) {
                debug_force("[PAP3][ENC] %s: delta=%+d\n", name, static_cast<int>(it.d));
            } else {
                debug_force("[PAP3][ENC] off=0x%02X: delta=%+d\n", it.off, static_cast<int>(it.d));
            }

            if (_profile) _profile->onEncoderDelta(it.off, it.d);
        }
    };

    _inputs.onLightSensor = [](std::uint8_t /*lux*/) {};

    _inputs.onSwitchEdges = [this](std::uint8_t off, std::uint8_t changedSet, std::uint8_t changedClr) {
        auto forward = [this, off](uint8_t bit, bool pressed){
            if (_profile) _profile->onButton(off, bit, pressed);
        };
        for (std::uint8_t m = 1; m; m <<= 1) {
            if (changedSet & m)  forward(m, true);
            if (changedClr & m)  forward(m, false);
        }

        // One-shot: dès qu’on a reçu une première photo des switches, on synchronise le sim.
        if (_initialSwitchSyncPending) {
            _initialSwitchSyncPending = false;
            if (_profile) {
                debug_force("[PAP3] first switch snapshot -> sync\n");
                _profile->syncSimToHardware();
            }
        }
    };
}

// -----------------------------------------------------------------------------
// Illumination & power
// -----------------------------------------------------------------------------

void PAP3Device::updatePower()
{
    // Power mask (from profile):
    //  0x01 = AP has power
    //  0x02 = DC Bus 1
    //  0x04 = DC Bus 2
    const std::uint8_t mask = _profile ? _profile->mcpPowerMask() : 0x00;

    const bool powered = (mask & 0x01) != 0; // new rule: illumination enabled only if AP has power
    _illum.setPowerAvailable(powered);

    // Apply dimming only when changed (compute() handles change detection).
    const auto out = _illum.compute();
    if (out.changed[0]) qSetDimming(0, out.raw[0]); // Backlight
    if (out.changed[1]) qSetDimming(1, out.raw[1]); // LCD
    if (out.changed[2]) qSetDimming(2, out.raw[2]); // LEDs

    // A/T solenoid ON if any DC bus is alive.
    const bool solenoidOn = (mask & 0x06) != 0; // 0x02 | 0x04
    qSetSolenoid(solenoidOn);
}

// One-shot callback to restore the solenoid to ON after the delay.
float PAP3Device::ATPulseFL(float /*elapsed*/, float /*since*/, int /*counter*/, void* refcon)
{
    auto* self = static_cast<PAP3Device*>(refcon);
    if (!self) return 0.0f;

    // Re-enable solenoid
    self->setATSolenoid(true);
    self->_atPulsePending = false;

    // Clean up the one-shot FlightLoop
    if (self->_atPulseFL) {
        XPLMDestroyFlightLoop(self->_atPulseFL);
        self->_atPulseFL = nullptr;
    }

    return 0.0f; // no rescheduling
}

void PAP3Device::pulseATSolenoid(unsigned millis)
{
    if (_atPulsePending) return; // already running, skip

    // Immediately turn OFF
    setATSolenoid(false);

    // Create FlightLoop if it doesn't exist yet
    if (!_atPulseFL) {
        XPLMCreateFlightLoop_t desc{};
        desc.structSize   = sizeof(desc);
        desc.phase        = xplm_FlightLoop_Phase_AfterFlightModel;
        desc.callbackFunc = &PAP3Device::ATPulseFL;
        desc.refcon       = this;
        _atPulseFL = XPLMCreateFlightLoop(&desc);
    }

    _atPulsePending = true;
    const float delaySec = static_cast<float>(millis) / 1000.0f;

    // Schedule the one-shot execution relative to now
    XPLMScheduleFlightLoop(_atPulseFL, delaySec, /*relativeToNow=*/1);
}

} // namespace pap3::device
