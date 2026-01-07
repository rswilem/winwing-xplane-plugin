#ifndef FF777_PDC_PROFILE_H
#define FF777_PDC_PROFILE_H

#include "pdc-aircraft-profile.h"

#include <string>

class FF777PDCProfile : public PDCAircraftProfile {
    public:
        FF777PDCProfile(ProductPDC *product);
        ~FF777PDCProfile();

        static bool IsEligible();
        const std::unordered_map<uint16_t, PDCButtonDef> &buttonDefs() const override;

        void buttonPressed(const PDCButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
