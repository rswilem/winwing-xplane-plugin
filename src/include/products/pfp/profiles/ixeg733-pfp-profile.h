#ifndef IXEG733_PFP_PROFILE_H
#define IXEG733_PFP_PROFILE_H

#include "pfp-aircraft-profile.h"
#include <regex>

class IXEG733PfpProfile : public PfpAircraftProfile {
private:
    std::regex datarefRegex;
    
public:
    IXEG733PfpProfile(ProductPFP *product);
    virtual ~IXEG733PfpProfile();

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<PFPButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
    void buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
