#ifndef URSA_MINOR_THROTTLE_AIRCRAFT_PROFILE_H
#define URSA_MINOR_THROTTLE_AIRCRAFT_PROFILE_H

#include <string>
#include <unordered_map>
#include <XPLMUtilities.h>

class ProductUrsaMinorThrottle;

enum class UrsaMinorThrottleDatarefType : unsigned char {
    EXECUTE_CMD = 1,
    SET_VALUE,
    SPEEDBRAKE
};

struct UrsaMinorThrottleButtonDef {
        std::string name;
        std::string dataref;
        UrsaMinorThrottleDatarefType datarefType = UrsaMinorThrottleDatarefType::EXECUTE_CMD;
        double value = 0.0;
};

class UrsaMinorThrottleAircraftProfile {
    protected:
        ProductUrsaMinorThrottle *product;

    public:
        UrsaMinorThrottleAircraftProfile(ProductUrsaMinorThrottle *product) : product(product) {};
        virtual ~UrsaMinorThrottleAircraftProfile() = default;

        virtual void update() = 0;

        virtual const std::unordered_map<uint16_t, UrsaMinorThrottleButtonDef> &buttonDefs() const = 0;
        virtual void buttonPressed(const UrsaMinorThrottleButtonDef *button, XPLMCommandPhase phase) = 0;

        virtual void updateDisplays() = 0;
};

#endif
