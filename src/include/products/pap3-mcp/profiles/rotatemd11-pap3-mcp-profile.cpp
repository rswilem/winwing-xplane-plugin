#include "rotatemd11-pap3-mcp-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-pap3-mcp.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <XPLMUtilities.h>

RotateMD11PAP3MCPProfile::RotateMD11PAP3MCPProfile(ProductPAP3MCP *product) :
    PAP3MCPAircraftProfile(product) {
    // Monitor power and brightness - MD-11 specific behavior
    Dataref::getInstance()->monitorExistingDataref<bool>("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd", [this, product](bool hasPower) {
        if (hasPower) {
            // LCD at full brightness when powered
            product->setLedBrightness(PAP3MCPLed::LCD_BACKLIGHT, 255);

            // Button backlight: 0 at startup, then adjustable via panel lights
            if (!backlightInitialized) {
                product->setLedBrightness(PAP3MCPLed::BACKLIGHT, 0);
                product->setLedBrightness(PAP3MCPLed::OVERALL_LED_BRIGHTNESS, 0);
                backlightInitialized = true;
            } else {
                // Use FGS panel brightness for button backlight
                float panelLights = Dataref::getInstance()->getCached<float>("Rotate/aircraft/systems/light_fgs_panel_brt_ratio");
                uint8_t brightness = std::clamp(panelLights, 0.0f, 1.0f) * 255.0f;
                product->setLedBrightness(PAP3MCPLed::BACKLIGHT, brightness);
                product->setLedBrightness(PAP3MCPLed::OVERALL_LED_BRIGHTNESS, brightness);
            }
        } else {
            // No power - turn off all displays
            product->setLedBrightness(PAP3MCPLed::BACKLIGHT, 0);
            product->setLedBrightness(PAP3MCPLed::LCD_BACKLIGHT, 0);
            product->setLedBrightness(PAP3MCPLed::OVERALL_LED_BRIGHTNESS, 0);
        }

        product->forceStateSync();
    });

    Dataref::getInstance()->monitorExistingDataref<float>("Rotate/aircraft/systems/light_fgs_panel_brt_ratio", [](float panelLights) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd");
    });

    // MD-11 MCP has NO LED annunciators - all LEDs disabled by not setting up monitors
    // The real MD-11 uses a different display system (not LED buttons)
}

RotateMD11PAP3MCPProfile::~RotateMD11PAP3MCPProfile() {
    Dataref::getInstance()->unbind("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd");
    Dataref::getInstance()->unbind("Rotate/aircraft/systems/light_fgs_panel_brt_ratio");
}

bool RotateMD11PAP3MCPProfile::IsEligible() {
    return Dataref::getInstance()->exists("Rotate/aircraft/systems/gcp_alt_presel_ft");
}

const std::vector<std::string> &RotateMD11PAP3MCPProfile::displayDatarefs() const {
    static std::vector<std::string> datarefs = {
        "Rotate/aircraft/systems/elec_dc_batt_bus_pwrd",
        "Rotate/aircraft/systems/light_fgs_panel_brt_ratio",
        "Rotate/aircraft/systems/gcp_spd_presel_ias",
        "Rotate/aircraft/systems/gcp_spd_presel_mach",
        "Rotate/aircraft/systems/gcp_active_ias_mach_mode",
        "Rotate/aircraft/systems/gcp_hdg_presel_deg",
        "Rotate/aircraft/systems/gcp_alt_presel_ft",
        "Rotate/aircraft/systems/gcp_vs_sel_fpm",
        "Rotate/aircraft/systems/afs_fms_spd_engaged",
        "Rotate/aircraft/systems/afs_roll_mode",
        "Rotate/aircraft/systems/gcp_hdg_trk_presel_set",
        "sim/cockpit/radios/nav1_obs_degm",
        "sim/cockpit/radios/nav2_obs_degm",
    };
    return datarefs;
}

