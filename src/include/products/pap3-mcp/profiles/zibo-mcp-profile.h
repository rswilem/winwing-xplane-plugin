#ifndef ZIBO_MCP_PROFILE_H
#define ZIBO_MCP_PROFILE_H

#include "mcp-aircraft-profile.h"
#include <vector>

class ZiboMCPProfile : public MCPAircraftProfile {
public:
    explicit ZiboMCPProfile(ProductPAP3MCP* p) : MCPAircraftProfile(p) {}
    ~ZiboMCPProfile() override = default;

    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<MCPButtonDef>& buttonDefs() const override;

    void updateDisplayData(MCPDisplayData& out) override;
    void updateLeds() override;
    void buttonPressed(const MCPButtonDef* button, XPLMCommandPhase phase) override;

    static bool IsEligible(); // heuristique avion Zibo
private:
    mutable std::vector<std::string> drs_;
    mutable std::vector<MCPButtonDef> btns_;
};

#endif
