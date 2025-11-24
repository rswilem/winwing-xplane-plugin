#ifndef ZIBO_URSA_MINOR_JOYSTICK_PROFILE_H
#define ZIBO_URSA_MINOR_JOYSTICK_PROFILE_H

#include "ursa-minor-joystick-aircraft-profile.h"

#include <string>

class ZiboUrsaMinorJoystickProfile : public UrsaMinorJoystickAircraftProfile {
    private:
        int lastVibration = 0;
        float lastGForce = 0.0f;

    public:
        ZiboUrsaMinorJoystickProfile(ProductUrsaMinorJoystick *product);
        ~ZiboUrsaMinorJoystickProfile();

        static bool IsEligible();

        void update() override;
};

#endif
