#ifndef ZIBO_URSA_MINOR_JOYSTICK_PROFILE_H
#define ZIBO_URSA_MINOR_JOYSTICK_PROFILE_H

#include "ursa-minor-joystick-aircraft-profile.h"

#include <string>

class ZiboUrsaMinorJoystickProfile : public UrsaMinorJoystickAircraftProfile {
    private:
        bool didInitializeDatarefs = false;
        int lastVibration = 0;
        float lastGForce = 0.0f;

    public:
        ZiboUrsaMinorJoystickProfile(ProductUrsaMinorJoystick *product);
        ~ZiboUrsaMinorJoystickProfile();

        static bool IsEligible();

        // Override base class methods
        void initialize() override;
        void update() override;
        void cleanup() override;
};

#endif // ZIBO_URSA_MINOR_JOYSTICK_PROFILE_H
