#include "ff777-pdc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-pdc.h"

#include <algorithm>
#include <cmath>

FF777PDCProfile::FF777PDCProfile(ProductPDC *product) : PDCAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/ckpt/lights/glareshield", [product](float brightness) {
        bool hasPower = Dataref::getInstance()->getCached<bool>("1-sim/output/mcp/ok");
        uint8_t target = hasPower ? brightness * 255 : 0;
        product->setLedBrightness(PDCLed::BACKLIGHT, target);

        product->forceStateSync();
    });

    // We abuse the GPU hatch dataref to trigger an update when the UI is closed.
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/anim/hatchGPU", [product](bool gpuHatchOpen) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/output/mcp/ok");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/output/mcp/ok", [product](bool hasPower) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lights/glareshield");
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/ckpt/indLightTestSwitch/anim", [this, product](int isTest) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/output/mcp/ok");
    });
}

FF777PDCProfile::~FF777PDCProfile() {
    Dataref::getInstance()->unbind("1-sim/ckpt/lights/glareshield");
    Dataref::getInstance()->unbind("1-sim/anim/hatchGPU");
    Dataref::getInstance()->unbind("1-sim/output/mcp/ok");
    Dataref::getInstance()->unbind("1-sim/ckpt/indLightTestSwitch/anim");
}

bool FF777PDCProfile::IsEligible() {
    // FF777 datarefs that don't exist on the FF767
    return Dataref::getInstance()->exists("1-sim/ckpt/mcpApLButton/anim") &&
           Dataref::getInstance()->exists("1-sim/output/mcp/ok");
}

