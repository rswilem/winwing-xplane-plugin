#ifndef LAMINAR_MCDU_PROFILE_H
#define LAMINAR_MCDU_PROFILE_H

#include "mcdu-aircraft-profile.h"
#include <regex>

class LaminarMcduProfile : public McduAircraftProfile {
private:
    
public:
    LaminarMcduProfile(ProductMCDU *product);
    ~LaminarMcduProfile();

    static bool IsEligible();
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<MCDUButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page) override;
    void buttonPressed(const MCDUButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
