#ifndef MCP_ZIBO_PROFILE_H
#define MCP_ZIBO_PROFILE_H

#include "../mcp-aircraft-profile.h" // <-- IMPORTANT: même base que celle incluse par product-pap3-mcp.h
#include <vector>
#include <string>

/**
 * ZiboMcpProfile
 *
 * Aircraft-specific bindings for the Zibo 737.
 * - Provides LED bindings (dataref -> logical LED + rule)
 * - Provides Display bindings (dataref -> MCP display item)
 */
class ZiboMcpProfile : public McpAircraftProfile
{
public:
    static bool IsEligible();

    // McpAircraftProfile overrides
    const char *name() const override { return "Zibo 737 MCP Profile"; }
    void initialize() override; // resolve XPLM handles for LEDs & displays
    void nudgeBankAngleTo(int targetPos);

    const std::vector<McpLedBinding> &ledBindings() const override { return _ledBindings; }
    const std::vector<std::string> &ledDatarefs() const override { return _ledDatarefs; }
    const std::vector<McpDisplayBinding> &displayBindings() const override { return _displayBindings; }
    const std::vector<McpButtonAction> &buttonActions() const override { return _buttonActions; }
    const std::vector<McpEncoderAction> &encoderActions() const override { return _encoderActions; }
    const std::vector<McpSwitchAction> &switchActions() const override;
    void registerSwitchPositionMonitors(std::function<const uint8_t *()> getHwSnapshot) override;
    void unregisterSwitchPositionMonitors() override;
    void enforceSwitchPositionsFromHardware(const uint8_t *now, const uint8_t *prev) override;
    void startAutopilotPowerMonitor(std::function<void(bool)> onChange) override;
    bool isAutothrottleDisarmedAtStartup() override;

private:
    // --- LED bindings ---
    std::vector<McpLedBinding> _ledBindings; // (dataref, logical, rule, handle)
    std::vector<std::string> _ledDatarefs;   // cached list of LED dataref names

    std::vector<McpButtonAction> _buttonActions;
    std::vector<McpEncoderAction> _encoderActions;
    std::vector<McpSwitchAction> _switchActions;

    std::vector<std::string> _switchPosMonitoredRefs;
    std::function<const uint8_t *()> _getHwSnapshot = nullptr;
    std::function<const uint8_t *()> _getHwState;
    std::vector<std::string> _swPosMonRefs;

    // (Optionnel) petit helper pour BANK angle:
    int desiredBankFromHw(const uint8_t *hw) const;

    // --- Display bindings ---
    std::vector<McpDisplayBinding> _displayBindings; // (dataref, item, type, handle)
};

#endif // MCP_ZIBO_PROFILE_H
