#ifndef TOLISS_MCDU_PROFILE_H
#define TOLISS_MCDU_PROFILE_H

#include "mcdu-aircraft-profile.h"
#include <regex>

class TolissMcduProfile : public McduAircraftProfile {
private:
    std::regex datarefRegex;

public:
    TolissMcduProfile(ProductMCDU *product);
    ~TolissMcduProfile();
    
    static bool IsEligible();
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<MCDUButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page) override;
    void buttonPressed(const MCDUButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
