#ifndef TOLISS_MCDU_PROFILE_H
#define TOLISS_MCDU_PROFILE_H

#include "mcdu-aircraft-profile.h"

class TolissMcduProfile : public McduAircraftProfile {
private:
    std::regex datarefRegex;
    void writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string& text, char color = 'W', bool fontSmall = false);
public:
    TolissMcduProfile();
    ~TolissMcduProfile();
    
    static bool IsEligible();
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<MCDUButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
};

#endif
