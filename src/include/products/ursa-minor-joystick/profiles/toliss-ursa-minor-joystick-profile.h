#ifndef TOLISS_URSA_MINOR_JOYSTICK_PROFILE_H
#define TOLISS_URSA_MINOR_JOYSTICK_PROFILE_H

#include "ursa-minor-joystick-aircraft-profile.h"

#include <string>

class TolissUrsaMinorJoystickProfile : public UrsaMinorJoystickAircraftProfile {
    private:
        int lastVibration = 0;
        float lastGForce = 0.0f;

    public:
        TolissUrsaMinorJoystickProfile(ProductUrsaMinorJoystick *product);
        ~TolissUrsaMinorJoystickProfile();

        static bool IsEligible();

        void update() override;
};

#endif
