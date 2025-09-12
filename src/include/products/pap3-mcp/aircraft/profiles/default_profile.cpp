#include "default_profile.h"
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>

namespace pap3::aircraft {

DefaultPAP3Profile::DefaultPAP3Profile() {
     // MCP values
    _drSpd     = XPLMFindDataRef("laminar/B738/autopilot/mcp_speed_dial_kts_mach");
    _drHdg     = XPLMFindDataRef("laminar/B738/autopilot/mcp_hdg_dial");
    _drAlt     = XPLMFindDataRef("laminar/B738/autopilot/mcp_alt_dial");
    _drVvi     = XPLMFindDataRef("sim/cockpit2/autopilot/vvi_dial_fpm");
    _drCrsCapt = XPLMFindDataRef("laminar/B738/autopilot/course_pilot");
    _drCrsFo   = XPLMFindDataRef("laminar/B738/autopilot/course_copilot");

    // LEDs (exemples courants, adaptez si besoin)
    _drLedN1     = XPLMFindDataRef("laminar/B738/autopilot/n1_status1");
    _drLedSpd    = XPLMFindDataRef("laminar/B738/autopilot/speed_status1");
    _drLedVnav   = XPLMFindDataRef("laminar/B738/autopilot/vnav_status1");
    _drLedLvlChg = XPLMFindDataRef("laminar/B738/autopilot/lvl_chg_status");
    _drLedHdgSel = XPLMFindDataRef("laminar/B738/autopilot/hdg_sel_status");
    _drLedLnav      = XPLMFindDataRef("laminar/B738/autopilot/lnav_status");
    _drLedVorLoc    = XPLMFindDataRef("laminar/B738/autopilot/vorloc_status");
    _drLedApp       = XPLMFindDataRef("laminar/B738/autopilot/app_status");
    _drLedAltHld    = XPLMFindDataRef("laminar/B738/autopilot/alt_hld_status");
    _drLedVs        = XPLMFindDataRef("laminar/B738/autopilot/vs_status");
    _drLedCmdA      = XPLMFindDataRef("laminar/B738/autopilot/cmd_a_status");
    _drLedCwsA      = XPLMFindDataRef("laminar/B738/autopilot/cws_a_status");
    _drLedCmdB      = XPLMFindDataRef("laminar/B738/autopilot/cmd_b_status");
    _drLedCwsB      = XPLMFindDataRef("laminar/B738/autopilot/cws_b_status");
    _drLedAtArm     = XPLMFindDataRef("laminar/B738/autopilot/autothrottle_status1");
    _drLedMaCapt    = XPLMFindDataRef("laminar/B738/autopilot/master_capt_status");
    _drLedMaFo      = XPLMFindDataRef("laminar/B738/autopilot/master_fo_status");
}

DefaultPAP3Profile::~DefaultPAP3Profile() { stop(); }

bool DefaultPAP3Profile::isEligible() const {
    return true; // Toujours OK si aucun profil spécifique
}

void DefaultPAP3Profile::start(StateCallback onChanged) {
    if (_running) return;
    _cb = std::move(onChanged);
    XPLMRegisterFlightLoopCallback(&DefaultPAP3Profile::FlightLoopThunk, 0.04f, this); // ~25 Hz
    _running = true;
}

void DefaultPAP3Profile::stop() {
    if (!_running) return;
    XPLMUnregisterFlightLoopCallback(&DefaultPAP3Profile::FlightLoopThunk, this);
    _running = false;
}

pap3::aircraft::State DefaultPAP3Profile::current() const { return _state; }

float DefaultPAP3Profile::FlightLoopThunk(float elapsed, float, int, void* refcon) {
    auto* self = static_cast<DefaultPAP3Profile*>(refcon);
    if (self) self->poll();
    return 0.04f; // ~25Hz
}

void DefaultPAP3Profile::poll() {
    if (_drSpd)     _state.spd     = XPLMGetDataf(_drSpd);
    if (_drHdg)     _state.hdg     = XPLMGetDatai(_drHdg);
    if (_drAlt)     _state.alt     = XPLMGetDatai(_drAlt);
    if (_drVvi)     _state.vvi     = XPLMGetDataf(_drVvi);
    if (_drCrsCapt) _state.crsCapt = XPLMGetDatai(_drCrsCapt);
    if (_drCrsFo)   _state.crsFo   = XPLMGetDatai(_drCrsFo);

    // LEDs
    _state.led.N1      = (XPLMGetDataf(_drLedN1)     > 0.5f);
    _state.led.SPEED   = (XPLMGetDataf(_drLedSpd)    > 0.5f);
    _state.led.VNAV    = (XPLMGetDataf(_drLedVnav)   > 0.5f);
    _state.led.LVL_CHG = (XPLMGetDataf(_drLedLvlChg) > 0.5f);
    _state.led.HDG_SEL = (XPLMGetDataf(_drLedHdgSel) > 0.5f);
    _state.led.LNAV    = (XPLMGetDataf(_drLedLnav)   > 0.5f);
    _state.led.VORLOC  = (XPLMGetDataf(_drLedVorLoc) > 0.5f);
    _state.led.APP     = (XPLMGetDataf(_drLedApp)    > 0.5f);
    _state.led.ALT_HLD = (XPLMGetDataf(_drLedAltHld) > 0.5f);
    _state.led.V_S     = (XPLMGetDataf(_drLedVs)      > 0.5f);
    _state.led.CMD_A   = (XPLMGetDataf(_drLedCmdA)    > 0.5f);
    _state.led.CWS_A   = (XPLMGetDataf(_drLedCwsA)    > 0.5f);
    _state.led.CMD_B   = (XPLMGetDataf(_drLedCmdB)    > 0.5f);
    _state.led.CWS_B   = (XPLMGetDataf(_drLedCwsB)    > 0.5f);
    _state.led.AT_ARM  = (XPLMGetDataf(_drLedAtArm)   > 0.5f);
    _state.led.MA_CAPT = (XPLMGetDataf(_drLedMaCapt)  > 0.5f);
    _state.led.MA_FO   = (XPLMGetDataf(_drLedMaFo)    > 0.5f);

    if (_cb) _cb(_state);
}

} // namespace pap3::aircraft