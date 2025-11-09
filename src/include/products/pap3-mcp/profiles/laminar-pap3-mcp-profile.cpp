#include "laminar-pap3-mcp-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-pap3-mcp.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <XPLMUtilities.h>

LaminarPAP3MCPProfile::LaminarPAP3MCPProfile(ProductPAP3MCP *product) :
    PAP3MCPAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("sim/cockpit2/electrical/instrument_brightness_ratio_manual", [product](std::vector<float> brightness) {
        if (brightness.size() >= 16) {
            bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit2/autopilot/autopilot_has_power");
            uint8_t target = hasPower ? brightness[15] * 255.0f : 0;
            product->setLedBrightness(PAP3MCPLed::BACKLIGHT, target);
            product->setLedBrightness(PAP3MCPLed::LCD_BACKLIGHT, target);
            product->forceStateSync();
        }
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit2/autopilot/autopilot_has_power", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/instrument_brightness_ratio_manual");
    });
}

LaminarPAP3MCPProfile::~LaminarPAP3MCPProfile() {
    Dataref::getInstance()->unbind("sim/cockpit2/electrical/instrument_brightness_ratio_manual");
    Dataref::getInstance()->unbind("sim/cockpit2/autopilot/autopilot_has_power");
}

bool LaminarPAP3MCPProfile::IsEligible() {
    return true;
}

const std::vector<std::string> &LaminarPAP3MCPProfile::displayDatarefs() const {
    static std::vector<std::string> datarefs = {
        "sim/cockpit2/autopilot/airspeed_dial_kts_mach",
        "sim/cockpit/autopilot/heading_mag",
        "sim/cockpit2/autopilot/altitude_dial_ft",
        "sim/cockpit2/autopilot/vvi_dial_fpm",
        "sim/cockpit/radios/nav1_obs_degm",
        "sim/cockpit/radios/nav2_obs_degm"};
    return datarefs;
}

const std::unordered_map<uint16_t, PAP3MCPButtonDef> &LaminarPAP3MCPProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, PAP3MCPButtonDef> buttons = {
        {0, {"N1", "sim/autopilot/FMS"}}, // N1 equivalent
        {1, {"SPEED", "sim/autopilot/autothrottle_on"}},
        {2, {"VNAV", "sim/autopilot/FMS"}}, // VNAV equivalent
        {3, {"LVL CHG", "sim/autopilot/level_change"}},
        {4, {"HDG SEL", "sim/autopilot/heading"}},
        {5, {"LNAV", "sim/autopilot/gpss"}}, // LNAV equivalent
        {6, {"VOR LOC", "sim/autopilot/NAV"}},
        {7, {"APP", "sim/autopilot/approach"}},

        {8, {"ALT HLD", "sim/autopilot/altitude_hold"}},
        {9, {"V/S", "sim/autopilot/vertical_speed"}},
        {10, {"CMD A", "sim/autopilot/servos_on"}},
        {11, {"CWS A", "sim/autopilot/cwsa"}},
        {12, {"CMD B", "sim/autopilot/servos2_on"}},
        {13, {"CWS B", "sim/autopilot/cwsb"}}};
    return buttons;
}

const std::vector<PAP3MCPEncoderDef> &LaminarPAP3MCPProfile::encoderDefs() const {
    static std::vector<PAP3MCPEncoderDef> encoders = {
        {0, "CRS CAPT", "sim/radios/obs1_up", "sim/radios/obs1_down"},
        {1, "SPD", "sim/autopilot/airspeed_up", "sim/autopilot/airspeed_down"},
        {2, "HDG", "sim/autopilot/heading_up", "sim/autopilot/heading_down"},
        {3, "ALT", "sim/autopilot/altitude_up", "sim/autopilot/altitude_down"},
        {4, "V/S", "sim/autopilot/vertical_speed_up", "sim/autopilot/vertical_speed_down"},
        {5, "CRS FO", "sim/radios/obs2_up", "sim/radios/obs2_down"}};
    return encoders;
}

void LaminarPAP3MCPProfile::updateDisplayData(PAP3MCPDisplayData &data) {
    // Use getCached() for performance
    data.speed = Dataref::getInstance()->getCached<float>("sim/cockpit2/autopilot/airspeed_dial_kts_mach");
    data.heading = static_cast<int>(Dataref::getInstance()->getCached<float>("sim/cockpit/autopilot/heading_mag"));
    data.altitude = static_cast<int>(Dataref::getInstance()->getCached<float>("sim/cockpit2/autopilot/altitude_dial_ft"));
    data.verticalSpeed = Dataref::getInstance()->getCached<float>("sim/cockpit2/autopilot/vvi_dial_fpm");
    data.verticalSpeedVisible = true; // Always show for Laminar
    data.crsCapt = static_cast<int>(Dataref::getInstance()->getCached<float>("sim/cockpit/radios/nav1_obs_degm"));
    data.crsFo = static_cast<int>(Dataref::getInstance()->getCached<float>("sim/cockpit/radios/nav2_obs_degm"));

    data.digitA = false;
    data.digitB = false;

    // Check if displays should be enabled based on power
    data.displayEnabled = Dataref::getInstance()->getCached<bool>("sim/cockpit2/autopilot/autopilot_has_power");
}

void LaminarPAP3MCPProfile::buttonPressed(const PAP3MCPButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty()) {
        return;
    }

    if (button->datarefType == PAP3MCPDatarefType::EXECUTE_CMD_ONCE) {
        if (phase == xplm_CommandBegin) {
            Dataref::getInstance()->executeCommand(button->dataref.c_str());
        }
    }
}

void LaminarPAP3MCPProfile::encoderRotated(const PAP3MCPEncoderDef *encoder, int8_t delta) {
    if (!encoder || delta == 0) {
        return;
    }

    const char *cmd = (delta > 0) ? encoder->incCmd.c_str() : encoder->decCmd.c_str();
    int steps = std::abs(static_cast<int>(delta));

    for (int i = 0; i < steps; i++) {
        Dataref::getInstance()->executeCommand(cmd);
    }
}
