#ifndef ZIBO_PFP_PROFILE_H
#define ZIBO_PFP_PROFILE_H

#include "pfp-aircraft-profile.h"

class ZiboPfpProfile : public PfpAircraftProfile {
private:
    static std::vector<std::string> datarefsList;
    static std::vector<PFPButtonDef> buttonsList;
    static std::map<char, int> colors;
    
public:
    ZiboPfpProfile();
    virtual ~ZiboPfpProfile() = default;

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<PFPButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
    
private:
    void writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color = 'W', bool fontSmall = false);
};

#endif
