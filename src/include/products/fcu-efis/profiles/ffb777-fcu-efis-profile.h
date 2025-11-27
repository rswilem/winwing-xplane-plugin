#ifndef FF777_FCU_EFIS_PROFILE_H
#define FF777_FCU_EFIS_PROFILE_H

#include "fcu-efis-aircraft-profile.h"

#include <map>
#include <string>
#include <vector>

class FF777FCUEfisProfile : public FCUEfisAircraftProfile {
private:
    bool isTestMode();
    
public:
    FF777FCUEfisProfile(ProductFCUEfis *product);
    ~FF777FCUEfisProfile();
    
    // Méthode permettant de vérifier que le FFB777 est bien chargé
    static bool IsEligible();
    
    // Override base class methods
    const std::vector <std::string> &displayDatarefs() const override;
    const std::unordered_map <uint16_t, FCUEfisButtonDef> &buttonDefs() const override;
    void updateDisplayData(FCUDisplayData &data) override;
    
    bool hasEfisLeft() const override {
        return true;
    }
    
    bool hasEfisRight() const override {
        return true;
    }
    
    void buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) override;
    
    // Méthode créée pour pallier l'absence d'une dataref déterminant si Baro est sur STD
    bool isBaroSTD(const char *side);
    
    // Méthode créée pour pallier l'absence d'une dataref pointant sur la valeur Baro en inHg
    float valBaroInHg(const char *datarefValBaro) ;
    
};

#endif // FF777_FCU_EFIS_PROFILE_H
