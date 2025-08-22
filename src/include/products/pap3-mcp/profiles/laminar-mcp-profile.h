#ifndef LAMINAR_MCP_PROFILE_H
#define LAMINAR_MCP_PROFILE_H

#include "mcp-aircraft-profile.h"
#include <vector>

class LaminarMCPProfile : public MCPAircraftProfile {
public:
    explicit LaminarMCPProfile(ProductPAP3MCP* p) : MCPAircraftProfile(p) {}
    ~LaminarMCPProfile() override = default;

    const std::vector<std::string>& displayDatarefs() const override;
    const std::vector<MCPButtonDef>& buttonDefs() const override;

    void updateDisplayData(MCPDisplayData& out) override;
    void updateLeds() override;
    void buttonPressed(const MCPButtonDef* button, XPLMCommandPhase phase) override;

    static bool IsEligible(); // heuristique Laminar
private:
    mutable std::vector<std::string> drs_;
    mutable std::vector<MCPButtonDef> btns_;
};

#endif
