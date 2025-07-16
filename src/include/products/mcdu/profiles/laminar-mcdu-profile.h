#ifndef LAMINAR_MCDU_PROFILE_H
#define LAMINAR_MCDU_PROFILE_H

#include "mcdu-aircraft-profile.h"
#include <regex>

class LaminarMcduProfile : public McduAircraftProfile {
private:
    void writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string& text, char color = 'W', bool fontSmall = false);
    
public:
    LaminarMcduProfile();
    ~LaminarMcduProfile();

    static bool IsEligible();
    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<MCDUButtonDef>& buttonDefs() const override;
    const std::map<char, int>& colorMap() const override;
    void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) override;
};

#endif
