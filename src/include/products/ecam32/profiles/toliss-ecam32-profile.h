#ifndef TOLISS_ECAM32_PROFILE_H
#define TOLISS_ECAM32_PROFILE_H

#include "ecam32-aircraft-profile.h"

#include <string>

class TolissECAM32Profile : public ECAM32AircraftProfile {
    private:

    public:
        TolissECAM32Profile(ProductECAM32 *product);
        ~TolissECAM32Profile();

        static bool IsEligible();
        const std::unordered_map<uint16_t, ECAM32ButtonDef> &buttonDefs() const override;
    
        void buttonPressed(const ECAM32ButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
