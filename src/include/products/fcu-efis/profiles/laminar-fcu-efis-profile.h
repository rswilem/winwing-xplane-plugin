#ifndef LAMINAR_FCU_EFIS_PROFILE_H
#define LAMINAR_FCU_EFIS_PROFILE_H

#include "fcu-efis-aircraft-profile.h"

#include <map>
#include <string>
#include <vector>

class LaminarFCUEfisProfile : public FCUEfisAircraftProfile {
    public:
        LaminarFCUEfisProfile(ProductFCUEfis *product);
        ~LaminarFCUEfisProfile();

        static bool IsEligible();

        // Override base class methods
        const std::vector<std::string> &displayDatarefs() const override;
        const std::vector<FCUEfisButtonDef> &buttonDefs() const override;
        void updateDisplayData(FCUDisplayData &data) override;

        bool hasEfisLeft() const override {
            return true;
        }

        bool hasEfisRight() const override {
            return true;
        }

        void buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) override;
};

#endif // LAMINAR_FCU_EFIS_PROFILE_H
