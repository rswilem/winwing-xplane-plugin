#include "toliss-agp-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-agp.h"

#include <algorithm>
#include <cmath>

TolissAGPProfile::TolissAGPProfile(ProductAGP *product) : AGPAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        uint8_t backlightBrightness = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255 : 0;

        product->setLedBrightness(AGPLed::BACKLIGHT, backlightBrightness);
        product->setLedBrightness(AGPLed::LCD_BRIGHTNESS, backlightBrightness);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NoseGearInd", [product](int indicator) {
        product->setLedBrightness(AGPLed::LDG_GEAR_UNLK_CENTER, (indicator & 1) ? 1 : 0);
        product->setLedBrightness(AGPLed::LDG_GEAR_ARROW_GREEN_CENTER, (indicator & 2) ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/LeftGearInd", [product](int indicator) {
        product->setLedBrightness(AGPLed::LDG_GEAR_UNLK_LEFT, (indicator & 1) ? 1 : 0);
        product->setLedBrightness(AGPLed::LDG_GEAR_ARROW_GREEN_LEFT, (indicator & 2) ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/RightGearInd", [product](int indicator) {
        product->setLedBrightness(AGPLed::LDG_GEAR_UNLK_RIGHT, (indicator & 1) ? 1 : 0);
        product->setLedBrightness(AGPLed::LDG_GEAR_ARROW_GREEN_RIGHT, (indicator & 2) ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/TerrainSelectedND1", [product](bool enabled) {
        if (product->terrainNDPreference == AGPTerrainNDPreference::CAPTAIN) {
            product->setLedBrightness(AGPLed::TERRAIN_ON, enabled ? 1 : 0);
        }
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/TerrainSelectedND2", [product](bool enabled) {
        if (product->terrainNDPreference == AGPTerrainNDPreference::FIRST_OFFICER) {
            product->setLedBrightness(AGPLed::TERRAIN_ON, enabled ? 1 : 0);
        }
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AutoBrkLo", [product](int brakeMinimumIndicator) {
        int brakeMediumIndicator = Dataref::getInstance()->getCached<int>("AirbusFBW/AutoBrkMed");
        int brakeMaximumIndicator = Dataref::getInstance()->getCached<int>("AirbusFBW/AutoBrkMax");
        product->setLedBrightness(AGPLed::AUTOBRK_LO_ON, brakeMinimumIndicator >= 1);
        product->setLedBrightness(AGPLed::AUTOBRK_MED_ON, brakeMediumIndicator >= 1);
        product->setLedBrightness(AGPLed::AUTOBRK_HI_ON, brakeMaximumIndicator >= 1);
        product->setLedBrightness(AGPLed::AUTOBRK_DECEL_LO, brakeMinimumIndicator == 2);
        product->setLedBrightness(AGPLed::AUTOBRK_DECEL_MED, brakeMediumIndicator == 2);
        product->setLedBrightness(AGPLed::AUTOBRK_DECEL_HI, brakeMaximumIndicator == 2);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AutoBrkMed", [](int indicator) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AutoBrkLo");
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AutoBrkMax", [](int indicator) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AutoBrkLo");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/BrakeFan", [product](bool enabled) {
        product->setLedBrightness(AGPLed::BRAKE_FAN_ON, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/BrakeTemperatureArray", [product](std::vector<float> temperatures) {
        bool anyHot = std::any_of(temperatures.begin(), temperatures.end(), [](float temp) {
            return temp > 300.0f;
        });
        product->setLedBrightness(AGPLed::BRAKE_FAN_HOT, anyHot ? 1 : 0);
    });
}

TolissAGPProfile::~TolissAGPProfile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("AirbusFBW/NoseGearInd");
    Dataref::getInstance()->unbind("AirbusFBW/LeftGearInd");
    Dataref::getInstance()->unbind("AirbusFBW/RightGearInd");
    Dataref::getInstance()->unbind("AirbusFBW/TerrainSelectedND2");
    Dataref::getInstance()->unbind("AirbusFBW/AutoBrkLo");
    Dataref::getInstance()->unbind("AirbusFBW/AutoBrkMed");
    Dataref::getInstance()->unbind("AirbusFBW/AutoBrkMax");
    Dataref::getInstance()->unbind("AirbusFBW/BrakeFan");
    Dataref::getInstance()->unbind("AirbusFBW/BrakeTemperatureArray");
}

bool TolissAGPProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::unordered_map<uint16_t, AGPButtonDef> &TolissAGPProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, AGPButtonDef> buttons = {
        {0, {"UNKNOWN 1", ""}},
        {1, {"UNKNOWN 2", ""}},
        {2, {"UNKNOWN 3", ""}},
        {3, {"UNKNOWN 4", ""}},
        {4, {"UNKNOWN 5", ""}},
        {5, {"UNKNOWN 6", ""}},
        {6, {"UNKNOWN 7", ""}},
        {7, {"UNKNOWN 8", ""}},
        {8, {"UNKNOWN 9", ""}},
        {9, {"UNKNOWN 10", ""}},
        {10, {"UNKNOWN 11", ""}},
        {11, {"UNKNOWN 12", ""}},
        {12, {"UNKNOWN 13", ""}},
        {13, {"UNKNOWN 14", ""}},
        {14, {"UNKNOWN 15", ""}},
        {15, {"UNKNOWN 16", ""}},
        {16, {"UNKNOWN 17", ""}},
        {17, {"UNKNOWN 18", ""}},
        {18, {"UNKNOWN 19", ""}},
        {19, {"UNKNOWN 20", ""}},
        {20, {"UNKNOWN 21", ""}},
        {21, {"UNKNOWN 22", ""}}};
    return buttons;
}

void TolissAGPProfile::buttonPressed(const AGPButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    datarefManager->executeCommand(button->dataref.c_str(), phase);
}
