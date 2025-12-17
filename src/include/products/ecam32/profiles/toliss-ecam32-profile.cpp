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
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AnnunMode");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/ECPAvail", [this, product](bool enabled) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AnnunMode");
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AnnunMode", [this](int annunMode) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/CLRillum");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDENG");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDBLEED");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDPRESS");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDELEC");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDHYD");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDFUEL");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDAPU");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDCOND");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDDOOR");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDWHEEL");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDFCTL");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDSTATUS");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/CLRillum", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::CLR_LEFT, enabled || isAnnunTest() ? 1 : 0);
        product->setLedBrightness(ECAM32Led::CLR_RIGHT, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDENG", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::ENG, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDBLEED", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::BLEED, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDPRESS", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::PRESS, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDELEC", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::ELEC, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDHYD", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::HYD, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDFUEL", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::FUEL, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDAPU", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::APU, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDCOND", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::COND, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDDOOR", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::DOOR, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDWHEEL", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::WHEEL, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDFCTL", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::F_CTL, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDSTATUS", [this, product](bool enabled) {
        product->setLedBrightness(ECAM32Led::STS, enabled || isAnnunTest() ? 1 : 0);
    });
}

TolissECAM32Profile::~TolissECAM32Profile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/ECPAvail");
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

bool TolissECAM32Profile::isAnnunTest() {
    return Dataref::getInstance()->get<int>("AirbusFBW/AnnunMode") == 2 && Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
}
