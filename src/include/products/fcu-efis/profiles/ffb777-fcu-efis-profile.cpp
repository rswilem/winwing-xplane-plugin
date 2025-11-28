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


// OK ===============================================================================================================
// Constructeur de la classe
FF777FCUEfisProfile::FF777FCUEfisProfile(ProductFCUEfis *product) :
    FCUEfisAircraftProfile(product) {
    
    // ====================================================
    // SECTION 1: MONITORING DE L'ÉCLAIRAGE ET ALIMENTATION
    // ====================================================
    
    // Test d'allumage du MCP
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/output/mcp/ok", [product](bool hasPower) {
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
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/ap_engage_a", [this, product](int engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    // Autopilot B (CMD R sur B777)
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/ap_engage_b", [this, product](int engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    // Autothrottle
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/autothrottle_arm", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    // LOC mode
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/loc", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, armed || isTestMode() ? 1 : 0);
    });

    // APPROACH mode
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/app", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    // ====================================================
    // SECTION 3: MONITORING DES LEDs EFIS CAPTAIN (Gauche)
    // ====================================================
    
    // Flight Director Captain
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/fd_capt", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    // AUTRES BOUTONS EFIS CAPTAIN
    // Touche WPT Captain
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/capt/wpt_toggle", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
    
    // Touche ARPT Captain
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/capt/arpt_toggle", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
        
    // Touche DATA Captain
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/capt/data_toggle", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
        
    // - "777/displays/captain/show_vor"                // DATAREF À TROUVER
    // - "777/displays/captain/show_ndb"                // DATAREF À TROUVER
        
    // ==========================================================
    // SECTION 4: MONITORING DES LEDs EFIS FIRST OFFICER (Droite)
    // ==========================================================
    
    // Flight Director First Officer
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/fd_fo", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    // TODO: Mapping des autres boutons EFIS FO selon datarefs B777
        
    // Touche WPT First Officer
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/fo/wpt_toggle", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
    
    // Touche ARPT First Officer
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/fo/arpt_toggle", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
        
    // Touche DATA First Officer
    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/efis/fo/data_toggle", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });
        
    // - "777/displays/fo/show_vor"                // DATAREF À TROUVER
    // - "777/displays/fo/show_ndb"                // DATAREF À TROUVER
        
} // !Constructeur


// OK ? ===============================================================================================================
// Destructeur de la classe
FF777FCUEfisProfile::~FF777FCUEfisProfile() {
    // Unbind de toutes les datarefs monitorées
    Dataref::getInstance()->unbind("1-sim/output/mcp/ok");
    
    // MCP datarefs
    Dataref::getInstance()->unbind("1-sim/output/mcp/ap_engage_a");
    Dataref::getInstance()->unbind("1-sim/output/mcp/ap_engage_b");
    Dataref::getInstance()->unbind("1-sim/output/mcp/autothrottle_arm");
    Dataref::getInstance()->unbind("1-sim/output/mcp/loc");
    Dataref::getInstance()->unbind("1-sim/output/mcp/app");
    
    // EFIS datarefs
    Dataref::getInstance()->unbind("1-sim/output/mcp/fd_capt");
    Dataref::getInstance()->unbind("1-sim/output/mcp/fd_fo");
    
    // TODO: Unbind des autres datarefs ajoutées
    
} // !Destructeur


// OK ============================================================================================================
// Méthode permettant de vérifier que c'est bien le FFB777 qui est chargé
bool FF777FCUEfisProfile::IsEligible() {
    /* On vérifie la présence d'au moins une dataref spécifique du FF B777, qui attestera que cet avion est bien chargé.
     Sachant que les datarefs testées ici n'ont pas vocation à provoquer des actions ou à être modifiées, on peut
     utiliser des datarefs '/anim' idéales pour des besoins de simple monitoring.
     Les datarefs choisies pour le test sont des datarefs dont l'existence est confirmée par DataRefTool ou FF */
    bool eligible = Dataref::getInstance()->exists("1-sim/ckpt/mcpApLButton/anim") ||
    Dataref::getInstance()->exists("1-sim/ckpt/cptHsiRangeSwitch/anim") ||
    Dataref::getInstance()->exists("1-sim/output/mcp/ok");
    
    return eligible;
    
} // !IsEligible


