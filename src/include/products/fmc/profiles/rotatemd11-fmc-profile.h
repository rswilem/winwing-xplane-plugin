#ifndef ROTATEMD11_FMC_PROFILE_H
#define ROTATEMD11_FMC_PROFILE_H

#include "fmc-aircraft-profile.h"

class RotateMD11FMCProfile : public FMCAircraftProfile {
    private:
        std::string processUTF8Arrows(const std::string &input);
        std::vector<int> buildStylePositionMap(const std::string &content, const std::string &style);

    public:
        RotateMD11FMCProfile(ProductFMC *product);
        virtual ~RotateMD11FMCProfile();

        static bool IsEligible();

        const std::vector<std::string> &displayDatarefs() const override;
        const std::vector<FMCButtonDef> &buttonDefs() const override;
        const std::map<char, FMCTextColor> &colorMap() const override;
        void mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) override;
        void updatePage(std::vector<std::vector<char>> &page) override;
        void buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
