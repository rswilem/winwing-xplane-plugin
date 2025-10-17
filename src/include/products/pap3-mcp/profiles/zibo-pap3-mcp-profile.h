#ifndef ZIBO_PAP3MCP_PROFILE_H
#define ZIBO_PAP3MCP_PROFILE_H

#include "pap3-mcp-aircraft-profile.h"

#include <map>
#include <string>
#include <vector>

class ZiboPAP3MCPProfile : public PAP3MCPAircraftProfile {
    private:
        int readBankAngleIndex();
        void setBankAngleIndex(int target);
        void maybeToggle(const char *dataref, bool hwState, const char *toggleCmd);

        // Hardware switch states
        bool hwFDCaptOn = false;
        bool hwFDFoOn = false;
        bool hwATOn = false;
        bool hwApDiscEngaged = false;

    public:
        ZiboPAP3MCPProfile(ProductPAP3MCP *product);
        ~ZiboPAP3MCPProfile();

        static bool IsEligible();

        // Override base class methods
        const std::vector<std::string> &displayDatarefs() const override;
        const std::vector<PAP3MCPButtonDef> &buttonDefs() const override;
        const std::vector<PAP3MCPEncoderDef> &encoderDefs() const override;
        void updateDisplayData(PAP3MCPDisplayData &data) override;
        void buttonPressed(const PAP3MCPButtonDef *button, XPLMCommandPhase phase) override;
        void encoderRotated(const PAP3MCPEncoderDef *encoder, int8_t delta) override;

        void handleBankAngleSwitch(uint8_t switchByte) override;
        void handleSwitchChanged(uint8_t byteOffset, uint8_t bitMask, bool state) override;
};

#endif // ZIBO_PAP3MCP_PROFILE_H
