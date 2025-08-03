#ifndef ZIBO_PFP_PROFILE_H
#define ZIBO_PFP_PROFILE_H

#include "pfp-aircraft-profile.h"
#include <regex>

class ZiboPfpProfile : public PfpAircraftProfile {
private:
    std::regex datarefRegex;
    
public:
    ZiboPfpProfile(ProductPFP *product);
    virtual ~ZiboPfpProfile();

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<PFPButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page) override;
    void buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
