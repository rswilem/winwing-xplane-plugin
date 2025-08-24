#ifndef SSG748_FMC_PROFILE_H
#define SSG748_FMC_PROFILE_H

#include "fmc-aircraft-profile.h"
#include <regex>

class SSG748FMCProfile : public FMCAircraftProfile {
private:
    std::regex datarefRegex;
    
public:
    SSG748FMCProfile(ProductFMC *product);
    virtual ~SSG748FMCProfile();

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<FMCButtonDef>& buttonDefs() const override;
    const std::map<char, FMCTextColor>& colorMap() const override;
    void mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) override;
    void updatePage(std::vector<std::vector<char>>& page) override;
    void buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
