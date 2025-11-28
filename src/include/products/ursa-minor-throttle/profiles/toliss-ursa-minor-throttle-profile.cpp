#include "toliss-ursa-minor-throttle-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-ursa-minor-throttle.h"

#include <algorithm>
#include <cmath>

TolissUrsaMinorThrottleProfile::TolissUrsaMinorThrottleProfile(ProductUrsaMinorThrottle *product) : UrsaMinorThrottleAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        uint8_t backlightBrightness = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255 : 0;

        product->setLedBrightness(UrsaMinorThrottleLed::BACKLIGHT, backlightBrightness);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AnnunMode", [this](int annunMode) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/CLRillum");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/SDENG");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/SDENG", [this, product](bool enabled) {
        product->setLedBrightness(UrsaMinorThrottleLed::ENG, enabled || isAnnunTest() ? 1 : 0);
    });
}

TolissUrsaMinorThrottleProfile::~TolissUrsaMinorThrottleProfile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/SDENG");
}

bool TolissUrsaMinorThrottleProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

void TolissUrsaMinorThrottleProfile::update() {
    if (Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/avionics_on")) {
        float gForce = Dataref::getInstance()->get<float>("sim/flightmodel/forces/g_nrml");
        float delta = fabs(gForce - lastGForce);
        lastGForce = gForce;

        bool onGround = Dataref::getInstance()->getCached<bool>("sim/flightmodel/failures/onground_any");
        uint8_t vibration = (uint8_t) std::min(255.0f, delta * (onGround ? product->vibrationMultiplier : product->vibrationMultiplier / 2.0f));
        if (vibration < 6) {
            vibration = 0;
        }

        if (lastVibration != vibration) {
            product->setVibration(vibration);
            lastVibration = vibration;
        }
    } else if (lastVibration > 0) {
        lastVibration = 0;
        product->setVibration(lastVibration);
    }
}

const std::unordered_map<uint16_t, UrsaMinorThrottleButtonDef> &TolissUrsaMinorThrottleProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, UrsaMinorThrottleButtonDef> buttons = {
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
    };

    return buttons;
}

void TolissUrsaMinorThrottleProfile::buttonPressed(const UrsaMinorThrottleButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    datarefManager->executeCommand(button->dataref.c_str(), phase);
}

bool TolissUrsaMinorThrottleProfile::isAnnunTest() {
    return Dataref::getInstance()->get<int>("AirbusFBW/AnnunMode") == 2;
}
