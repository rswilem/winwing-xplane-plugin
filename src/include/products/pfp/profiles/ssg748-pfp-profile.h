#ifndef SSG748_PFP_PROFILE_H
#define SSG748_PFP_PROFILE_H

#include "pfp-aircraft-profile.h"
#include <regex>

class SSG748PfpProfile : public PfpAircraftProfile {
private:
    std::regex datarefRegex;
    PFPColor getColorFromCode(char colorCode, bool fontSmall) const;
    
public:
    SSG748PfpProfile(ProductPFP *product);
    virtual ~SSG748PfpProfile();

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<PFPButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
    void buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
