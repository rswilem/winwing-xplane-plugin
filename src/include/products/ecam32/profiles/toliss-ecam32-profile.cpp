#include "toliss-ecam32-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-ecam32.h"

#include <algorithm>
#include <cmath>

TolissECAM32Profile::TolissECAM32Profile(ProductECAM32 *product) : ECAM32AircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
        bool ecpAvailable = Dataref::getInstance()->get<bool>("AirbusFBW/ECPAvail");
        uint8_t backlightBrightness = hasPower && ecpAvailable ? brightness * 255 : 0;

        product->setLedBrightness(ECAM32Led::BACKLIGHT, backlightBrightness);
        product->setLedBrightness(ECAM32Led::EMER_CANC_BRIGHTNESS, backlightBrightness);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/OHPLightsATA31_Raw");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/ECPAvail", [this, product](bool enabled) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/OHPLightsATA31_Raw");
    });

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/OHPLightsATA31_Raw", [product](std::vector<float> panelLights) {
        if (panelLights.size() < 45) {
            return;
        }

        product->setLedBrightness(ECAM32Led::ENG, panelLights[30] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::BLEED, panelLights[31] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::PRESS, panelLights[32] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::ELEC, panelLights[33] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::HYD, panelLights[34] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::FUEL, panelLights[35] ? 1 : 0);

        product->setLedBrightness(ECAM32Led::APU, panelLights[36] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::COND, panelLights[37] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::DOOR, panelLights[38] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::WHEEL, panelLights[39] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::F_CTL, panelLights[40] ? 1 : 0);

        product->setLedBrightness(ECAM32Led::STS, panelLights[41] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::CLR_LEFT, panelLights[42] ? 1 : 0);
        product->setLedBrightness(ECAM32Led::CLR_RIGHT, panelLights[43] ? 1 : 0);
    });
}

TolissECAM32Profile::~TolissECAM32Profile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/ECPAvail");
    Dataref::getInstance()->unbind("AirbusFBW/OHPLightsATA31_Raw");
}

bool TolissECAM32Profile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::unordered_map<uint16_t, ECAM32ButtonDef> &TolissECAM32Profile::buttonDefs() const {
    static const std::unordered_map<uint16_t, ECAM32ButtonDef> buttons = {
        {0, {"EMPTY 1", ""}},
        {1, {"TOCONFIG", "AirbusFBW/TOConfigPress"}},
        {2, {"EMPTY 2", ""}},
        {3, {"EMER", "AirbusFBW/EmerCancel"}},
        {4, {"ENG", "AirbusFBW/ECP/SelectEnginePage"}},
        {5, {"BLEED", "AirbusFBW/ECP/SelectBleedPage"}},
        {6, {"PRESS", "AirbusFBW/ECP/SelectPressPage"}},
        {7, {"ELEC", "AirbusFBW/ECP/SelectElecACPage"}},
        {8, {"HYD", "AirbusFBW/ECP/SelectHydraulicPage"}},
        {9, {"FUEL", "AirbusFBW/ECP/SelectFuelPage"}},
        {10, {"APU", "AirbusFBW/ECP/SelectAPUPage"}},
        {11, {"COND", "AirbusFBW/ECP/SelectConditioningPage"}},
        {12, {"DOOR", "AirbusFBW/ECP/SelectDoorOxyPage"}},
        {13, {"WHEEL", "AirbusFBW/ECP/SelectWheelPage"}},
        {14, {"F/CTL", "AirbusFBW/ECP/SelectFlightControlPage"}},
        {15, {"ALL", "AirbusFBW/ECAMAll"}},
        {16, {"CLR LEFT", "AirbusFBW/ECP/CaptainClear"}},
        {17, {"EMPTY 3", ""}},
        {18, {"STS", "AirbusFBW/ECP/SelectStatusPage"}},
        {19, {"RCL", "AirbusFBW/ECAMRecall"}},
        {20, {"EMPTY 4", ""}},
        {21, {"CLR RIGHT", "AirbusFBW/ECP/CopilotClear"}}};
    return buttons;
}

void TolissECAM32Profile::buttonPressed(const ECAM32ButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    datarefManager->executeCommand(button->dataref.c_str(), phase);
}