// KO ===============================================================================================================
// MÉTHODE RETOURNANT LA 'LISTE' DES DATAREFS DE MONITORING
const std::vector <std::string> &FF777FCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // Alimentation et état
        //"T7Avionics/mcp_power",
        "1-sim/output/mcp/ok",
        
        // MCP - Speed
        //"sim/cockpit2/autopilot/airspeed_dial_kts_mach",
        //"sim/cockpit/autopilot/airspeed_is_mach",
        "1-sim/output/mcp/isMachTrg"
        //"777/autopilot/speed_mode", // SPD, FLCH, etc.
        "1-sim/output/mcp/spd",
        "1-sim/output/mcp/fma_spd_mode",
        
        // MCP - Heading
        //"sim/cockpit/autopilot/heading_mag",
        //"777/autopilot/heading_hold_active",
        "1-sim/output/mcp/hdg",
        "1-sim/output/mcp/fma_hdg_mode",
        
        // MCP - Altitude
        //"sim/cockpit/autopilot/altitude",
        //"777/autopilot/altitude_hold_active",
        "1-sim/output/mcp/alt",
        "1-sim/output/mcp/fma_alt_mode",
        
        // MCP - Vertical Speed
        //"sim/cockpit/autopilot/vertical_velocity",
        //"777/autopilot/vnav_active",
        "1-sim/output/mcp/vs",
        "1-sim/output/mcp/fma_vs_mode",
        
        // EFIS - Barometric settings
        // "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot",
        // "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot",
        "1-sim/output/efis/capt/baro_value",
        "1-sim/output/efis/fo/baro_value",
        "777/displays/captain_baro_std",                                        // DATAREF À TROUVER
        "777/displays/fo_baro_std",                                             // DATAREF À TROUVER
        // "777/displays/captain_baro_unit", // 0=inHg, 1=hPa
        // "777/displays/fo_baro_unit",
        "1-sim/output/efis/capt/baro_mode", // 0=inHg, 1=hPa
        "1-sim/output/efis/fo/baro_mode",
        
        // ND Mode et Range
        // "777/displays/captain_nd_mode", // 0=APP, 1=VOR, 2=MAP, 3=PLAN
        // "777/displays/fo_nd_mode",
        // "777/displays/captain_nd_range",
        // "777/displays/fo_nd_range",
        "1-sim/efis/capt/nd_mode",      // 0=APP, 1=VOR, 2=MAP, 3=PLAN
        "1-sim/efis/FO/nd_mode",
        "1-sim/efis/capt/nd_range",
        "1-sim/efis/fo/nd_range",
        
    };
    
    return datarefs;
    
} // !displayDatarefs


// ===============================================================================================================
// MÉTHODE RENVOYANT LE MAPPAGE DES BOUTONS FCU+EFIS AVEC LES COMMANDES ASSOCIÉES
/*
Les FCU EFIS Winwing étant conçus pour Airbus, il est important de garder en tête à quel n° de bouton est associée
l'action attendue sur Airbus (indiquée par le bouton) et de lui affecter ce qui s'y rapporte le mieux sur le B777.
Les associations Airbus sont rappelées en commentaire, face à chaque bouton.
Dans la majorité des cas la tranposition est naturelle, mais il y a qq exceptions qui créent autant d'entorses :
    - AP1 Airbus sera utilisé en tant que LNAV Boeing
    - AP2                                 VNAV
    - EXPED                               A/P
    - ...                                       */
