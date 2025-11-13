#ifndef URSA_MINOR_JOYSTICK_AIRCRAFT_PROFILE_H
#define URSA_MINOR_JOYSTICK_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>

class ProductUrsaMinorJoystick;

class UrsaMinorJoystickAircraftProfile {
    protected:
        ProductUrsaMinorJoystick *product;

    public:
        UrsaMinorJoystickAircraftProfile(ProductUrsaMinorJoystick *product) :
            product(product) {};
        virtual ~UrsaMinorJoystickAircraftProfile() = default;

        virtual void initialize() = 0;
        virtual void update() = 0;
        virtual void cleanup() = 0;
};

#endif // URSA_MINOR_JOYSTICK_AIRCRAFT_PROFILE_H
