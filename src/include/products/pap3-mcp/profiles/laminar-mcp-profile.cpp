#include "laminar-mcp-profile.h"
#include "product-pap3-mcp.h"
#include "dataref.h"
#include <cmath>
#include <cstdio>

bool LaminarMCPProfile::IsEligible() {
    // Heuristique simple: si Zibo non détecté -> fallback Laminar
    return true;
}

const std::vector<std::string>& LaminarMCPProfile::displayDatarefs() const {
    if (drs_.empty()) {
        drs_ = {
            "sim/cockpit2/autopilot/airspeed_dial_kts",
            "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot",
            "sim/cockpit2/autopilot/altitude_dial_ft",
            "sim/cockpit2/autopilot/vvi_dial_fpm",
            "sim/cockpit2/radios/actuators/nav1_obs_deg_mag_pilot",
            "sim/cockpit2/radios/actuators/nav2_obs_deg_mag_copilot",
            "sim/cockpit2/autopilot/autothrottle_enabled",
            "sim/cockpit2/autopilot/approach_status",
            "sim/cockpit2/autopilot/nav_status",
            "sim/cockpit2/autopilot/heading_mode"
        };
    }
    return drs_;
}

const std::vector<MCPButtonDef>& LaminarMCPProfile::buttonDefs() const {
    if (btns_.empty()) {
        // TODO: à remplir quand on cartographiera l’input report PAP3
    }
    return btns_;
}

static std::string fmt3(int v) { char b[8]; std::snprintf(b,sizeof b,"%03d",v); return b; }
static std::string fmt4(int v) { char b[8]; std::snprintf(b,sizeof b,"%04d",v); return b; }
static std::string fmt5(int v) { char b[8]; std::snprintf(b,sizeof b,"%05d",v); return b; }

void LaminarMCPProfile::updateDisplayData(MCPDisplayData& out) {
    auto dr = Dataref::getInstance();
    int spd = (int)std::round(std::max(0.0, dr->getFloat("sim/cockpit2/autopilot/airspeed_dial_kts")));
    int hdg = ((int)std::llround(dr->getFloat("sim/cockpit2/autopilot/heading_dial_deg_mag_pilot"))) % 360; if (hdg<0) hdg+=360;
    int alt = (int)std::round(std::max(0.0, dr->getFloat("sim/cockpit2/autopilot/altitude_dial_ft")));
    int vsi = (int)std::round(dr->getFloat("sim/cockpit2/autopilot/vvi_dial_fpm"));
    int crs1= ((int)std::llround(dr->getFloat("sim/cockpit2/radios/actuators/nav1_obs_deg_mag_pilot"))) % 360; if (crs1<0) crs1+=360;
    int crs2= ((int)std::llround(dr->getFloat("sim/cockpit2/radios/actuators/nav2_obs_deg_mag_copilot"))) % 360; if (crs2<0) crs2+=360;

    out.speed4      = (spd < 100) ? (" " + fmt4(spd).substr(1)) : (spd<=999? (" " + fmt3(spd)) : fmt4(spd%10000));
    out.heading3    = fmt3(hdg);
    out.altitude5   = fmt5(alt);
    int av = std::min(std::abs(vsi), 9999);
    out.vs4         = fmt4(av);
    out.vsNegative  = (vsi < -50);
    out.vsPositive  = (vsi >  50);
    out.captCourse3 = fmt3(crs1);
    out.foCourse3   = fmt3(crs2);
}

void LaminarMCPProfile::updateLeds() {
    auto dr = Dataref::getInstance();
    auto* p = product;

    bool at = dr->getInt("sim/cockpit2/autopilot/autothrottle_enabled") != 0;
    int app  = dr->getInt("sim/cockpit2/autopilot/approach_status");
    int nav  = dr->getInt("sim/cockpit2/autopilot/nav_status");
    int hdg  = dr->getInt("sim/cockpit2/autopilot/heading_mode");

    p->setLed(ProductPAP3MCP::Led::AT_ARM, at);
    p->setLed(ProductPAP3MCP::Led::APP,    app >= 1);
    p->setLed(ProductPAP3MCP::Led::LNAV,   nav >= 1);
    p->setLed(ProductPAP3MCP::Led::VOR_LOC,nav == 2);
    p->setLed(ProductPAP3MCP::Led::HDG_SEL,hdg != 0);
}

void LaminarMCPProfile::buttonPressed(const MCPButtonDef* button, XPLMCommandPhase phase) {
    (void)button; (void)phase;
    // TODO
}
