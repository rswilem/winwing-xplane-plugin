#ifndef ECAM32_AIRCRAFT_PROFILE_H
#define ECAM32_AIRCRAFT_PROFILE_H

#include <string>
#include <unordered_map>
#include <XPLMUtilities.h>

class ProductECAM32;

struct ECAM32ButtonDef {
        std::string name;
        std::string dataref;
        double value = 0.0;
};

class ECAM32AircraftProfile {
    protected:
        ProductECAM32 *product;

    public:
        ECAM32AircraftProfile(ProductECAM32 *product) : product(product) {};
        virtual ~ECAM32AircraftProfile() = default;

        virtual const std::unordered_map<uint16_t, ECAM32ButtonDef> &buttonDefs() const = 0;
        virtual void buttonPressed(const ECAM32ButtonDef *button, XPLMCommandPhase phase) = 0;
};

#endif
