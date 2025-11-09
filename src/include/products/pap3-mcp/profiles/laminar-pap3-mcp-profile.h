#ifndef LAMINAR_PAP3MCP_PROFILE_H
#define LAMINAR_PAP3MCP_PROFILE_H

#include "pap3-mcp-aircraft-profile.h"

#include <map>
#include <string>
#include <vector>

class LaminarPAP3MCPProfile : public PAP3MCPAircraftProfile {
    public:
        LaminarPAP3MCPProfile(ProductPAP3MCP *product);
        ~LaminarPAP3MCPProfile();

        static bool IsEligible();

        // Override base class methods
        const std::vector<std::string> &displayDatarefs() const override;
        const std::unordered_map<uint16_t, PAP3MCPButtonDef> &buttonDefs() const override;
        const std::vector<PAP3MCPEncoderDef> &encoderDefs() const override;
        void updateDisplayData(PAP3MCPDisplayData &data) override;
        void buttonPressed(const PAP3MCPButtonDef *button, XPLMCommandPhase phase) override;
        void encoderRotated(const PAP3MCPEncoderDef *encoder, int8_t delta) override;
};

#endif // LAMINAR_PAP3MCP_PROFILE_H
