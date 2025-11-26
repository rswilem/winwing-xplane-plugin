#ifndef AGP_AIRCRAFT_PROFILE_H
#define AGP_AIRCRAFT_PROFILE_H

#include <string>
#include <unordered_map>
#include <XPLMUtilities.h>

class ProductAGP;

struct AGPButtonDef {
        std::string name;
        std::string dataref;
        double value = 0.0;
};

class AGPAircraftProfile {
    protected:
        ProductAGP *product;

    public:
        AGPAircraftProfile(ProductAGP *product) : product(product) {};
        virtual ~AGPAircraftProfile() = default;

        virtual const std::unordered_map<uint16_t, AGPButtonDef> &buttonDefs() const = 0;
        virtual void buttonPressed(const AGPButtonDef *button, XPLMCommandPhase phase) = 0;
};

#endif
