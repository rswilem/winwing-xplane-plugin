#ifndef ZIBO_PDC_PROFILE_H
#define ZIBO_PDC_PROFILE_H

#include "pdc-aircraft-profile.h"

#include <string>

class ZiboPDCProfile : public PDCAircraftProfile {
    public:
        ZiboPDCProfile(ProductPDC *product);
        ~ZiboPDCProfile();

        static bool IsEligible();
        const std::unordered_map<PDCButtonIndex3N3M, PDCButtonDef> &buttonDefs() const override;

        void buttonPressed(const PDCButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
