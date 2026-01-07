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

class PDCAircraftProfile {
    protected:
        ProductPDC *product;

    public:
        PDCAircraftProfile(ProductPDC *product) : product(product) {};
        virtual ~PDCAircraftProfile() = default;

        virtual const std::unordered_map<uint16_t, PDCButtonDef> &buttonDefs() const = 0;
        virtual void buttonPressed(const PDCButtonDef *button, XPLMCommandPhase phase) = 0;
};

#endif