const std::unordered_map<uint16_t, PDCButtonDef> &FF777PDCProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, PDCButtonDef> buttons = {
        {0, {"FPV", "1-sim/command/cptHsiFpvButton_button"}},
        {1, {"MTRS", "1-sim/command/cptHsiMtrsButton_button"}},
        {2, {"VSD", ""}},
        {3, {"WXR", "1-sim/command/cptHsiWxrButton_button"}},
        {4, {"STA", "1-sim/command/cptHsiStaButton_button"}},
        {5, {"WPT", "1-sim/command/cptHsiWptButton_button"}},
        {6, {"ARPT", "1-sim/command/cptHsiArptButton_button"}},
        {7, {"DATA", "1-sim/command/cptHsiDataButton_button"}},
        {8, {"POS", "1-sim/command/cptHsiPosButton_button"}},
        {9, {"TERR", "1-sim/command/cptHsiTerrButton_button"}},
        {10, {"LEFT VOR1", "1-sim/ckpt/cptHsiVorLSwitch/anim,1-sim/command/cptHsiVorLSwitch_button,1-sim/command/cptHsiVorLSwitch_trigger", PDCDatarefType::SET_VALUE_USING_COMMANDS, 1.0}}, //1-sim/ckpt/cptHsiVorLSwitch/anim -1 and 0 and 1 for VOR
        {11, {"LEFT OFF", "1-sim/ckpt/cptHsiVorLSwitch/anim,1-sim/command/cptHsiVorLSwitch_button,1-sim/command/cptHsiVorLSwitch_trigger", PDCDatarefType::SET_VALUE_USING_COMMANDS, 0.0}},
        {12, {"LEFT ADF1", "1-sim/ckpt/cptHsiVorLSwitch/anim,1-sim/command/cptHsiVorLSwitch_button,1-sim/command/cptHsiVorLSwitch_trigger", PDCDatarefType::SET_VALUE_USING_COMMANDS, -1.0}},
        {13, {"RIGHT VOR2", "1-sim/ckpt/cptHsiVorRSwitch/anim,1-sim/command/cptHsiVorRSwitch_button,1-sim/command/cptHsiVorRSwitch_trigger", PDCDatarefType::SET_VALUE_USING_COMMANDS, 1.0}},
        {14, {"RIGHT OFF", "1-sim/ckpt/cptHsiVorRSwitch/anim,1-sim/command/cptHsiVorRSwitch_button,1-sim/command/cptHsiVorRSwitch_trigger", PDCDatarefType::SET_VALUE_USING_COMMANDS, 0.0}},
        {15, {"RIGHT ADF2", "1-sim/ckpt/cptHsiVorRSwitch/anim,1-sim/command/cptHsiVorRSwitch_button,1-sim/command/cptHsiVorRSwitch_trigger", PDCDatarefType::SET_VALUE_USING_COMMANDS, -1.0}},
        {16, {"BARO RST", "1-sim/command/cptHsiRstButton_button"}},
        {17, {"VOR MAP CTR", "1-sim/command/cptHsiCtrButton_button"}},
        {18, {"RANGE TFC", "1-sim/command/cptHsiTfcButton_button"}},
        {19, {"BARO STD", "1-sim/command/cptHsiStdButton_button"}},
        {20, {"PDC3M RANGE MINUS", "1-sim/command/cptHsiRangeSwitch_switch-"}},
        {21, {"PDC3M RANGE PLUS", "1-sim/command/cptHsiRangeSwitch_switch+"}},
        {22, {"BARO knob left fast", "1-sim/command/cptHsiBaroRotary_rotary-", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {23, {"BARO knob right fast", "1-sim/command/cptHsiBaroRotary_rotary+", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {24, {"MINS RADIO", "1-sim/command/cptHsiMinsModeRotary_set_0"}},
        {25, {"MINS BARO", "1-sim/command/cptHsiMinsModeRotary_set_1"}},
        {26, {"Baro inHg", "1-sim/command/cptHsiBaroModeRotary_set_0"}},
        {27, {"Baro HPA", "1-sim/command/cptHsiBaroModeRotary_set_1"}},
        {28, {"Map APP", "1-sim/command/cptHsiModeSwitch_set_0"}},
        {29, {"Map VOR", "1-sim/command/cptHsiModeSwitch_set_1"}},
        {30, {"Map MAP", "1-sim/command/cptHsiModeSwitch_set_2"}},
        {31, {"Map PLN", "1-sim/command/cptHsiModeSwitch_set_3"}},
        {32, {"Mins knob left fast", "1-sim/command/cptHsiMinsRotary_rotary-", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {33, {"Mins knob left slow", "1-sim/command/cptHsiMinsRotary_rotary-", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {34, {"Mins knob center", ""}},
        {35, {"Mins knob right slow", "1-sim/command/cptHsiMinsRotary_rotary+", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {36, {"Mins knob right fast", "1-sim/command/cptHsiMinsRotary_rotary+", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {37, {"BARO knob left slow", "1-sim/command/cptHsiBaroRotary_rotary-", PDCDatarefType::EXECUTE_CMD_PHASED}},
        {38, {"BARO knob center", ""}},
        {39, {"BARO knob right slow", "1-sim/command/cptHsiBaroRotary_rotary+", PDCDatarefType::EXECUTE_CMD_PHASED}},
    };

    return buttons;
}

void FF777PDCProfile::buttonPressed(const PDCButtonDef *button, XPLMCommandPhase phase) {
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
        auto fullUpCmd = parts[1];
        auto moveOneDownCmd = parts[2];

        datarefManager->executeCommand(fullUpCmd.c_str());
        // Due to the command above, current value is now 1. We need to move down to the target value.

        int steps = 1 - static_cast<int>(button->value);
        for (int i = 0; i < std::abs(steps); i++) {
            datarefManager->executeCommand(moveOneDownCmd.c_str());
        }

    } else if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::SET_VALUE) {
        datarefManager->set<float>(button->dataref.c_str(), button->value);

    } else if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    } else if (phase == xplm_CommandBegin && button->datarefType == PDCDatarefType::EXECUTE_CMD_PHASED) {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
    }
}