const std::unordered_map<uint16_t, PAP3MCPButtonDef> &RotateMD11PAP3MCPProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, PAP3MCPButtonDef> buttons = {
        // Row 1 (byte 0x01) - Main autopilot mode buttons
        // Note: MD-11 uses dataref writes instead of commands for buttons
        {1, {"SPEED (FMS SPD)", "Rotate/aircraft/controls/fgs_fms_spd", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {2, {"PROF (VNAV)", "Rotate/aircraft/controls/fgs_prof", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {3, {"LVL CHG", "Rotate/aircraft/controls/fgs_alt_mode_sel", PAP3MCPDatarefType::SET_VALUE, -1.0}},
        {4, {"HDG SEL", "Rotate/aircraft/controls/fgs_hdg_mode_sel", PAP3MCPDatarefType::SET_VALUE, -1.0}},
        {5, {"NAV (LNAV)", "Rotate/aircraft/controls/fgs_nav", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {7, {"APPR/LAND (APP)", "Rotate/aircraft/controls/fgs_appr_land", PAP3MCPDatarefType::SET_VALUE, 1.0}},

        // Row 2 (byte 0x02) - Additional autopilot buttons
        {8, {"ALT HOLD", "Rotate/aircraft/controls/fgs_alt_mode_sel", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {9, {"V/S (FPA)", "Rotate/aircraft/controls/fgs_vs_fpa", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {10, {"CMD A (AP)", "Rotate/aircraft/controls/fgs_autoflight", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {11, {"CWS A (AFS OVR 1)", "Rotate/aircraft/controls/fgs_afs_ovrd_off_1", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {12, {"CMD B (AP)", "Rotate/aircraft/controls/fgs_autoflight", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {13, {"CWS B (AFS OVR 2)", "Rotate/aircraft/controls/fgs_afs_ovrd_off_2", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {14, {"C/O (IAS/MACH)", "Rotate/aircraft/controls/fgs_ias_mach", PAP3MCPDatarefType::SET_VALUE, 1.0}},
        {15, {"SPD INTV", "Rotate/aircraft/controls/fgs_spd_sel_mode", PAP3MCPDatarefType::SET_VALUE, -1.0}},

        // Row 3 (byte 0x03)
        {16, {"ALT INTV", "Rotate/aircraft/controls/fgs_alt_mode_sel", PAP3MCPDatarefType::SET_VALUE, -1.0}}};
    return buttons;
}

const std::vector<PAP3MCPEncoderDef> &RotateMD11PAP3MCPProfile::encoderDefs() const {
    static std::vector<PAP3MCPEncoderDef> encoders = {
        {0, "CRS CAPT", "sim/autopilot/heading_up", "sim/autopilot/heading_down"},
        {1, "SPD", "Rotate/aircraft/controls_c/fgs_spd_sel_up", "Rotate/aircraft/controls_c/fgs_spd_sel_dn"},
        {2, "HDG", "Rotate/aircraft/controls_c/fgs_hdg_sel_up", "Rotate/aircraft/controls_c/fgs_hdg_sel_dn"},
        {3, "ALT", "Rotate/aircraft/controls_c/fgs_alt_sel_up", "Rotate/aircraft/controls_c/fgs_alt_sel_dn"},
        {4, "V/S (Pitch)", "Rotate/aircraft/controls_c/fgs_pitch_sel_up", "Rotate/aircraft/controls_c/fgs_pitch_sel_dn"},
        {5, "CRS FO", "sim/autopilot/heading_up", "sim/autopilot/heading_down"}};
    return encoders;
}

void RotateMD11PAP3MCPProfile::updateDisplayData(PAP3MCPDisplayData &data) {
    auto dataref = Dataref::getInstance();
    data.showLabels = true;             // MD-11 MCP has labels on the display
    data.showDashesWhenInactive = true; // Show dashes (---) when displays are inactive
    data.showLabelsWhenInactive = true; // Show labels even when displays are inactive
    data.showCourse = false;            // MD-11 doesn't have CRS displays on the MCP
    bool hasPower = dataref->getCached<bool>("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd");
    data.displayEnabled = hasPower;

    if (!hasPower) {
        return;
    }

    // Speed - check if IAS or MACH mode
    int iasMachMode = dataref->getCached<int>("Rotate/aircraft/systems/gcp_active_ias_mach_mode");
    if (iasMachMode == 0) {
        // IAS mode
        data.speed = dataref->getCached<float>("Rotate/aircraft/systems/gcp_spd_presel_ias");
    } else {
        // MACH mode
        data.speed = dataref->getCached<float>("Rotate/aircraft/systems/gcp_spd_presel_mach");
    }

    // Control SPD LCD visibility based on FMS SPD engagement
    // When FMS SPD is engaged (value = 1), hide the SPD display
    int fmsSpdEngaged = dataref->getCached<int>("Rotate/aircraft/systems/afs_fms_spd_engaged");
    data.speedVisible = (fmsSpdEngaged != 1);

    // Heading
    int hdgRaw = dataref->getCached<int>("Rotate/aircraft/systems/gcp_hdg_presel_deg");
    data.heading = hdgRaw % 360;
    if (data.heading < 0) {
        data.heading += 360;
    }

    // Control HDG LCD visibility
    int hdgTrkSel = dataref->getCached<int>("Rotate/aircraft/systems/gcp_hdg_trk_presel_set");
    data.headingVisible = (hdgTrkSel == 1);

    // Altitude
    data.altitude = dataref->getCached<int>("Rotate/aircraft/systems/gcp_alt_presel_ft");

    // Vertical speed - only show when V/S is non-zero
    data.verticalSpeed = dataref->getCached<float>("Rotate/aircraft/systems/gcp_vs_sel_fpm");
    data.verticalSpeedVisible = (std::abs(data.verticalSpeed) > 0.5f);

    // Course - MD-11 doesn't have CRS displays on the MCP (using standard datarefs for reference)
    data.crsCapt = dataref->getCached<int>("sim/cockpit/radios/nav1_obs_degm");
    data.crsFo = dataref->getCached<int>("sim/cockpit/radios/nav2_obs_degm");

    // MD-11 doesn't have special digit flags
    data.digitA = false;
    data.digitB = false;
    data.displayTest = false;

    // MD-11 MCP has NO LED annunciators
    data.ledN1 = false;
    data.ledSpeed = false;
    data.ledVNAV = false;
    data.ledLvlChg = false;
    data.ledHdgSel = false;
    data.ledLNAV = false;
    data.ledVORLOC = false;
    data.ledAPP = false;
    data.ledAltHld = false;
    data.ledVS = false;
    data.ledCmdA = false;
    data.ledCwsA = false;
    data.ledCmdB = false;
    data.ledCwsB = false;
    data.ledATArm = false;
    data.ledMaCapt = false;
    data.ledMaFO = false;
}

void RotateMD11PAP3MCPProfile::buttonPressed(const PAP3MCPButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty()) {
        return;
    }

    // MD-11 uses dataref writes for buttons (momentary: press=1/value, release=0)
    if (button->datarefType == PAP3MCPDatarefType::SET_VALUE) {
        if (phase == xplm_CommandBegin) {
            // Press: write the specified value
            Dataref::getInstance()->set<int>(button->dataref.c_str(), static_cast<int>(button->value));
        } else if (phase == xplm_CommandEnd) {
            // Release: write 0
            Dataref::getInstance()->set<int>(button->dataref.c_str(), 0);
        }
    } else if (button->datarefType == PAP3MCPDatarefType::EXECUTE_CMD_ONCE) {
        if (phase == xplm_CommandBegin) {
            Dataref::getInstance()->executeCommand(button->dataref.c_str());
        }
    } else if (button->datarefType == PAP3MCPDatarefType::EXECUTE_CMD_BEGIN_END) {
        if (phase == xplm_CommandBegin) {
            Dataref::getInstance()->executeCommand(button->dataref.c_str(), xplm_CommandBegin);
        } else if (phase == xplm_CommandEnd) {
            Dataref::getInstance()->executeCommand(button->dataref.c_str(), xplm_CommandEnd);
        }
    }
}

void RotateMD11PAP3MCPProfile::encoderRotated(const PAP3MCPEncoderDef *encoder, int8_t delta) {
    if (!encoder || delta == 0) {
        return;
    }

    const char *cmd = (delta > 0) ? encoder->incCmd.c_str() : encoder->decCmd.c_str();
    int steps = std::abs(static_cast<int>(delta));

    for (int i = 0; i < steps; i++) {
        Dataref::getInstance()->executeCommand(cmd);
    }
}
