#include "zibo-pdc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-pdc.h"

#include <algorithm>
#include <cmath>

ZiboPDCProfile::ZiboPDCProfile(ProductPDC *product) : PDCAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/panel_brightness", [this, product](std::vector<float> panelBrightness) {
        if (panelBrightness.size() < 1) {
            return;
        }

        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
        bool hasMainBus = Dataref::getInstance()->get<bool>("laminar/B738/electric/main_bus");
        float ratio = std::clamp(hasMainBus ? panelBrightness[0] : 0.5f, 0.0f, 1.0f);
        uint8_t brightness = hasPower ? ratio * 255 : 0;
        product->setLedBrightness(PDCLed::BACKLIGHT, brightness);

        product->forceStateSync();
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [product](bool hasPower) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/panel_brightness");
    });

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/dspl_light_test", [this](std::vector<float> displayTest) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/panel_brightness");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/B738/electric/main_bus", [product](bool hasPower) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/electrical/avionics_on");
    });
}

ZiboPDCProfile::~ZiboPDCProfile() {
    Dataref::getInstance()->unbind("laminar/B738/electric/panel_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("laminar/B738/dspl_light_test");
    Dataref::getInstance()->unbind("laminar/B738/electric/main_bus");
}

bool ZiboPDCProfile::IsEligible() {
    return Dataref::getInstance()->exists("laminar/B738/autopilot/mcp_speed_dial_kts_mach");
}

const std::unordered_map<uint16_t, PDCButtonDef> &ZiboPDCProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, PDCButtonDef> buttons = {
        {0, {"FPV", "laminar/B738/EFIS_control/capt/push_button/fpv_press"}},
        {1, {"MTRS", "laminar/B738/EFIS_control/capt/push_button/mtrs_press"}},
        {2, {"VSD", ""}},
        {3, {"WXR", "laminar/B738/EFIS_control/capt/push_button/wxr_press"}},
        {4, {"STA", "laminar/B738/EFIS_control/capt/push_button/sta_press"}},
        {5, {"WPT", "laminar/B738/EFIS_control/capt/push_button/wpt_press"}},
        {6, {"ARPT", "laminar/B738/EFIS_control/capt/push_button/arpt_press"}},
        {7, {"DATA", "laminar/B738/EFIS_control/capt/push_button/data_press"}},
        {8, {"POS", "laminar/B738/EFIS_control/capt/push_button/pos_press"}},
        {9, {"TERR", "laminar/B738/EFIS_control/capt/push_button/terr_press"}},
        {10, {"LEFT VOR1", "laminar/B738/EFIS_control/capt/vor1_off_pos,laminar/B738/EFIS_control/capt/vor1_off_dn,laminar/B738/EFIS_control/capt/vor1_off_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, 1.0}},
        {11, {"LEFT OFF", "laminar/B738/EFIS_control/capt/vor1_off_pos,laminar/B738/EFIS_control/capt/vor1_off_dn,laminar/B738/EFIS_control/capt/vor1_off_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, 0.0}},
        {12, {"LEFT ADF1", "laminar/B738/EFIS_control/capt/vor1_off_pos,laminar/B738/EFIS_control/capt/vor1_off_dn,laminar/B738/EFIS_control/capt/vor1_off_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, -1.0}},
        {13, {"RIGHT VOR2", "laminar/B738/EFIS_control/capt/vor2_off_pos,laminar/B738/EFIS_control/capt/vor2_off_dn,laminar/B738/EFIS_control/capt/vor2_off_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, 1.0}},
        {14, {"RIGHT OFF", "laminar/B738/EFIS_control/capt/vor2_off_pos,laminar/B738/EFIS_control/capt/vor2_off_dn,laminar/B738/EFIS_control/capt/vor2_off_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, 0.0}},
        {15, {"RIGHT ADF2", "laminar/B738/EFIS_control/capt/vor2_off_pos,laminar/B738/EFIS_control/capt/vor2_off_dn,laminar/B738/EFIS_control/capt/vor2_off_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, -1.0}},
        {16, {"BARO RST", "laminar/B738/EFIS_control/capt/push_button/rst_press"}},
        {17, {"VOR MAP CTR", "laminar/B738/EFIS_control/capt/push_button/ctr_press"}},
        {18, {"RANGE TFC", "laminar/B738/EFIS_control/capt/push_button/tfc_press"}},
        {19, {"BARO STD", "laminar/B738/EFIS_control/capt/push_button/std_press"}},
        {20, {"PDC3M RANGE MINUS", "laminar/B738/EFIS_control/capt/map_range_dn"}}, // PDC3N - laminar/B738/EFIS/capt/map_range,0,1,2,3,4,5,6,7
        {21, {"PDC3M RANGE PLUS", "laminar/B738/EFIS_control/capt/map_range_up"}},
        {22, {"BARO knob left fast", "laminar/B738/pilot/barometer_dn_fast", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {23, {"BARO knob right fast", "laminar/B738/pilot/barometer_up_fast", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {24, {"MINS RADIO", "laminar/B738/EFIS_control/cpt/minimums,laminar/B738/EFIS_control/cpt/minimums_up,laminar/B738/EFIS_control/cpt/minimums_dn", PDCDatarefType::SET_VALUE_USING_COMMANDS, 0.0}}, // Caution, up and down are inverted
        {25, {"MINS BARO", "laminar/B738/EFIS_control/cpt/minimums,laminar/B738/EFIS_control/cpt/minimums_up,laminar/B738/EFIS_control/cpt/minimums_dn", PDCDatarefType::SET_VALUE_USING_COMMANDS, 1.0}},  // Caution, up and down are inverted
        {26, {"Baro inHg", "laminar/B738/EFIS_control/capt/baro_in_hpa,laminar/B738/EFIS_control/capt/baro_in_hpa_dn,laminar/B738/EFIS_control/capt/baro_in_hpa_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, 0.0}},
        {27, {"Baro HPA", "laminar/B738/EFIS_control/capt/baro_in_hpa,laminar/B738/EFIS_control/capt/baro_in_hpa_dn,laminar/B738/EFIS_control/capt/baro_in_hpa_up", PDCDatarefType::SET_VALUE_USING_COMMANDS, 1.0}},
        {28, {"Map APP", "laminar/B738/EFIS_control/capt/map_mode_pos", PDCDatarefType::SET_VALUE, 0.0}},
        {29, {"Map VOR", "laminar/B738/EFIS_control/capt/map_mode_pos", PDCDatarefType::SET_VALUE, 1.0}},
        {30, {"Map MAP", "laminar/B738/EFIS_control/capt/map_mode_pos", PDCDatarefType::SET_VALUE, 2.0}},
        {31, {"Map PLN", "laminar/B738/EFIS_control/capt/map_mode_pos", PDCDatarefType::SET_VALUE, 3.0}},
        {32, {"Mins knob left fast", "laminar/B738/pfd/dh_pilot_dn_fast", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {33, {"Mins knob left slow", "laminar/B738/pfd/dh_pilot_dn_slow", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {34, {"Mins knob center", ""}},
        {35, {"Mins knob right slow", "laminar/B738/pfd/dh_pilot_up_slow", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {36, {"Mins knob right fast", "laminar/B738/pfd/dh_pilot_up_fast", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {37, {"BARO knob left slow", "laminar/B738/pilot/barometer_dn_slow", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {38, {"BARO knob center", ""}},
        {39, {"BARO knob right slow", "laminar/B738/pilot/barometer_up_slow", PDCDatarefType::EXECUTE_CMD_PHASED}},
    };

    return buttons;
}

void ZiboPDCProfile::buttonPressed(const PDCButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();

    if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::SET_VALUE_USING_COMMANDS) {
        std::stringstream ss(button->dataref);
        std::string item;
        std::vector<std::string> parts;
        while (std::getline(ss, item, ',')) {
            parts.push_back(item);
        }

        auto posRef = parts[0];
        auto leftCmd = parts[1];
        auto rightCmd = parts[2];

        int current = datarefManager->get<int>(posRef.c_str());
        int target = static_cast<int>(button->value);

        if (current < target) {
            for (int i = current; i < target; i++) {
                datarefManager->executeCommand(rightCmd.c_str());
            }
        } else if (current > target) {
            for (int i = current; i > target; i--) {
                datarefManager->executeCommand(leftCmd.c_str());
            }
        }

    } else if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::SET_VALUE) {
        datarefManager->set<float>(button->dataref.c_str(), button->value);

    } else if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    } else if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::EXECUTE_CMD_PHASED) {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
    }
}
