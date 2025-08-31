// zibo_profile.h
#pragma once
#include "../pap3_aircraft.h"
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <vector>
#include <string>
#include <cstdint>

namespace pap3::aircraft {

class ZiboPAP3Profile final : public PAP3AircraftProfile {
public:
    using State = pap3::aircraft::State;

    ZiboPAP3Profile();
    ~ZiboPAP3Profile() override;

    bool  isEligible() const override;
    void  start(StateCallback onChanged) override;
    void  stop() override;
    State current() const override;

    void syncSimToHardware() override;


    // Input hooks (must match PAP3AircraftProfile)
    void onButton(uint8_t off, uint8_t mask, bool pressed) override;
    void onEncoderDelta(uint8_t posOff, int8_t delta) override;

    void syncSimToHardwareFromRaw(const std::uint8_t* report, int len) override;

    uint8_t mcpPowerMask() const override;


    // Device power for illumination gating
    bool mcpHasPower() const override;

    // --- Bank angle (Zibo) ---
    int  readBankIndex() const;        // 0..4
    void nudgeBankAngleTo(int target); // 0..4

private:
    bool _haveHwSnapshot{false};
    static float FlightLoopThunk(float elapsed, float, int, void* refcon);
    void poll();

    // Helpers
    void execOnce(XPLMCommandRef cmd);
    void repeatCmd(XPLMCommandRef inc, XPLMCommandRef dec, int8_t delta, int stepPerTick = 1);
    void maybeToggle(XPLMDataRef posRef, bool desiredOn, XPLMCommandRef toggleCmd);
    bool debounce(float& lastTs, float minDeltaSec = 0.05f) const;

    State         _state{};
    StateCallback _cb;
    bool          _running{false};

    // Datarefs

    // LCDs
    XPLMDataRef _drSpd{nullptr};
    XPLMDataRef _drHdg{nullptr};
    XPLMDataRef _drAlt{nullptr};
    XPLMDataRef _drVvi{nullptr};
    XPLMDataRef _drVviShow{nullptr};
    XPLMDataRef _drCrsCapt{nullptr};
    XPLMDataRef _drCrsFo{nullptr};
    
    // MCP LCD "special digits"
    XPLMDataRef _drDigitA{nullptr};
    XPLMDataRef _drDigitB{nullptr};
    
    // LEDs
    XPLMDataRef _drLedN1{nullptr};
    XPLMDataRef _drLedSpd{nullptr};
    XPLMDataRef _drLedVnav{nullptr};
    XPLMDataRef _drLedLvlChg{nullptr};
    XPLMDataRef _drLedHdgSel{nullptr};
    XPLMDataRef _drLedLnav{nullptr};
    XPLMDataRef _drLedVorLoc{nullptr};
    XPLMDataRef _drLedApp{nullptr};
    XPLMDataRef _drLedAltHld{nullptr};
    XPLMDataRef _drLedVs{nullptr};
    XPLMDataRef _drLedCmdA{nullptr};
    XPLMDataRef _drLedCwsA{nullptr};
    XPLMDataRef _drLedCmdB{nullptr};
    XPLMDataRef _drLedCwsB{nullptr};
    XPLMDataRef _drLedAtArm{nullptr};
    XPLMDataRef _drLedMaCapt{nullptr};
    XPLMDataRef _drLedMaFo{nullptr};

    // --- Switch position datarefs (Zibo) ---
    XPLMDataRef _drFDCaptPos{nullptr};   // laminar/B738/autopilot/flight_director_pos        (0/1)
    XPLMDataRef _drFDFoPos{nullptr};     // laminar/B738/autopilot/flight_director_fo_pos     (0/1)
    XPLMDataRef _drATArmPos{nullptr};    // laminar/B738/autopilot/autothrottle_arm_pos       (0/1)
    XPLMDataRef _drApDiscPos{nullptr};   // laminar/B738/autopilot/disconnect_pos             (0/1)

    // --- Toggle commands (Zibo) ---
    XPLMCommandRef _cmdFDCaptToggle{nullptr}; // laminar/B738/autopilot/flight_director_toggle
    XPLMCommandRef _cmdFDFoToggle{nullptr};   // laminar/B738/autopilot/flight_director_fo_toggle
    XPLMCommandRef _cmdATArmToggle{nullptr};  // laminar/B738/autopilot/autothrottle_arm_toggle
    XPLMCommandRef _cmdApDiscToggle{nullptr}; // laminar/B738/autopilot/disconnect_toggle

    // Power-related datarefs 
    XPLMDataRef _drHasApPower{nullptr}; // sim/cockpit2/autopilot/autopilot_has_power (int)
    XPLMDataRef _drDcBus1{nullptr};     // laminar/B738/electric/dc_bus1_status     (int/float)
    XPLMDataRef _drDcBus2{nullptr};     // laminar/B738/electric/dc_bus2_status     (int/float}

     // --- Bank angle (Zibo) ---
    XPLMDataRef    _drBankIdx{nullptr};          // laminar/B738/autopilot/bank_angle_pos (0..4) ou _sel suivant version
    XPLMCommandRef _cmdBankUp{nullptr};          // laminar/B738/autopilot/bank_angle_up
    XPLMCommandRef _cmdBankDn{nullptr};          // laminar/B738/autopilot/bank_angle_dn

    // --- Last known hardware state inferred from PAP3 inputs ---
    bool _hwFDCaptOn{false};
    bool _hwFDFoOn{false};
    bool _hwATOn{false};
    bool _hwApDiscEngaged{false}; // true = UP/ENGAGED, false = DOWN/DISENGAGED

    // --- Simple debounce for double-line switches (AT/AP DISC) ---
    float _lastAtToggleTime{0.0f};
    float _lastApDiscToggleTime{0.0f};

    // Button bindings (offset/mask -> press/release commands)
    struct BtnBinding {
        uint8_t off;
        uint8_t mask;
        XPLMCommandRef press{nullptr};
        XPLMCommandRef release{nullptr}; // optional
    };
    std::vector<BtnBinding> _btns;

    // Encoder bindings (posOff -> inc/dec commands)
    struct EncBinding {
        uint8_t posOff;
        XPLMCommandRef inc{nullptr};
        XPLMCommandRef dec{nullptr};
        int stepPerTick{1};
    };
    std::vector<EncBinding> _encs;
};

} // namespace pap3::aircraft