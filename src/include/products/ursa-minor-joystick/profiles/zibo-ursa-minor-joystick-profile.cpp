#include "zibo-ursa-minor-joystick-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-ursa-minor-joystick.h"

#include <algorithm>
#include <cmath>

ZiboUrsaMinorJoystickProfile::ZiboUrsaMinorJoystickProfile(ProductUrsaMinorJoystick *product) : UrsaMinorJoystickAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/panel_brightness", [product](std::vector<float> panelBrightness) {
        if (panelBrightness.size() < 4) {
            return;
        }

        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
        uint8_t target = hasPower ? panelBrightness[3] * 255.0f : 0;
        product->setLedBrightness(target);

        if (!hasPower) {
            product->setVibration(0);
        }
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/panel_brightness");
    });
}

ZiboUrsaMinorJoystickProfile::~ZiboUrsaMinorJoystickProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("laminar/B738/electric/panel_brightness");
    Dataref::getInstance()->unbind("sim/flightmodel/failures/onground_any");
}

bool ZiboUrsaMinorJoystickProfile::IsEligible() {
    return Dataref::getInstance()->exists("laminar/B738/electric/panel_brightness");
}

void ZiboUrsaMinorJoystickProfile::update() {
    if (Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/avionics_on")) {
        float gForce = Dataref::getInstance()->get<float>("sim/flightmodel/forces/g_nrml");
        float delta = fabs(gForce - lastGForce);
        lastGForce = gForce;

        bool onGround = Dataref::getInstance()->getCached<bool>("sim/flightmodel/failures/onground_any");
        uint8_t vibration = (uint8_t) std::min(255.0f, delta * (onGround ? product->vibrationFactor : product->vibrationFactor / 2.0f));
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

