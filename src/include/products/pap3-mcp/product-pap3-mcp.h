#ifndef PRODUCT_PAP3_MCP_H
#define PRODUCT_PAP3_MCP_H

#include "usbdevice.h"
#include "mcp-aircraft-profile.h" // abstract profile (e.g., ZiboMcpProfile)
#include <set>
#include <map>
#include <string>
#include <vector>

/**
 * ProductPAP3MCP
 *
 * Winwing PAP3 "MCP" product driver.
 * - LEDs: event-driven (FCU-style monitors), short 14-byte frames (no padding, no HID-65).
 * - LCDs: optional; we keep LEDs intact and add LCD init + 0x38/0x2A transactions on value change.
 * - Display payload starts at absolute offset 0x19 (per your reverse engineering).
 */
class ProductPAP3MCP : public USBDevice {
private:
    // --- State ---
    McpAircraftProfile *profile = nullptr;
    int lastUpdateCycle = 0;
    std::set<int> pressedButtonIndices;
    std::map<std::string, int> selectorPositions;
    bool _apHasPower = true;

    // Differential cache: key = PAP3 LED hw id, value = last 0x00/0x01 sent
    std::map<uint8_t, uint8_t> _lastLedValue;

    // Keep track of which datarefs we registered (unbind on disconnect)
    std::vector<std::string> _ledMonitoredRefs;

    // Keep track of which switch datarefs we registered
    std::vector<std::string> _switchMonitoredRefs;

    // --- Helpers ---
    void setProfileForCurrentAircraft();
    void monitorDatarefs(); // touch refs so profiles can resolve handles
    void updateDisplays();  // currently a no-op

    // LEDs
    void sendLedRaw(uint8_t ledId, bool on); // short 14-byte frame, no padding
    void registerLedMonitors();              // FCU-style: subscribe to ref changes
    void unregisterLedMonitors();            // unbind when disconnecting

    void enforceSwitchPositionsFromHardware(const uint8_t *now, const uint8_t *prev);

    // --- Magnetic AT solenoid ---
    void pulseATSolenoid();
    // Low-level A/T solenoid setter (no pulse)
    void setATSolenoid(bool on);

    // One-shot FL to re-energize the A/T solenoid after a short delay
    static float PAP3_ATSolenoidOnFL(float inElapsedSinceLastCall,
                                     float inElapsedTimeSinceLastFlightLoop,
                                     int inCounter,
                                     void *inRefcon);

    // One-frame input poller (ensures immediate HID reads)
    static float PAP3_InputPollFL(float inElapsedSinceLastCall,
                                  float inElapsedTimeSinceLastFlightLoop,
                                  int inCounter,
                                  void *inRefcon);
    bool _inputFLRegistered = false;

    // Menu
    static void createSubmenuIfNeeded();
    static void pap3MenuCallback(void *mRef, void *iRef);

    // PAP3 hardware LED map (product-level)
    enum class Pap3Led : uint8_t
    {
        N1 = 0x03,
        SPEED = 0x04,
        VNAV = 0x05,
        LVL_CHG = 0x06,
        HDG_SEL = 0x07,
        LNAV = 0x08,
        VOR_LOC = 0x09,
        APP = 0x0A,
        ALT_HLD = 0x0B,
        VS = 0x0C,
        CMD_A = 0x0D,
        CWS_A = 0x0E,
        CMD_B = 0x0F,
        CWS_B = 0x10,
        AT_ARM = 0x11,
        MA_CAPT = 0x12,
        MA_FO = 0x13
    };

