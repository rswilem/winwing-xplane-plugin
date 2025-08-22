#include "zibo-mcp-profile.h"
#include "product-pap3-mcp.h"
#include "dataref.h"
#include <cmath>
#include <cstdio>

// Heuristique simple : si l’avion contient "B738X" ou "Zibo" dans le path/author.
// Tu peux raffiner selon ton infra (AppState, etc.)
bool ZiboMCPProfile::IsEligible() {
    // TODO: brancher sur ta logique AppState si dispo. Par défaut on renvoie true pour tester.
    return true;
}

const std::vector<std::string>& ZiboMCPProfile::displayDatarefs() const {
    if (drs_.empty()) {
        // Valeurs "stock" X-Plane (compat Laminar 737) — modifie ici pour Zibo si tu as les DR exactes
        drs_ = {
            "sim/cockpit2/autopilot/airspeed_dial_kts",
            "sim/cockpit2/autopilot/heading_dial_deg_mag_pilot",
            "sim/cockpit2/autopilot/altitude_dial_ft",
            "sim/cockpit2/autopilot/vvi_dial_fpm",
            "sim/cockpit2/radios/actuators/nav1_obs_deg_mag_pilot",
            "sim/cockpit2/radios/actuators/nav2_obs_deg_mag_copilot",
            // LEDs (exemples stocks : A/T, APP, LNAV/LOC...) — à adapter Zibo si tu as mieux :
            "sim/cockpit2/autopilot/autothrottle_enabled",
            "sim/cockpit2/autopilot/approach_status",
            "sim/cockpit2/autopilot/nav_status",
            "sim/cockpit2/autopilot/heading_mode" // 0/1
        };
    }
    return drs_;
}

const std::vector<MCPButtonDef>& ZiboMCPProfile::buttonDefs() const {
    if (btns_.empty()) {
        // TODO: remplir quand on aura cartographié l’input report du PAP3
        // Exemple:
        // btns_.push_back({ /*id*/ 0, "HDG SEL", "sim/autopilot/heading_select", MCPDatarefType::EXECUTE_CMD_ONCE, 0.0 });
    }
    return btns_;
}

static std::string fmt_ndigits(int v, int width) {
    char buf[16]; std::snprintf(buf, sizeof(buf), "%0*d", width, std::abs(v));
    return std::string(buf);
}
static std::string fmt_heading_deg(double deg) {
    int h = ((int)std::llround(deg)) % 360; if (h < 0) h += 360;
    char buf[8]; std::snprintf(buf, sizeof(buf), "%03d", h);
    return std::string(buf);
}
static std::string fmt_altitude5(int ft) {
    // 5 digits, ex:  35000 => "35000",  9000 => "09000"
    char buf[16]; std::snprintf(buf, sizeof(buf), "%05d", std::max(0, ft));
    return std::string(buf);
}

void ZiboMCPProfile::updateDisplayData(MCPDisplayData& out) {
    auto dr = Dataref::getInstance();

    // ==== valeurs "stock" X-Plane (OK aussi pour Laminar) ====
    double ias = dr->getFloat("sim/cockpit2/autopilot/airspeed_dial_kts");
    double hdg = dr->getFloat("sim/cockpit2/autopilot/heading_dial_deg_mag_pilot");
    double alt = dr->getFloat("sim/cockpit2/autopilot/altitude_dial_ft");
    double vsi = dr->getFloat("sim/cockpit2/autopilot/vvi_dial_fpm");
    double crs1 = dr->getFloat("sim/cockpit2/radios/actuators/nav1_obs_deg_mag_pilot");
    double crs2 = dr->getFloat("sim/cockpit2/radios/actuators/nav2_obs_deg_mag_copilot");

    // TODO Zibo : si tu as DR spécifiques (ex : laminar/B738/autopilot/...), remplace ici.

    // ---- Formatages 7-seg ----
    // Speed: 4 chars -> " 250", "0280", etc. On force 4 digits si >999
    int spd = (int)std::round(std::max(0.0, ias));
    if      (spd <= 999) out.speed4 = (spd < 100) ? (" " + fmt_ndigits(spd,3)) : fmt_ndigits(spd,4);
    else                  out.speed4 = fmt_ndigits(spd % 10000, 4);

    out.heading3    = fmt_heading_deg(hdg);
    out.altitude5   = fmt_altitude5((int)std::round(std::max(0.0, alt)));

    int v = (int)std::round(std::fabs(vsi));
    if (v > 9999) v = 9999;
    out.vs4        = fmt_ndigits(v,4);
    out.vsNegative = (vsi < -50); // deadband ±50fpm
    out.vsPositive = (vsi >  50);

    auto fmt_crs3 = [](double deg){
        int d = ((int)std::llround(deg)) % 360; if (d < 0) d += 360;
        char buf[8]; std::snprintf(buf, sizeof(buf), "%03d", d);
        return std::string(buf);
    };
    out.captCourse3 = fmt_crs3(crs1);
    out.foCourse3   = fmt_crs3(crs2);
}

void ZiboMCPProfile::updateLeds() {
    auto dr = Dataref::getInstance();
    auto* p = product;

    // Exemples *stock* qui allument quelques LEDs — à remplacer si tu as les DR Zibo précises
    bool at = dr->getInt("sim/cockpit2/autopilot/autothrottle_enabled") != 0;
    int app = dr->getInt("sim/cockpit2/autopilot/approach_status"); // 0 off, 1 armed, 2 captured
    int nav = dr->getInt("sim/cockpit2/autopilot/nav_status");       // 0 none, 1 armed, 2 captured/track
    int hdg_mode = dr->getInt("sim/cockpit2/autopilot/heading_mode");// 0/1

    p->setLed(ProductPAP3MCP::Led::AT_ARM, at);
    p->setLed(ProductPAP3MCP::Led::APP,    app >= 1);
    // NAV/LNAV/VOR LOC (approx stock) :
    p->setLed(ProductPAP3MCP::Led::LNAV,   nav >= 1);
    p->setLed(ProductPAP3MCP::Led::VOR_LOC,nav == 2);
    // HDG SEL :
    p->setLed(ProductPAP3MCP::Led::HDG_SEL, hdg_mode != 0);

    // TODO: VNAV, FL CHG, ALT HLD, VS, CMD A/B, etc. -> mettre tes datarefs Zibo.
}

void ZiboMCPProfile::buttonPressed(const MCPButtonDef* button, XPLMCommandPhase phase) {
    if (!button) return;
    // TODO: appeler XPLMCommandRef pour EXECUTE_CMD_ONCE / begin/continue/end
    (void)phase; (void)button;
}
