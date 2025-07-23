#ifndef FF777_PFP_PROFILE_H
#define FF777_PFP_PROFILE_H

#include "pfp-aircraft-profile.h"

class FlightFactor777PfpProfile : public PfpAircraftProfile {
private:
    static std::vector<std::string> datarefsList;
    static std::vector<PFPButtonDef> buttonsList;
    static std::map<char, int> colors;
    
public:
    FlightFactor777PfpProfile();
    virtual ~FlightFactor777PfpProfile();

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<PFPButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
    
private:
    void writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color = 'W', bool fontSmall = false);
};

#endif
