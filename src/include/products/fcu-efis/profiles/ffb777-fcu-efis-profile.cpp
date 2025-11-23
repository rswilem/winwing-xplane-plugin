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


FF777FCUEfisProfile::FF777FCUEfisProfile(ProductFCUEfis *product) :
    FCUEfisAircraftProfile(product) {
    
    // ========================================================================
    // SECTION 1: MONITORING DE L'ÉCLAIRAGE ET ALIMENTATION
    // ========================================================================
    
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

    // ========================================================================
    // SECTION 2: MONITORING DES LEDs DU MCP (MODE CONTROL PANEL)
    // ========================================================================
    
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

    // ========================================================================
    // SECTION 3: MONITORING DES LEDs EFIS CAPTAIN (Gauche)
    // ========================================================================
    
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

    // ========================================================================
    // SECTION 4: MONITORING DES LEDs EFIS FIRST OFFICER (Droite)
    // ========================================================================
    
    // Flight Director First Officer
    Dataref::getInstance()->monitorExistingDataref<int>("777/displays/fd_right_on", [this, product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    // TODO: Mapping des autres boutons EFIS FO selon datarefs B777
}

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

bool FF777FCUEfisProfile::IsEligible() {
    // Détection du B777: chercher une dataref spécifique au FlightFactor 777
    // Exemples possibles: "T7Avionics/version" ou "777/fmc/status"
    bool eligible = Dataref::getInstance()->exists("T7Avionics/mcp_power") ||
                    Dataref::getInstance()->exists("777/autopilot/ap_left_engaged");
    return eligible;
}

const std::vector<std::string> &FF777FCUEfisProfile::displayDatarefs() const {
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

const std::unordered_map<uint16_t, FCUEfisButtonDef> &FF777FCUEfisProfile::buttonDefs() const {
    // IMPORTANT: Ce mapping est hypothétique et doit être adapté selon:
    // 1. La disposition physique de votre matériel
    // 2. Les commandes réelles du FlightFactor B777v2
    //
    // Pour trouver les bonnes commandes, utilisez DataRefTool dans X-Plane
    
    static const std::unordered_map<uint16_t, FCUEfisButtonDef> buttons = {
        // ====================================================================
        // MCP - MODE CONTROL PANEL (Centre)
        // ====================================================================
        
        {0, {"SPD", "777/MCP/spd_sel_button"}},
        {1, {"LNAV", "777/MCP/lnav_button"}},
        {2, {"VNAV", "777/MCP/vnav_button"}},
        {3, {"CMD L", "777/MCP/cmd_left_button"}},
        {4, {"CMD R", "777/MCP/cmd_right_button"}},
        {5, {"A/THR", "777/MCP/autothrottle_button"}},
        {6, {"FLCH", "777/MCP/flch_button"}},
        {7, {"HDG HOLD", "777/MCP/hdg_hold_button"}},
        {8, {"APP", "777/MCP/app_button"}},
        {9, {"ALT HOLD", "777/MCP/alt_hold_button"}},
        {10, {"LOC", "777/MCP/loc_button"}},
        {11, {"V/S", "777/MCP/vs_button"}},
        
        // Rotary encoders - Speed
        {12, {"SPD DEC", "sim/autopilot/airspeed_down"}},
        {13, {"SPD INC", "sim/autopilot/airspeed_up"}},
        {14, {"CO", "777/MCP/co_button"}}, // C/O = Change Over
        
        // Rotary encoders - Heading
        {15, {"HDG DEC", "sim/autopilot/heading_down"}},
        {16, {"HDG INC", "sim/autopilot/heading_up"}},
        
        // Rotary encoders - Altitude
        {17, {"ALT DEC", "sim/autopilot/altitude_down"}},
        {18, {"ALT INC", "sim/autopilot/altitude_up"}},
        
        // Rotary encoders - Vertical Speed
        {19, {"VS DEC", "sim/autopilot/vertical_speed_down"}},
        {20, {"VS INC", "sim/autopilot/vertical_speed_up"}},
        
        // Altitude increment selector
        {21, {"ALT 100", "777/MCP/alt_increment", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {22, {"ALT 1000", "777/MCP/alt_increment", FCUEfisDatarefType::SET_VALUE, 1.0}},
        
        // Buttons 23-31 reserved for future MCP expansion
        
        // ====================================================================
        // EFIS CAPTAIN (Gauche) - Buttons 32-63
        // ====================================================================
        
        {32, {"L_FD", "777/displays/captain_fd_toggle"}},
        {33, {"L_MINIMUMS", "777/displays/captain_minimums_toggle"}},
        {34, {"L_BARO PUSH", "777/displays/captain_baro_push"}},
        {35, {"L_BARO PULL", "777/displays/captain_baro_pull"}},
        {36, {"L_BARO DEC", "custom", FCUEfisDatarefType::BAROMETER_PILOT, -1.0}},
        {37, {"L_BARO INC", "custom", FCUEfisDatarefType::BAROMETER_PILOT, 1.0}},
        {38, {"L_inHg", "777/displays/captain_baro_unit", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {39, {"L_hPa", "777/displays/captain_baro_unit", FCUEfisDatarefType::SET_VALUE, 1.0}},
        
        // ND Mode selector
        {40, {"L_MODE APP", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {41, {"L_MODE VOR", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {42, {"L_MODE MAP", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {43, {"L_MODE PLAN", "777/displays/captain_nd_mode", FCUEfisDatarefType::SET_VALUE, 3.0}},
        
        // ND Range selector
        {44, {"L_RANGE 5", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {45, {"L_RANGE 10", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {46, {"L_RANGE 20", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {47, {"L_RANGE 40", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 3.0}},
        {48, {"L_RANGE 80", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 4.0}},
        {49, {"L_RANGE 160", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 5.0}},
        {50, {"L_RANGE 320", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 6.0}},
        {51, {"L_RANGE 640", "777/displays/captain_nd_range", FCUEfisDatarefType::SET_VALUE, 7.0}},
        
        // ND Options
        {52, {"L_WPT", "777/displays/captain_show_wpt"}},
        {53, {"L_STA", "777/displays/captain_show_sta"}},
        {54, {"L_ARPT", "777/displays/captain_show_arpt"}},
        {55, {"L_DATA", "777/displays/captain_show_data"}},
        {56, {"L_POS", "777/displays/captain_show_pos"}},
        {57, {"L_TERR", "777/displays/captain_show_terrain"}},
        
        // VOR/ADF selectors
        {58, {"L_VOR1 L", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {59, {"L_VOR1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {60, {"L_VOR1 R", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {61, {"L_VOR2 L", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {62, {"L_VOR2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {63, {"L_VOR2 R", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0}},
        
        // ====================================================================
        // EFIS FIRST OFFICER (Droite) - Buttons 64-95
        // ====================================================================
        
        {64, {"R_FD", "777/displays/fo_fd_toggle"}},
        {65, {"R_MINIMUMS", "777/displays/fo_minimums_toggle"}},
        {66, {"R_BARO PUSH", "777/displays/fo_baro_push"}},
        {67, {"R_BARO PULL", "777/displays/fo_baro_pull"}},
        {68, {"R_BARO DEC", "custom", FCUEfisDatarefType::BAROMETER_FO, -1.0}},
        {69, {"R_BARO INC", "custom", FCUEfisDatarefType::BAROMETER_FO, 1.0}},
        {70, {"R_inHg", "777/displays/fo_baro_unit", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {71, {"R_hPa", "777/displays/fo_baro_unit", FCUEfisDatarefType::SET_VALUE, 1.0}},
        
        // ND Mode selector
        {72, {"R_MODE APP", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {73, {"R_MODE VOR", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {74, {"R_MODE MAP", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {75, {"R_MODE PLAN", "777/displays/fo_nd_mode", FCUEfisDatarefType::SET_VALUE, 3.0}},
        
        // ND Range selector
        {76, {"R_RANGE 5", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {77, {"R_RANGE 10", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {78, {"R_RANGE 20", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {79, {"R_RANGE 40", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 3.0}},
        {80, {"R_RANGE 80", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 4.0}},
        {81, {"R_RANGE 160", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 5.0}},
        {82, {"R_RANGE 320", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 6.0}},
        {83, {"R_RANGE 640", "777/displays/fo_nd_range", FCUEfisDatarefType::SET_VALUE, 7.0}},
        
        // ND Options
        {84, {"R_WPT", "777/displays/fo_show_wpt"}},
        {85, {"R_STA", "777/displays/fo_show_sta"}},
        {86, {"R_ARPT", "777/displays/fo_show_arpt"}},
        {87, {"R_DATA", "777/displays/fo_show_data"}},
        {88, {"R_POS", "777/displays/fo_show_pos"}},
        {89, {"R_TERR", "777/displays/fo_show_terrain"}},
        
        // VOR/ADF selectors (inversés par rapport à Captain)
        {90, {"R_VOR1 R", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {91, {"R_VOR1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {92, {"R_VOR1 L", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0}},
        {93, {"R_VOR2 R", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0}},
        {94, {"R_VOR2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0}},
        {95, {"R_VOR2 L", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0}},
    };
    return buttons;
}

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

    // ========================================================================
    // HEADING DISPLAY
    // ========================================================================
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

    // ========================================================================
    // ALTITUDE DISPLAY
    // ========================================================================
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

    // ========================================================================
    // VERTICAL SPEED DISPLAY
    // ========================================================================
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

    // ========================================================================
    // EFIS BAROMETER DISPLAYS (Captain & First Officer)
    // ========================================================================
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

void FF777FCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();

    // ========================================================================
    // GESTION DES BAROMETERS (Captain & FO)
    // ========================================================================
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
    
    // ========================================================================
    // GESTION DES BOUTONS SET_VALUE
    // ========================================================================
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::SET_VALUE) {
        datarefManager->set<float>(button->dataref.c_str(), button->value);
    }
    
    // ========================================================================
    // GESTION DES BOUTONS TOGGLE
    // ========================================================================
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE) {
        int currentValue = datarefManager->get<int>(button->dataref.c_str());
        int newValue = currentValue ? 0 : 1;
        datarefManager->set<int>(button->dataref.c_str(), newValue);
    }
    
    // ========================================================================
    // GESTION DES COMMANDES X-PLANE
    // ========================================================================
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    }
}

bool FF777FCUEfisProfile::isTestMode() {
    // TODO: Identifier la dataref de mode test pour le B777
    // Exemple hypothétique: "777/test/display_test"
    // return Dataref::getInstance()->get<int>("777/test/display_test") == 1;
    return false; // Par défaut, pas de mode test
}