const std::unordered_map <uint16_t, FCUEfisButtonDef> &FF777FCUEfisProfile::buttonDefs() const {
    
    static const std::unordered_map <uint16_t, FCUEfisButtonDef> buttons = {
        
        // ====================================================================
        // MCP - MODE CONTROL PANEL (Centre)
        // ====================================================================
        
        {0, {"SPD",     "1-sim/command/mcpIasMachButton_button"}},      // Bouton FCU "MACH"
        {1, {"LOC",     "1-sim/command/mcpLocButton_button"}},          // Bouton FCU "LOC"
        //{2, },                                                        // Bouton FCU "TRK" à affecter
        {3, {"LNAV",    "1-sim/command/mcpLnavButton_button"}},         // Bouton FCU "AP1" DÉTOURNÉ vers LNAV
        {4, {"VNAV",    "1-sim/command/mcpVnavButton_button"}},         // Bouton FCU "AP2" DÉTOURNÉ vers VNAV
        {5, {"A/THR",   "1-sim/command/mcpAtButton_button"}},           // Bouton FCU "A/THR"
        {6, {"A/P L",   "1-sim/command/mcpApLButton_button"}},          // Bouton FCU "EXPED" DÉTOURNÉ vers A/P ?
        //{7, },                                                        // Bouton FCU "METRIC" à affecter
        {8, {"APP",     "1-sim/command/mcpAppButton_button"}},          // Bouton FCU "APPR"
        // Commandes orphelines :
        // {"CMD R", ""}
        // {"FLCH", ""}
        
        // Rotary encoders - Speed
        {9,  {"SPD DEC", "1-sim/command/mcpSpdRotary_rotary-"}},        // Bouton FCU "SPEED -"
        {10, {"SPD INC", "1-sim/command/mcpSpdRotary_rotary+"}},        // Bouton FCU "SPEED +"
        {11, {"SPD",     "1-sim/command/mcpSpdRotary_push"}},           // Bouton FCU "SPEED PUSH"
        //{12, },                                                       // Bouton FCU "SPEED PULL" à affecter ?
        
        // Rotary encoders - Heading
        {13, {"HDG DEC",  "1-sim/command/mcpHdgRotary_rotary-"}},       // Bouton FCU "HDG -"
        {14, {"HDG INC",  "1-sim/command/mcpHdgRotary_rotary+"}},       // Bouton FCU "HDG +"
        {15, {"HDG HOLD", "1-sim/command/mcpHdgHoldButton_button"}},    // Bouton FCU "HDG PUSH"
        //{16, },                                                       // Bouton FCU "HDG PULL" à affecter ?
        // Commande orpheline :
        // {"CO", ""}
        
        // Rotary encoders - Altitude
        {17, {"ALT DEC",  "1-sim/command/mcpAltRotary_rotary-"}},       // Bouton FCU "ALT -"
        {18, {"ALT INC",  "1-sim/command/mcpAltRotary_rotary+"}},       // Bouton FCU "ALT +"
        {19, {"ALT HOLD", "1-sim/command/mcpAltRotary_push"}},          // Bouton FCU "ALT PUSH"
        //{20, },                                                       // Bouton FCU "ALT PULL" à affecter ?
        
        // Rotary encoders - Vertical Speed
        {21, {"VS DEC", "1-sim/command/mcpVsRotary_rotary-"}},          // Bouton FCU "VS -"
        {22, {"VS INC", "1-sim/command/mcpVsRotary_rotary+"}},          // Bouton FCU "VS +"
        {23, {"V/S",    "1-sim/command/mcpVsButton_button"}},           // Bouton FCU "VS PUSH"
        //{24, },                                                       // Bouton FCU "VS PULL" à affecter
        
        // Altitude par 100/1000
        {25, {"ALT 100",  "1-sim/command/mcpAltModeSwitch_set_0"}},     // Bouton FCU "ALT 100"
        {26, {"ALT 1000", "1-sim/command/mcpAltModeSwitch_set_1"}},     // Bouton FCU "ALT 1000"
        
        
        // ====================================================================
        // EFIS CAPTAIN (Gauche) - Buttons 32-63
        // ====================================================================
        
        // Boutons FD et LS
        {32, {"L_FD", "1-sim/command/mcpFdLSwitch_trigger"}},           // Bouton EFIS-L "FD"
        //{33, },                                                       // Bouton EFIS-L "LS" à affecter
        // Commande orpheline à défaut de bouton existant
        // {"R_MINIMUMS", ""}
        
        // ND Options
        // Les boutons seront sans doute à redistribuer car il n'y a pas de vraie correspondance entre Airbus et Boeing
        {34, {"L_DATA", "1-sim/command/cptHsiDataButton_button"}},      // Bouton EFIS-L "CSTR"
        {35, {"L_WPT",  "1-sim/command/cptHsiWptButton_button"}},       // Bouton EFIS-L "WPT"
        {36, {"L_STA",  "1-sim/command/cptHsiStaButton_button"}},       // Bouton EFIS-L "VOR.D" - Usage détourné
        //{37, },                                                       // Bouton EFIS-L "NDB" à affecter
        {38, {"L_ARPT", "1-sim/command/cptHsiArpButton_button"}},       // Bouton EFIS-L "ARPT"
        
        // BARO
        {39, {"L_BARO PUSH", "1-sim/command/cptHsiStdButton_button"}},      // Bouton EFIS-L "STD PUSH"
        //{40, {"R_BARO PULL", "777/displays/captain_baro_pull"}},          // Bouton EFIS-L "STD PULL" existe ?
        {41, {"L_BARO DEC",  "1-sim/command/cptHsiBaroRotary_rotary-"}},    // Bouton EFIS-L "PRESS -"
        {42, {"L_BARO INC",  "1-sim/command/cptHsiBaroRotary_rotary+"}},    // Bouton EFIS-L "PRESS +"
        {43, {"L_inHg",      "1-sim/command/cptHsiBaroModeRotary_set_0"}},  // Bouton EFIS-L "INHG"
        {44, {"L_hPa",       "1-sim/command/cptHsiBaroModeRotary_set_1"}},  // Bouton EFIS-L "HPA"
        
        // ND Mode selector
        // Les positions seront sans doute à redistribuer après tests
        {45, {"L_MODE APP",  "1-sim/command/cptHsiModeSwitch_set_0"}},  // Bouton EFIS-L "MODE LS"
        {46, {"L_MODE VOR",  "1-sim/command/cptHsiModeSwitch_set_1"}},  // Bouton EFIS-L "MODE VOR"
        //{47, },                                                       // Bouton EFIS-L "MODE NAV" à affecter?
        {48, {"L_MODE MAP",  "1-sim/command/cptHsiModeSwitch_set_2"}},  // Bouton EFIS-L "MODE ARC"
        {49, {"L_MODE PLAN", "1-sim/command/cptHsiModeSwitch_set_3"}},  // Bouton EFIS-L "MODE PLAN"
        
        // ND Range selector
        // Les positions seront sans doute à redistribuer après tests
        {50, {"L_RANGE 10",  "1-sim/command/cptHsiRangeSwitch_set_0"}}, // Bouton EFIS-L "RANGE 10"
        {51, {"L_RANGE 20",  "1-sim/command/cptHsiRangeSwitch_set_1"}}, // Bouton EFIS-L "RANGE 20"
        {52, {"L_RANGE 40",  "1-sim/command/cptHsiRangeSwitch_set_2"}}, // Bouton EFIS-L "RANGE 40"
        {53, {"L_RANGE 80",  "1-sim/command/cptHsiRangeSwitch_set_3"}}, // Bouton EFIS-L "RANGE 80"
        {54, {"L_RANGE 160", "1-sim/command/cptHsiRangeSwitch_set_4"}}, // Bouton EFIS-L "RANGE 160"
        {55, {"L_RANGE 320", "1-sim/command/cptHsiRangeSwitch_set_5"}}, // Bouton EFIS-L "RANGE 320"
        // Commandes orphelines (inexistantes sur l'avion ou l'EFIS) :
        // {"R_RANGE 5", }                                              // Position inexistante dans le B777
        // {"R_RANGE 640", }                                            // Position inexistante sur l'EFIS
        
        // VOR/ADF selectors
        // Les positions seront sans doute à redistribuer après tests
        {56, {"L_VORL VOR", "1-sim/command/cptHsiVorLSwitch_trigger"}}, // Bouton EFIS-L "L-VOR"
        {57, {"L_VORL OFF", "1-sim/command/cptHsiVorLSwitch_trigger"}}, // Bouton EFIS-L "L-OFF"
        {58, {"L_VORL ADF", "1-sim/command/cptHsiVorLSwitch_trigger"}}, // Bouton EFIS-L "L-ADF"
        {59, {"L_VORR VOR", "1-sim/command/cptHsiVorRSwitch_trigger"}}, // Bouton EFIS-L "R-VOR"
        {60, {"L_VORR OFF", "1-sim/command/cptHsiVorRSwitch_trigger"}}, // Bouton EFIS-L "R-OFF"
        {61, {"L_VORR ADF", "1-sim/command/cptHsiVorRSwitch_trigger"}}, // Bouton EFIS-L "R-ADF"
        
        // COMMANDES EN ATTENTE D'UN BOUTON ERGONOMIQUEMENT ACCEPTABLE PARMI CEUX ENCORE LIBRES SUR LE FCU/EFIS AIRBUS
        // {XX, {"R_POS",  ""}},                                        // Bouton EFIS-L "POS"
        // {XX, {"R_TERR", ""}},                                        // Bouton EFIS-L "TERR"
        
        // BOUTONS RÉSERVÉS
        //{62, },   // Bouton ????
        //{63, },   // Bouton ????
        
        
        // ====================================================================
        // EFIS FIRST OFFICER (Droite) - Buttons 64-95
        // ====================================================================
        
        // Boutons FD et LS
        {64, {"R_FD", "1-sim/command/mcpFdRSwitch_trigger"}},           // Bouton EFIS-R "FD"
        //{65, },                                                       // Bouton EFIS-R "LS" à affecter
        // Commande orpheline :
        // {"R_MINIMUMS", "777/displays/fo_minimums_toggle"}
        
        // ND Options
        // Les boutons seront sans doute à redistribuer car il n'y a pas de vraie correspondance entre Airbus et Boeing
        {66, {"R_DATA", "1-sim/command/foHsiDataButton_button"}},       // Bouton EFIS-R "CSTR"
        {67, {"R_WPT",  "1-sim/command/foHsiWptButton_button"}},        // Bouton EFIS-R "WPT"
        {68, {"R_STA",  "1-sim/command/foHsiStaButton_button"}},        // Bouton EFIS-R "VOR.D" - Usage détourné
        //{69, },                                                       // Bouton EFIS-R "NDB" à affecter
        {70, {"R_ARPT", "1-sim/command/foHsiArpButton_button"}},        // Bouton EFIS-R "ARPT"
        
        // BARO
        {71, {"R_BARO PUSH", "1-sim/command/foHsiStdButton_button"}},       // Bouton EFIS-R "STD PUSH"
        //{72, {"R_BARO PULL", "777/displays/fo_baro_pull"}},               // Bouton EFIS-R "STD PULL" existe ?
        {73, {"R_BARO DEC",  "1-sim/command/foHsiBaroRotary_rotary-"}},     // Bouton EFIS-R "PRESS -"
        {74, {"R_BARO INC",  "1-sim/command/foHsiBaroRotary_rotary+"}},     // Bouton EFIS-R "PRESS +"
        {75, {"R_inHg",      "1-sim/command/foHsiBaroModeRotary_set_0"}},   // Bouton EFIS-R "INHG"
        {76, {"R_hPa",       "1-sim/command/foHsiBaroModeRotary_set_1"}},   // Bouton EFIS-R "HPA"
        
        // ND Mode selector
        // Les positions seront sans doute à redistribuer après tests
        {77, {"R_MODE APP",  "1-sim/command/foHsiModeSwitch_set_0"}},   // Bouton EFIS-R "MODE LS"
        {78, {"R_MODE VOR",  "1-sim/command/foHsiModeSwitch_set_1"}},   // Bouton EFIS-R "MODE VOR"
        //{79, },                                                       // Bouton EFIS-R "MODE NAV" à affecter
        {80, {"R_MODE MAP",  "1-sim/command/foHsiModeSwitch_set_2"}},   // Bouton EFIS-R "MODE ARC"
        {81, {"R_MODE PLAN", "1-sim/command/foHsiModeSwitch_set_3"}},   // Bouton EFIS-R "MODE PLAN"
        
        // ND Range selector
        // Les positions seront sans doute à redistribuer après tests
        {82, {"R_RANGE 10",  "1-sim/command/foHsiRangeSwitch_set_0"}},  // Bouton EFIS-R "RANGE 10"
        {83, {"R_RANGE 20",  "1-sim/command/foHsiRangeSwitch_set_1"}},  // Bouton EFIS-R "RANGE 20"
        {84, {"R_RANGE 40",  "1-sim/command/foHsiRangeSwitch_set_2"}},  // Bouton EFIS-R "RANGE 40"
        {85, {"R_RANGE 80",  "1-sim/command/foHsiRangeSwitch_set_3"}},  // Bouton EFIS-R "RANGE 80"
        {86, {"R_RANGE 160", "1-sim/command/foHsiRangeSwitch_set_4"}},  // Bouton EFIS-R "RANGE 160"
        {87, {"R_RANGE 320", "1-sim/command/foHsiRangeSwitch_set_5"}},  // Bouton EFIS-R "RANGE 320"
        // Commandes orphelines :
        // {"R_RANGE 5", }                                              // Position inexistante dans le B777
        // {"R_RANGE 640", }                                            // Position inexistante sur l'EFIS
        
        // VOR/ADF selectors
        // Les positions seront sans doute à redistribuer après tests
        {88, {"R_VORL VOR", "1-sim/command/foHsiVorLSwitch_trigger"}},  // Bouton EFIS-R "L-VOR"
        {89, {"R_VORL OFF", "1-sim/command/foHsiVorLSwitch_trigger"}},  // Bouton EFIS-R "L-OFF"
        {90, {"R_VORL ADF", "1-sim/command/foHsiVorLSwitch_trigger"}},  // Bouton EFIS-R "L-ADF"
        {91, {"R_VORR VOR", "1-sim/command/foHsiVorRSwitch_trigger"}},  // Bouton EFIS-R "R-VOR"
        {92, {"R_VORR OFF", "1-sim/command/foHsiVorRSwitch_trigger"}},  // Bouton EFIS-R "R-OFF"
        {93, {"R_VORR ADF", "1-sim/command/foHsiVorRSwitch_trigger"}},  // Bouton EFIS-R "R-ADF"
        
        // COMMANDES EN ATTENTE D'UN BOUTON ERGONOMIQUEMENT ACCEPTABLE PARMI CEUX ENCORE LIBRES SUR LE FCU/EFIS AIRBUS
        // {XX, {"R_POS",  ""}},                                        // Bouton EFIS-R "POS"
        // {XX, {"R_TERR", ""}},                                        // Bouton EFIS-R "TERR"
        
        // BOUTONS RÉSERVÉS
        //{94, }, // Bouton ????
        //{95, }, // Bouton ????
        
    };
    return buttons;
    
} // !buttonDefs