    static inline uint8_t translateLogicalToPap3(McpLogicalLed logical)
    {
        switch (logical)
        {
        case McpLogicalLed::N1:
            return static_cast<uint8_t>(Pap3Led::N1);
        case McpLogicalLed::SPEED:
            return static_cast<uint8_t>(Pap3Led::SPEED);
        case McpLogicalLed::VNAV:
            return static_cast<uint8_t>(Pap3Led::VNAV);
        case McpLogicalLed::LVL_CHG:
            return static_cast<uint8_t>(Pap3Led::LVL_CHG);
        case McpLogicalLed::HDG_SEL:
            return static_cast<uint8_t>(Pap3Led::HDG_SEL);
        case McpLogicalLed::LNAV:
            return static_cast<uint8_t>(Pap3Led::LNAV);
        case McpLogicalLed::VOR_LOC:
            return static_cast<uint8_t>(Pap3Led::VOR_LOC);
        case McpLogicalLed::APP:
            return static_cast<uint8_t>(Pap3Led::APP);
        case McpLogicalLed::ALT_HLD:
            return static_cast<uint8_t>(Pap3Led::ALT_HLD);
        case McpLogicalLed::VS:
            return static_cast<uint8_t>(Pap3Led::VS);
        case McpLogicalLed::CMD_A:
            return static_cast<uint8_t>(Pap3Led::CMD_A);
        case McpLogicalLed::CWS_A:
            return static_cast<uint8_t>(Pap3Led::CWS_A);
        case McpLogicalLed::CMD_B:
            return static_cast<uint8_t>(Pap3Led::CMD_B);
        case McpLogicalLed::CWS_B:
            return static_cast<uint8_t>(Pap3Led::CWS_B);
        case McpLogicalLed::AT_ARM:
            return static_cast<uint8_t>(Pap3Led::AT_ARM);
        case McpLogicalLed::MA_CAPT:
            return static_cast<uint8_t>(Pap3Led::MA_CAPT);
        case McpLogicalLed::MA_FO:
            return static_cast<uint8_t>(Pap3Led::MA_FO);
        }
        return 0x00;
    }

    // --- Display monitoring (read-only) ---
    void registerDisplayMonitors();
    void unregisterDisplayMonitors();
    std::vector<std::string> _displayMonitoredRefs;

    struct DisplayState
    {
        bool hasInt = false;
        int lastInt = 0;
        bool hasFlt = false;
        float lastFlt = 0.0f;
    };
    std::map<McpDisplayItem, DisplayState> _displayCache;

    // --- LCD TX state ---
    uint8_t _lcdSeq = 1; // rolling sequence byte (never 0)
    bool _lcdDirty = false;

    // LCD transaction helpers (payload starts at absolute offset 0x19)
    void sendLcd38WithPayload(const std::vector<uint8_t> &payloadAfter0x19);
    void sendLcd38Empty();
    void sendLcd2ACommit();
    void sendLcdTransaction(const std::vector<uint8_t> &payloadAfter0x19);

    // Build payload from cached values and push immediately
    void onDisplayChanged();
    std::vector<uint8_t> buildLcdPayloadFromCache() const;

    bool _lastSpdMach = false;           // remembers last computed IAS/MACH heuristic
    mutable std::string _spdRefName;     // resolved once
    void refreshSpdFromDatarefAndPush(); // force re-read + push

    // One-shot FL to re-read VVI ~200ms after VS LED turns ON
    static float PAP3_ReevalVSFL(float inElapsedSinceLastCall,
                                 float inElapsedTimeSinceLastFlightLoop,
                                 int inCounter,
                                 void *inRefcon);

    // Force re-read of VVI and push LCD
    void refreshVviFromDatarefAndPush();

    // Cached name of the VVI dataref (resolved once from profile bindings)
    mutable std::string _vviRefName;

    // --- Input decoding (PAP3 -> profile events) ---
    void handleInputReport(const uint8_t *r, int len);
    void decodeButtons(const uint8_t *r, int len, const uint8_t *prev);
    void decodeEncoders(const uint8_t *r, int len, const uint8_t *prev);
    void decodeSwitches(const uint8_t *r, int len);

    static constexpr uint8_t kInputHeader = 0x01;
    static constexpr int kInputBytes = 0x20;

    struct InputState
    {
        bool hasLast = false;
        uint8_t last[kInputBytes] = {0};
    } _inp;
    
public:
    ProductPAP3MCP(HIDDeviceHandle hidDevice,
                   uint16_t vendorId,
                   uint16_t productId,
                   std::string vendorName,
                   std::string productName);
    ~ProductPAP3MCP() override;

    const char *classIdentifier() override;
    bool connect() override;
    void disconnect() override;
    void update() override;
    void forceLcdRefresh();
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;

    // LCD API (safe to call; LEDs are unaffected)
    void initializeDisplays();
    void setLedBrightness(int logicalLedId, uint8_t brightness); // PAP3 LEDs are on/off only
    static void initializeMenu();
    static bool getShowLcdLabels();
    static void toggleShowLcdLabels();
};

#endif // PRODUCT_PAP3_MCP_H
