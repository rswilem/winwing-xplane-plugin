#ifndef PDC_AIRCRAFT_PROFILE_H
#define PDC_AIRCRAFT_PROFILE_H

#include <string>
#include <unordered_map>
#include <XPLMUtilities.h>

class ProductPDC;

enum class PDCDatarefType : unsigned char {
    EXECUTE_CMD_ONCE = 1,
    EXECUTE_CMD_PHASED,
    SET_VALUE,
    SET_VALUE_USING_COMMANDS
};

struct PDCButtonDef {
        std::string name;
        std::string dataref;
        PDCDatarefType datarefType = PDCDatarefType::EXECUTE_CMD_ONCE;
        double value = 0.0;
};

enum class PDCDeviceVariant : unsigned char {
    VARIANT_3N_CAPTAIN = 0x00,
    VARIANT_3N_FIRSTOFFICER = 0x01,

    VARIANT_3M_CAPTAIN = 0x10,
    VARIANT_3M_FIRSTOFFICER = 0x11
};

using PDCButtonIndex3N3M = std::pair<signed char, signed char>;

namespace std {
    template<>
    struct hash<PDCButtonIndex3N3M> {
            size_t operator()(const PDCButtonIndex3N3M &p) const noexcept {
                return (static_cast<size_t>(p.first) << 16) | p.second;
            }
    };
}

class PDCAircraftProfile {
    protected:
        ProductPDC *product;

    public:
        PDCAircraftProfile(ProductPDC *product) : product(product) {};
        virtual ~PDCAircraftProfile() = default;

        virtual const std::unordered_map<PDCButtonIndex3N3M, PDCButtonDef> &buttonDefs() const = 0;
        virtual void buttonPressed(const PDCButtonDef *button, XPLMCommandPhase phase) = 0;
};

#endif