// ===============================================================================================================
// Méthode pour mise à jour des données affichées dans les cadrans
void FF777FCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    auto datarefManager = Dataref::getInstance();
    
    // État général
    //data.displayEnabled = datarefManager->getCached<bool>("T7Avionics/mcp_power");
    data.displayEnabled = datarefManager->getCached<bool>("1-sim/output/mcp/ok");
    data.displayTest = isTestMode();
    
    // ========================================================================
    // SPEED DISPLAY
    // ========================================================================
    data.spdMach = datarefManager->getCached<bool>("1-sim/output/mcp/isMachTrg");       // Dataref à CONFIRMER
    float speed = datarefManager->getCached<float>("1-sim/output/mcp/spd");             // Dataref à CONFIRMER
    
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
    float heading = datarefManager->getCached<float>("1-sim/output/mcp/hdg");           // Dataref à CONFIRMER
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
    float altitude = datarefManager->getCached<float>("1-sim/output/mcp/alt");          // Dateref à CONFIRMER
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
    float vs = datarefManager->getCached<float>("1-sim/output/mcp/vs");                 // Dataref à CONFIRMER
    
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
        
        //bool isStd = datarefManager->getCached<bool>(
        //     isCaptain ? "777/displays/captain_baro_std" : "777/displays/fo_baro_std"             // DATAREFS INTROUVABLE
        // );
        const char *side = (isCaptain ? "capt" : "fo") ;                                            // Méthode palliative
        bool isStd = isBaroSTD(side);
        
        /* Si comme je le suppose 'baro_mode' vaut 1 quand hPa, et O quand quand inHg, alors le code fonctionne */
        bool isBaroHpa = datarefManager->getCached<bool>(
            isCaptain ? "1-sim/output/efis/capt/baro_mode" : "1-sim/output/efis/fo/baro_mode"       // Datarefs à CONFIRMER
        );
        
        float baroValue = datarefManager->getCached<float>(
            isCaptain ? "1-sim/output/efis/cpt/baro_value" : "1-sim/output/efis/fo/baro_value"      // Datarefs à CONFIRMER
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
    
} // !updateDisplayData


