#include "toliss-ecam32-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-ecam32.h"

#include <algorithm>
#include <cmath>

TolissECAM32Profile::TolissECAM32Profile(ProductECAM32 *product) : ECAM32AircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        uint8_t backlightBrightness = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255 : 0;

        product->setLedBrightness(ECAM32Led::BACKLIGHT, backlightBrightness);
        product->setLedBrightness(ECAM32Led::EMER_CANC_BRIGHTNESS, backlightBrightness);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/CLRillum", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::CLR_LEFT, enabled ? 1 : 0);
        product->setLedBrightness(ECAM32Led::CLR_RIGHT, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDENG", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::ENG, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDBLEED", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::BLEED, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDPRESS", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::PRESS, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDELEC", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::ELEC, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDHYD", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::HYD, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDFUEL", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::FUEL, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDAPU", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::APU, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDCOND", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::COND, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDDOOR", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::DOOR, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDWHEEL", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::WHEEL, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDFCTL", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::F_CTL, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDSTATUS", [product](bool enabled) {
        product->setLedBrightness(ECAM32Led::STS, enabled ? 1 : 0);
    });
}

TolissECAM32Profile::~TolissECAM32Profile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/CLRillum");
    Dataref::getInstance()->unbind("AirbusFBW/SDENG");
    Dataref::getInstance()->unbind("AirbusFBW/SDBLEED");
    Dataref::getInstance()->unbind("AirbusFBW/SDPRESS");
    Dataref::getInstance()->unbind("AirbusFBW/SDELEC");
    Dataref::getInstance()->unbind("AirbusFBW/SDHYD");
    Dataref::getInstance()->unbind("AirbusFBW/SDFUEL");
    Dataref::getInstance()->unbind("AirbusFBW/SDAPU");
    Dataref::getInstance()->unbind("AirbusFBW/SDCOND");
    Dataref::getInstance()->unbind("AirbusFBW/SDDOOR");
    Dataref::getInstance()->unbind("AirbusFBW/SDWHEEL");
    Dataref::getInstance()->unbind("AirbusFBW/SDFCTL");
    Dataref::getInstance()->unbind("AirbusFBW/SDSTATUS");
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
