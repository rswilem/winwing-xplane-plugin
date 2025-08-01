#ifndef FF777_PFP_PROFILE_H
#define FF777_PFP_PROFILE_H

#include "pfp-aircraft-profile.h"

class FlightFactor777PfpProfile : public PfpAircraftProfile {
private:
    static std::vector<std::string> datarefsList;
    static std::vector<PFPButtonDef> buttonsList;
    static std::map<char, int> colors;
    
public:
    FlightFactor777PfpProfile(ProductPFP *product);
    virtual ~FlightFactor777PfpProfile();

    static bool IsEligible();
    
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<PFPButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
    void buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