// ===============================================================================================================
// Méthode qui gère l'appui sur un bouton selon sa nature
void FF777FCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }
    
    auto datarefManager = Dataref::getInstance();
    
    // =======================
    // GESTION DES BAROMETERS
    // =======================
    // TODO : LOGIQUE DE CE BLOC 'GESTION DES BAROMÈTRES' À S'APPROPRIER ...
    if (phase == xplm_CommandBegin &&
        (button->datarefType == FCUEfisDatarefType::BAROMETER_PILOT ||
         button->datarefType == FCUEfisDatarefType::BAROMETER_FO)) {
        
        bool isCaptain = button->datarefType == FCUEfisDatarefType::BAROMETER_PILOT;
        
        // bool isStd = datarefManager->getCached<bool>(
        //      isCaptain ? "777/displays/captain_baro_std" : "777/displays/fo_baro_std"            // DATAREF INTROUVABLE
        // );
        // Utilisation de la dataref remplacé par la méthode 'isBaroStd'
        bool isStd = isBaroSTD(isCaptain ? "capt" : "fo");                                          // Méthode palliative
        
        if (isStd) {
            return; // Pas d'ajustement en mode STD
        }
        
        /* Si comme je le suppose 'baro_mode' vaut 1 quand hPa, et O quand quand inHg, alors le code fonctionne */
        bool isBaroHpa =
        datarefManager->getCached<bool>(isCaptain ? "1-sim/output/efis/capt/baro_mode": "1-sim/output/efis/fo/baro_mode");
        
        /* DÉBUT DU BLOC ABANDONNÉ POUR CONTOURNER L'ABSENCE DE DATAREF =================== */
        //
        // const char *datarefName = isCaptain
        //    ? "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot"                   // DATAREF INTROUVABLE
        //    : "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot";                // DATAREF INTROUVABLE
        //
        // float baroValue = datarefManager->getCached<float>(datarefName);
        //
        const char *datarefName =
        isCaptain ? "1-sim/output/efis/capt/baro_value" : "1-sim/output/efis/fo/baro_value" ;   // Dataref intermédiaire
        // Appel à Méthode permettant de déterminer la valeur Baro en inHg
        float baroValue = valBaroInHg(datarefName);                                             // Méthode palliative
        //
        /* FIN DU BLOC DE SUBSTITUTION ==================================================== */
        
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
    
} // !buttonPressed


