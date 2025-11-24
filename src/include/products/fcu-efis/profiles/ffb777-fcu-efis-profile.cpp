#include "ffb777-fcu-efis-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-fcu-efis.h"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <XPLMUtilities.h>


// ===============================================================================================================
// Constructeur de le classe
FF777FCUEfisProfile::FF777FCUEfisProfile(ProductFCUEfis *product) :
    FCUEfisAircraftProfile(product) {
    
    // ====================================================
    // SECTION 1: MONITORING DE L'ÉCLAIRAGE ET ALIMENTATION
    // ====================================================
    
    // TODO: Identifier la dataref d'alimentation du MCP pour le B777
    // Exemple hypothétique: "T7Avionics/mcp_power"
    Dataref::getInstance()->monitorExistingDataref<bool>("T7Avionics/mcp_power", [product](bool hasPower) {
        // Luminosité des backlights
        uint8_t brightness = hasPower ? 255 : 0;
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, brightness);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, brightness);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, brightness);
        
        // Luminosité des écrans
        uint8_t screenBrightness = hasPower ? 200 : 0;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);
        
        product->forceStateSync();
    });

    // ==========================================================
    // SECTION 2: MONITORING DES LEDs DU MCP (MODE CONTROL PANEL)
    // ==========================================================
    
    // Autopilot A (CMD L sur B777)
    Dataref::getInstance()->monitorExistingDataref<int>("777/autopilot/ap_left_engaged", [this, product](int engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    // Autopilot B (CMD R sur B777)
    Dataref::getInstance()->monitorExistingDataref<int>("777/autopilot/ap_right_engaged", [this, product](int engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    // Autothrottle
    Dataref::getInstance()->monitorExistingDataref<int>("777/autopilot/autothrottle_armed", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    // LOC mode
    Dataref::getInstance()->monitorExistingDataref<int>("777/autopilot/loc_armed", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, armed || isTestMode() ? 1 : 0);
    });

    // APPROACH mode
    Dataref::getInstance()->monitorExistingDataref<int>("777/autopilot/app_armed", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    // ====================================================
    // SECTION 3: MONITORING DES LEDs EFIS CAPTAIN (Gauche)
    // ====================================================
    
    // Flight Director Captain
    Dataref::getInstance()->monitorExistingDataref<int>("777/displays/fd_left_on", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    // TODO: Mapping des autres boutons EFIS Captain selon datarefs B777
    // Exemples de datarefs possibles (à vérifier):
    // - "777/displays/captain/show_wpt"
    // - "777/displays/captain/show_vor"
    // - "777/displays/captain/show_ndb"
    // - "777/displays/captain/show_arpt"

    // ==========================================================
    // SECTION 4: MONITORING DES LEDs EFIS FIRST OFFICER (Droite)
    // ==========================================================
    
    // Flight Director First Officer
    Dataref::getInstance()->monitorExistingDataref<int>("777/displays/fd_right_on", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    // TODO: Mapping des autres boutons EFIS FO selon datarefs B777
}


// ===============================================================================================================
// Destructeur de la classe
FF777FCUEfisProfile::~FF777FCUEfisProfile() {
    // Unbind de toutes les datarefs monitorées
    Dataref::getInstance()->unbind("T7Avionics/mcp_power");
    
    // MCP datarefs
    Dataref::getInstance()->unbind("777/autopilot/ap_left_engaged");
    Dataref::getInstance()->unbind("777/autopilot/ap_right_engaged");
    Dataref::getInstance()->unbind("777/autopilot/autothrottle_armed");
    Dataref::getInstance()->unbind("777/autopilot/loc_armed");
    Dataref::getInstance()->unbind("777/autopilot/app_armed");
    
    // EFIS datarefs
    Dataref::getInstance()->unbind("777/displays/fd_left_on");
    Dataref::getInstance()->unbind("777/displays/fd_right_on");
    
    // TODO: Unbind des autres datarefs ajoutées
}

// OK ============================================================================================================
// Méthode permettant de vérifier que c'est bien le FFB777 qui est chargé
bool FF777FCUEfisProfile::IsEligible() {
    /* On vérifie la présence d'au moins une dataref spécifique du FF B777, qui attestera que cet avion est bien chargé.
       Sachant que les datarefs testées ici n'ont pas vocation à provoquer des actions ou à être modifiées, on peut
       utiliser des datarefs '/anim' idéales pour des besoins de simple monitoring   */
    bool eligible = Dataref::getInstance()->exists("1-sim/ckpt/mcpApLButton/anim") ||
                    Dataref::getInstance()->exists("1-sim/ckpt/cptHsiRangeSwitch/anim");
    return eligible;
}


// ===============================================================================================================
const std::vector <std::string> &FF777FCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // Alimentation et état
        "T7Avionics/mcp_power",
        
        // MCP - Speed
        "sim/cockpit2/autopilot/airspeed_dial_kts_mach",
        "sim/cockpit/autopilot/airspeed_is_mach",
        "777/autopilot/speed_mode", // SPD, FLCH, etc.
        
        // MCP - Heading
        "sim/cockpit/autopilot/heading_mag",
        "777/autopilot/heading_hold_active",
        
        // MCP - Altitude
        "sim/cockpit/autopilot/altitude",
        "777/autopilot/altitude_hold_active",
        
        // MCP - Vertical Speed
        "sim/cockpit/autopilot/vertical_velocity",
        "777/autopilot/vnav_active",
        
        // EFIS - Barometric settings
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot",
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot",
        "777/displays/captain_baro_std",
        "777/displays/fo_baro_std",
        "777/displays/captain_baro_unit", // 0=inHg, 1=hPa
        "777/displays/fo_baro_unit",
        
        // ND Mode et Range
        "777/displays/captain_nd_mode", // 0=APP, 1=VOR, 2=MAP, 3=PLAN
        "777/displays/fo_nd_mode",
        "777/displays/captain_nd_range",
        "777/displays/fo_nd_range",
    };

    return datarefs;
}


// ===============================================================================================================
// Méthode renvoyant le mappage des boutons FCU+EFIS avec leur Dataref associée
const std::unordered_map <uint16_t, FCUEfisButtonDef> &FF777FCUEfisProfile::buttonDefs() const {
    /* En l'état ce mapping est hypothétique et doit être adapté selon:
        1. La disposition physique du matériel
        2. Les datarefs réelles du FF B777v2
    
     Les FCU EFIS Winwing étant conçus pour la gamme Airbus, il est nécessaire de garder en tête
     à quel n° de bouton est associé l'action attendue sur Airbus (identifiable par le nom du bouton)
     et de lui affecter ce qui s'y rapporte le plus sur le B777
     Les associations Airbus sont rappelées en commentaire, face à chaque bouton.
     Dans la majorité des cas la tranposition est naturelle, mais il y a quelques exceptions :
        - AP1 Airbus sera utilisé en tant que LNAV Boeing
        - AP2                                 VNAV
        - EXPED                               A/P
        - ...
     */
    
    static const std::unordered_map <uint16_t, FCUEfisButtonDef> buttons = {
        
        // ====================================================================
        // MCP - MODE CONTROL PANEL (Centre)
        // ====================================================================
        
        {0, {"SPD", "777/MCP/spd_sel_button"}},         // OK? Bouton FCU "MACH"
        {1, {"LOC", "777/MCP/loc_button"}},             // OK? Bouton FCU "LOC"
        //{2, },                                          // Bouton FCU "TRK" à affecter
        {3, {"LNAV", "777/MCP/lnav_button"}},           // OK? Bouton FCU "AP1" détourné vers LNAV
        {4, {"VNAV", "777/MCP/vnav_button"}},           // OK? Bouton FCU "AP2" détourné vers VNAV
        {5, {"A/THR", "777/MCP/autothrottle_button"}},  // OK? Bouton FCU "A/THR"
        {6, {"CMD L", "777/MCP/cmd_left_button"}},      // OK? Bouton FCU "EXPED" détourné vers A/P ?
        //{7, },                                          // Bouton FCU "METRIC" à affecter
        {8, {"APP", "777/MCP/app_button"}},             // OK? Bouton FCU "APPR"
        // Commandes orphelines :
        // {"CMD R", "777/MCP/cmd_right_button"}
        // {"FLCH", "777/MCP/flch_button"}
        
        // Rotary encoders - Speed
        {9, {"SPD DEC", "sim/autopilot/airspeed_down"}},    // OK? Bouton FCU "SPEED -"
        {10, {"SPD INC", "sim/autopilot/airspeed_up"}},     // OK? Bouton FCU "SPEED +"
        {11, {"SPD", "777/MCP/spd_sel_button"}},            // OK? Bouton FCU "SPEED PUSH"
        //{12, },                                             // Bouton FCU "SPEED PULL" à affecter
        
        // Rotary encoders - Heading
        {13, {"HDG DEC", "sim/autopilot/heading_down"}},    // OK? Bouton FCU "HDG -"
        {14, {"HDG INC", "sim/autopilot/heading_up"}},      // OK? Bouton FCU "HDG +"
        {15, {"HDG HOLD", "777/MCP/hdg_hold_button"}},      // OK? Bouton FCU "HDG PUSH"
        //{16, },                                             // Bouton FCU "HDG PULL" à affecter
        // Commande orpheline :
        // {"CO", "777/MCP/co_button"}
        
        // Rotary encoders - Altitude
        {17, {"ALT DEC", "sim/autopilot/altitude_down"}},       // OK? Bouton FCU "ALT -"
        {18, {"ALT INC", "sim/autopilot/altitude_up"}},         // OK? Bouton FCU "ALT +"
        {19, {"ALT HOLD", "777/MCP/alt_hold_button"}},          // OK? Bouton FCU "ALT PUSH"
        //{20, },                                                // Bouton FCU "ALT PULL" à affecter
        
        // Rotary encoders - Vertical Speed
        {21, {"VS DEC", "sim/autopilot/vertical_speed_down"}},  // OK? Bouton FCU "VS -"
        {22, {"VS INC", "sim/autopilot/vertical_speed_up"}},    // OK? Bouton FCU "VS +"
        {23, {"V/S", "777/MCP/vs_button"}},                     // OK? Bouton FCU "VS PUSH"
        //{24, },                                                 // Bouton FCU "VS PULL" à affecter
        
        // Altitude par 100/1000
        {25, {"ALT 100", "777/MCP/alt_increment", FCUEfisDatarefType::SET_VALUE, 0.0}},     // OK? Bouton FCU "ALT 100"
        {26, {"ALT 1000", "777/MCP/alt_increment", FCUEfisDatarefType::SET_VALUE, 1.0}},    // OK? Bouton FCU "ALT 1000"
        
        
        // ====================================================================
        // EFIS CAPTAIN (Gauche) - Buttons 32-63
        // ====================================================================
        
        // Boutons FD et LS
        {32, {"R_FD", "777/displays/captain_fd_toggle"}},                    // OK? Bouton EFIS-R "FD"
        //{33, },                                                              // Bouton EFIS-R "LS" à affecter
        // Dataref orpheline :
        // {"R_MINIMUMS", "777/displays/captain_minimums_toggle"}
        
        // ND Options
        // Les boutons seront sans doute à redistribuer car il n'y a pas de vraie correspondance entre Airbus et Boeing
        {34, {"R_DATA", "777/displays/captain_show_data"}},  // OK? Bouton EFIS-R "CSTR"
        {35, {"R_WPT", "777/displays/captain_show_wpt"}},    // OK? Bouton EFIS-R "WPT"
        {36, {"R_STA", "777/displays/captain_show_sta"}},    // OK? Bouton EFIS-R "VOR.D" - Usage détourné
        //{37, },                                              // Bouton EFIS-R "NDB" à affecter
        {38, {"R_ARPT", "777/displays/captain_show_arpt"}},  // OK? Bouton EFIS-R "ARPT"
        
        // BARO
        {39, {"R_BARO PUSH", "777/displays/captain_baro_push"}},                                    // OK? Bouton EFIS-R "STD PUSH"
        {40, {"R_BARO PULL", "777/displays/captain_baro_pull"}},                                    // OK? Bouton EFIS-R "STD PULL" existe ?
        {41, {"R_BARO DEC", "custom", FCUEfisDatarefType::BAROMETER_FO, -1.0}},                     // OK? Bouton EFIS-R "PRESS -"
        {42, {"R_BARO INC", "custom", FCUEfisDatarefType::BAROMETER_FO, 1.0}},                      // OK? Bouton EFIS-R "PRESS +"
        {43, {"R_inHg", "777/displays/captain_baro_unit", FCUEfisDatarefType::SET_VALUE, 0.0}},     // OK? Bouton EFIS-R "INHG"
        {44, {"R_hPa", "777/displays/captain_baro_unit", FCUEfisDatarefType::SET_VALUE, 1.0}},      // OK? Bouton EFIS-R "HPA"
        
        // ND Mode selector
        // Les positions seront sans doute à redistribuer après tests
        {45, {"R_MODE APP", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 0.0}},   // OK? Bouton EFIS-R "MODE LS"
        {45, {"R_MODE VOR", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 1.0}},   // OK? Bouton EFIS-R "MODE VOR"
        //{47, },                                                                                     // Bouton EFIS-R "MODE NAV" à affecter
        {48, {"R_MODE MAP", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 2.0}},   // OK? Bouton EFIS-R "MODE ARC"
        {49, {"R_MODE PLAN", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 3.0}},  // OK? Bouton EFIS-R "MODE PLAN"
        
        // ND Range selector
        // Les positions seront sans doute à redistribuer après tests
        {50, {"R_RANGE 10", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 1.0}},  // OK? Bouton EFIS-R "RANGE 10"
        {51, {"R_RANGE 20", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 2.0}},  // OK? Bouton EFIS-R "RANGE 20"
        {52, {"R_RANGE 40", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 3.0}},  // OK? Bouton EFIS-R "RANGE 40"
        {53, {"R_RANGE 80", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 4.0}},  // OK? Bouton EFIS-R "RANGE 80"
        {54, {"R_RANGE 160", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 5.0}}, // OK? Bouton EFIS-R "RANGE 160"
        {55, {"R_RANGE 320", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 6.0}}, // OK? Bouton EFIS-R "RANGE 320"
        // Datarefs orphelines :
        // {"R_RANGE 5", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 0.0}        // Position inexistante dans le B777
        // {"R_RANGE 640", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 7.0}      // Position inexistante sur l'EFIS
        
        // VOR/ADF selectors (NON inversés par rapport à Captain)
        // Les positions seront sans doute à redistribuer après tests
        {56, {"R_VOR1 R", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0}},  // OK? Bouton EFIS-R "1-VOR"
        {57, {"R_VOR1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0}},// OK? Bouton EFIS-R "1-OFF"
        {58, {"R_VOR1 L", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0}},  // OK? Bouton EFIS-R "1-ADF"
        {59, {"R_VOR2 R", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0}},  // OK? Bouton EFIS-R "2-VOR"
        {60, {"R_VOR2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0}},// OK? Bouton EFIS-R "2-OFF"
        {61, {"R_VOR2 L", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0}},  // OK? Bouton EFIS-R "2-ADF"
        // Datarefs orphelines
        // {"R_POS", "777/displays/captain_show_pos"}
        // {"R_TERR", "777/displays/captain_show_terrain"}
        
        // BOUTONS RÉSERVÉS
        //{62, },   // Bouton ????
        //{63, },   // Bouton ????
        
        
        // ====================================================================
        // EFIS FIRST OFFICER (Droite) - Buttons 64-95
        // ====================================================================
        
        // Boutons FD et LS
        {64, {"R_FD", "777/displays/fo_fd_toggle"}},                    // OK? Bouton EFIS-R "FD"
        //{65, },                                                         // Bouton EFIS-R "LS" à affecter
        // Dataref orpheline :
        // {"R_MINIMUMS", "777/displays/fo_minimums_toggle"}
        
        // ND Options
        // Les boutons seront sans doute à redistribuer car il n'y a pas de vraie correspondance entre Airbus et Boeing
        {66, {"R_DATA", "777/displays/fo_show_data"}},  // OK? Bouton EFIS-R "CSTR"
        {67, {"R_WPT", "777/displays/fo_show_wpt"}},    // OK? Bouton EFIS-R "WPT"
        {68, {"R_STA", "777/displays/fo_show_sta"}},    // OK? Bouton EFIS-R "VOR.D" - Usage détourné
        //{69, },                                         // Bouton EFIS-R "NDB" à affecter
        {70, {"R_ARPT", "777/displays/fo_show_arpt"}},  // OK? Bouton EFIS-R "ARPT"  - Usage détourné
        
        // BARO
        {71, {"R_BARO PUSH", "777/displays/fo_baro_push"}},                                     // OK? Bouton EFIS-R "STD PUSH"
        {72, {"R_BARO PULL", "777/displays/fo_baro_pull"}},                                     // OK? Bouton EFIS-R "STD PULL" existe vraiment ?
        {73, {"R_BARO DEC", "custom", FCUEfisDatarefType::BAROMETER_FO, -1.0}},                 // OK? Bouton EFIS-R "PRESS -"
        {74, {"R_BARO INC", "custom", FCUEfisDatarefType::BAROMETER_FO, 1.0}},                  // OK? Bouton EFIS-R "PRESS +"
        {75, {"R_inHg", "777/displays/fo_baro_unit", FCUEfisDatarefType::SET_VALUE, 0.0}},      // OK? Bouton EFIS-R "INHG"
        {76, {"R_hPa", "777/displays/fo_baro_unit", FCUEfisDatarefType::SET_VALUE, 1.0}},       // OK? Bouton EFIS-R "HPA"
        
        // ND Mode selector
        // Les positions seront sans doute à redistribuer après tests
        {77, {"R_MODE APP", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 0.0}},   // OK? Bouton EFIS-R "MODE LS"
        {78, {"R_MODE VOR", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 1.0}},   // OK? Bouton EFIS-R "MODE VOR"
        //{79, },                                                                                // Bouton EFIS-R "MODE NAV" à affecter
        {80, {"R_MODE MAP", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 2.0}},   // OK? Bouton EFIS-R "MODE ARC"
        {81, {"R_MODE PLAN", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 3.0}},  // OK? Bouton EFIS-R "MODE PLAN"
        
        // ND Range selector
        // Les positions seront sans doute à redistribuer après tests
        {82, {"R_RANGE 10", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 1.0}},  // OK? Bouton EFIS-R "RANGE 10"
        {83, {"R_RANGE 20", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 2.0}},  // OK? Bouton EFIS-R "RANGE 20"
        {84, {"R_RANGE 40", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 3.0}},  // OK? Bouton EFIS-R "RANGE 40"
        {85, {"R_RANGE 80", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 4.0}},  // OK? Bouton EFIS-R "RANGE 80"
        {86, {"R_RANGE 160", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 5.0}}, // OK? Bouton EFIS-R "RANGE 160"
        {87, {"R_RANGE 320", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 6.0}}, // OK? Bouton EFIS-R "RANGE 320"
        // Datarefs orphelines :
        // {"R_RANGE 5", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 0.0}        // Position inexistante dans le B777
        // {"R_RANGE 640", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 7.0}      // Position inexistante sur l'EFIS
        
        // VOR/ADF selectors (NON inversés par rapport à Captain)
        // Les positions seront sans doute à redistribuer après tests
        {88, {"R_VOR1 R", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0}},  // OK? Bouton EFIS-R "1-VOR"
        {89, {"R_VOR1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0}},// OK? Bouton EFIS-R "1-OFF"
        {90, {"R_VOR1 L", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0}},  // OK? Bouton EFIS-R "1-ADF"
        {91, {"R_VOR2 R", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0}},  // OK? Bouton EFIS-R "2-VOR"
        {92, {"R_VOR2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0}},// OK? Bouton EFIS-R "2-OFF"
        {93, {"R_VOR2 L", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0}},  // OK? Bouton EFIS-R "2-ADF"
        // Datarefs orphelines
        // {"R_POS", "777/displays/fo_show_pos"}
        // {"R_TERR", "777/displays/fo_show_terrain"}
        
        // BOUTONS RÉSERVÉS
        //{94, }, // Bouton ????
        //{95, }, // Bouton ????
        
    };
    return buttons;
}

// ===============================================================================================================
// Méthode pour mise à jour des données affichées par les afficheurs
void FF777FCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    auto datarefManager = Dataref::getInstance();

    // État général
    data.displayEnabled = datarefManager->getCached<bool>("T7Avionics/mcp_power");
    data.displayTest = isTestMode();

    // ========================================================================
    // SPEED DISPLAY
    // ========================================================================
    data.spdMach = datarefManager->getCached<bool>("sim/cockpit/autopilot/airspeed_is_mach");
    float speed = datarefManager->getCached<float>("sim/cockpit2/autopilot/airspeed_dial_kts_mach");

    if (speed > 0) {
        std::stringstream ss;
        if (data.spdMach) {
            // Mode Mach: 0.82 -> "082"
            int machHundredths = static_cast<int>(std::round(speed * 100));
            ss << std::setfill('0') << std::setw(3) << machHundredths;
        } else {
            // Mode Speed: format en entier
            ss << std::setfill('0') << std::setw(3) << static_cast<int>(speed);
        }
        data.speed = ss.str();
    } else {
        data.speed = "---";
    }

    // Mode managé (B777 utilise différents modes: SPD, FLCH, etc.)
    // TODO: Adapter selon les modes du B777
    data.spdManaged = false; // Le B777 n'a pas vraiment de mode "managed" comme Airbus

    // ==================
    // HEADING DISPLAY
    // ==================
    float heading = datarefManager->getCached<float>("sim/cockpit/autopilot/heading_mag");
    if (heading >= 0) {
        int hdgDisplay = static_cast<int>(heading) % 360;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << hdgDisplay;
        data.heading = ss.str();
    } else {
        data.heading = "---";
    }

    data.hdgManaged = false; // Pas de mode managed sur B777
    data.hdgTrk = false;     // Le B777 affiche toujours HDG (pas de mode TRK)

    // ===================
    // ALTITUDE DISPLAY
    // ===================
    float altitude = datarefManager->getCached<float>("sim/cockpit/autopilot/altitude");
    if (altitude >= 0) {
        int altInt = static_cast<int>(altitude);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << altInt;
        data.altitude = ss.str();
    } else {
        data.altitude = "-----";
    }

    data.altManaged = false; // Pas de mode managed sur B777

    // =======================
    // VERTICAL SPEED DISPLAY
    // =======================
    float vs = datarefManager->getCached<float>("sim/cockpit/autopilot/vertical_velocity");
    
    // Le B777 affiche toujours en VS (pas de mode FPA)
    data.vsMode = true;
    data.fpaMode = false;
    
    std::stringstream ss;
    int vsInt = static_cast<int>(std::round(vs));
    int absVs = std::abs(vsInt);

    // Format B777: affiche toujours les valeurs complètes
    ss << std::setfill('0') << std::setw(4) << absVs;
    data.verticalSpeed = ss.str();

    data.vsSign = (vs >= 0);
    data.fpaComma = false;
    data.vsIndication = true;
    data.fpaIndication = false;
    data.vsVerticalLine = true;

    // Mode lateral (toujours actif)
    data.latMode = true;

    // ========================
    // EFIS BAROMETER DISPLAYS
    // ========================
    for (int i = 0; i < 2; i++) {
        bool isCaptain = i == 0;

        bool isStd = datarefManager->getCached<bool>(
            isCaptain ? "777/displays/captain_baro_std" : "777/displays/fo_baro_std"
        );
        bool isBaroHpa = datarefManager->getCached<bool>(
            isCaptain ? "777/displays/captain_baro_unit" : "777/displays/fo_baro_unit"
        );
        float baroValue = datarefManager->getCached<float>(
            isCaptain ? "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot"
                      : "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot"
        );

        EfisDisplayValue value = {
            .displayEnabled = data.displayEnabled,
            .displayTest = data.displayTest,
            .baro = "",
            .unitIsInHg = !isBaroHpa,
            .isStd = isStd,
        };

        if (!isStd && baroValue > 0) {
            value.setBaro(baroValue, !isBaroHpa);
        }

        if (isCaptain) {
            data.efisLeft = value;
        } else {
            data.efisRight = value;
        }
    }
}


// ===============================================================================================================
void FF777FCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();

    // =======================
    // GESTION DES BAROMETERS
    // =======================
    if (phase == xplm_CommandBegin &&
        (button->datarefType == FCUEfisDatarefType::BAROMETER_PILOT ||
         button->datarefType == FCUEfisDatarefType::BAROMETER_FO)) {
        
        bool isCaptain = button->datarefType == FCUEfisDatarefType::BAROMETER_PILOT;
        bool isStd = datarefManager->getCached<bool>(
            isCaptain ? "777/displays/captain_baro_std" : "777/displays/fo_baro_std"
        );
        
        if (isStd) {
            return; // Pas d'ajustement en mode STD
        }

        bool isBaroHpa = datarefManager->getCached<bool>(
            isCaptain ? "777/displays/captain_baro_unit" : "777/displays/fo_baro_unit"
        );
        const char *datarefName = isCaptain
            ? "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot"
            : "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot";
        
        float baroValue = datarefManager->getCached<float>(datarefName);
        bool increase = button->value > 0;

        if (isBaroHpa) {
            // Conversion inHg -> hPa, ajustement, conversion inverse
            float hpaValue = baroValue * 33.8639f;
            hpaValue += increase ? 1.0f : -1.0f;
            baroValue = hpaValue / 33.8639f;
        } else {
            // Ajustement direct en inHg (0.01 par clic)
            baroValue += increase ? 0.01f : -0.01f;
        }

        datarefManager->set<float>(datarefName, baroValue);
    }
    
    // ==============================
    // GESTION DES BOUTONS SET_VALUE
    // ==============================
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::SET_VALUE) {
        datarefManager->set<float>(button->dataref.c_str(), button->value);
    }
    
    // ============================
    // GESTION DES BOUTONS TOGGLE
    // ============================
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE) {
        int currentValue = datarefManager->get<int>(button->dataref.c_str());
        int newValue = currentValue ? 0 : 1;
        datarefManager->set<int>(button->dataref.c_str(), newValue);
    }
    
    // ==============================
    // GESTION DES COMMANDES X-PLANE
    // ==============================
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    }
}


// ===============================================================================================================
// Méthode lançant le mode test d'affichage
bool FF777FCUEfisProfile::isTestMode() {
    // TODO: Identifier la dataref de mode test pour le B777
    // Exemple hypothétique: "777/test/display_test"
    
    // return Dataref::getInstance()->get<int>("777/test/display_test") == 1;
    return false; // Par défaut, pas de mode test
}