// ===============================================================================================================
// Méthode lançant le mode test d'affichage
bool FF777FCUEfisProfile::isTestMode() {
    // TODO: Identifier la dataref de mode test pour le B777
    // Exemple hypothétique: "777/test/display_test"
    
    // return Dataref::getInstance()->get<int>("777/test/display_test") == 1;           // DATAREF À TROUVER
    return false; // Par défaut, pas de mode test
    
} // !isTestMode


// ===============================================================================================================
// Méthode permettant de déterminer si Baro est en mode STD (à défaut d'avoir trouvé la dataref qui va bien)
bool FF777FCUEfisProfile::isBaroSTD(const char *side) {
    
    // reconstituer le 'const char' "1-sim/output/efis/capt/baro_value"
    //                           ou "1-sim/output/efis/fo/baro_value" selon le cas
    // avec insertion d'une variable remplaçant 'cpt' ou 'fo'
    char myDataref[100];
    const char* part1 = "1-sim/output/efis/";
    const char* part2 = "/baro_value";

    strcpy(myDataref, part1);       // recopie de part1                         "1-sim/output/efis/"
    strcat(myDataref, side);        // concaténation avec side (=capt p.e.)     "1-sim/output/efis/capt"
    strcat(myDataref, part2);       // concaténation avec part2                 "1-sim/output/efis/capt/baro_value"

    auto datarefManager = Dataref::getInstance();
    
    if (datarefManager->getCached<float>(myDataref) == 1013 ||
        datarefManager->getCached<float>(myDataref) *100 == 2992) {
        
        return true;
    }
    else return false;
    
} // !isBaroSTD


// ===============================================================================================================
// Méthode permettant de déterminer la valeur Baro en inHg (à défaut d'avoir trouvé la dataref qui va bien)
float FF777FCUEfisProfile::valBaroInHg(const char *datarefValBaro) {
    
    // Détermination de la valeur de pression atmo
    auto datarefManager = Dataref::getInstance();
    float valInHg = datarefManager->getCached<float>(datarefValBaro) ;
    
    // Si la valeur est sup à 450, c'est qu'elle est en hPa,
    // on la ramène alors en inHg
    if (valInHg > 450) valInHg = valInHg / 33.8639;
    
    // Troncature (un arrondi serait p.e. préférable) à 2 chiffres après la virgule
    valInHg = (int) (valInHg*100) / 100;
    
    return valInHg;
    
} // !valBaroInHg
