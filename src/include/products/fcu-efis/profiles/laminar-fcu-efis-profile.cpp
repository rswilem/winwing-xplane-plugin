#include "laminar-fcu-efis-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-fcu-efis.h"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <XPLMUtilities.h>

LaminarFCUEfisProfile::LaminarFCUEfisProfile(ProductFCUEfis *product) :
    FCUEfisAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("sim/cockpit2/electrical/instrument_brightness_ratio", [product](std::vector<float> brightness) {
        if (brightness.size() < 2) {
            return;
        }
        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/battery_on");

        uint8_t target = hasPower ? brightness[14] * 255.0f : 0;
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EXPED_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISR_FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISL_FLAG_GREEN, target);

        uint8_t screenBrightness = hasPower ? brightness[10] * 255.0f : 0;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);

        product->forceStateSync();
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/battery_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/instrument_brightness_ratio");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/autopilot/ap1_mode", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/autopilot/ap2_mode", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/autopilot/a_thr_mode", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, engaged ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/autopilot/loc_mode", [product](bool illuminated) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, illuminated ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/autopilot/appr_mode", [product](bool illuminated) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, illuminated ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("laminar/A333/annun/autopilot/alt_mode", [product](bool illuminated) {
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, illuminated ? 255 : 0);
    });

    // Monitor EFIS Right (Captain) LED states
    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/fo_flight_director_on", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, engaged ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/fo_ls_bars_on", [product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISR_LS_GREEN, on ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_fo_cstr", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_fo_fix", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_fo_vor", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_fo_ndb", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_NDB_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_fo_arpt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, show ? 255 : 0);
    });

    // Monitor EFIS Left (First Officer) LED states
    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/capt_flight_director_on", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, engaged ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/captain_ls_bars_on", [product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISL_LS_GREEN, on ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_capt_cstr", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_capt_fix", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_capt_vor", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_capt_ndb", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_NDB_GREEN, show ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/A333/annun/EFIS_capt_arpt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_ARPT_GREEN, show ? 255 : 0);
    });
}

LaminarFCUEfisProfile::~LaminarFCUEfisProfile() {
    Dataref::getInstance()->unbind("sim/cockpit2/electrical/instrument_brightness_ratio");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/battery_on");

    // Unbind FCU datarefs
    Dataref::getInstance()->unbind("laminar/A333/annun/autopilot/ap1_mode");
    Dataref::getInstance()->unbind("laminar/A333/annun/autopilot/ap2_mode");
    Dataref::getInstance()->unbind("laminar/A333/annun/autopilot/a_thr_mode");
    Dataref::getInstance()->unbind("laminar/A333/annun/autopilot/loc_mode");
    Dataref::getInstance()->unbind("laminar/A333/annun/autopilot/appr_mode");
    Dataref::getInstance()->unbind("laminar/A333/annun/autopilot/alt_mode");

    // Unbind EFIS Right datarefs
    Dataref::getInstance()->unbind("laminar/A333/annun/fo_flight_director_on");
    Dataref::getInstance()->unbind("laminar/A333/annun/fo_ls_bars_on");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_fo_cstr");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_fo_fix");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_fo_vor");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_fo_ndb");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_fo_arpt");

    // Unbind EFIS Left datarefs
    Dataref::getInstance()->unbind("laminar/A333/annun/capt_flight_director_on");
    Dataref::getInstance()->unbind("laminar/A333/annun/captain_ls_bars_on");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_capt_cstr");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_capt_fix");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_capt_vor");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_capt_ndb");
    Dataref::getInstance()->unbind("laminar/A333/annun/EFIS_capt_arpt");
}

bool LaminarFCUEfisProfile::IsEligible() {
    bool eligible = Dataref::getInstance()->exists("laminar/A333/ckpt_temp");
    return eligible;
}

const std::vector<std::string> &LaminarFCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // FCU display datarefs
        "sim/cockpit2/autopilot/airspeed_dial_kts_mach", // float
        "AirbusFBW/SPDmanaged",                          // int, 1 or 0
        "sim/cockpit2/autopilot/vnav_speed_window_open",                           // int, 1 or 0

        "sim/cockpit/autopilot/heading_mag", // float
        "AirbusFBW/HDGmanaged",              // int, 1 or 0
        "AirbusFBW/HDGdashed",               // int, 1 or 0

        "sim/cockpit/autopilot/altitude", // float
        "AirbusFBW/ALTmanaged",           // int, 1 or 0

        "sim/cockpit/autopilot/vertical_velocity", // float
        "sim/cockpit2/autopilot/speed_status",                      // int, 1 or 0

        "sim/cockpit/autopilot/airspeed_is_mach", // int, 1 or 0
        "sim/cockpit2/autopilot/trk_fpa",                   // HDG=VS,TRK=FPA, // int, 1 or 0

        // FCU button state datarefs for LED monitoring
        "AirbusFBW/AP1Engage", // int, 1 or 0
        "AirbusFBW/AP2Engage", // int, 1 or 0

        // EFIS barometric pressure datarefs
        "AirbusFBW/BaroStdCapt",                                         // int, 1 or 0
        "AirbusFBW/BaroUnitCapt",                                        // int, 1 for hPa, 0 for inHg
        "AirbusFBW/BaroStdFO",                                           // int, 1 or 0
        "AirbusFBW/BaroUnitFO",                                          // int, 1 for hPa, 0 for inHg
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot",   // float, inHg
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot", // float, inHg
    };

    return datarefs;
}

const std::vector<FCUEfisButtonDef> &LaminarFCUEfisProfile::buttonDefs() const {
    static const std::vector<FCUEfisButtonDef> buttons = {
        {0, "MACH", "sim/autopilot/knots_mach_toggle"},
        {1, "LOC", "AirbusFBW/LOCbutton"},
        {2, "TRK", "sim/autopilot/trkfpa"},
        {3, "AP1", "AirbusFBW/AP1Engage", FCUEfisDatarefType::TOGGLE_VALUE},
        {4, "AP2", "AirbusFBW/AP2Engage", FCUEfisDatarefType::TOGGLE_VALUE},
        {5, "A/THR", "AirbusFBW/ATHRbutton"},
        {6, "EXPED", "AirbusFBW/EXPEDbutton"},
        {7, "METRIC", "toliss_airbus/metric_alt_button_push"},
        {8, "APPR", "AirbusFBW/APPRbutton"},
        {9, "SPD DEC", "sim/autopilot/airspeed_down"},
        {10, "SPD INC", "sim/autopilot/airspeed_up"},
        {11, "SPD PUSH", "sim/autopilot/spd_intv"},
        {12, "SPD PULL", "sim/autopilot/spd_intv"},
        {13, "HDG DEC", "sim/autopilot/heading_down"},
        {14, "HDG INC", "sim/autopilot/heading_up"},
        {15, "HDG PUSH", "sim/autopilot/gpss"},
        {16, "HDG PULL", "sim/autopilot/heading"},
        {17, "ALT DEC", "sim/autopilot/altitude_down"},
        {18, "ALT INC", "sim/autopilot/altitude_up"},
        {19, "ALT PUSH", "sim/autopilot/FMS"},
        {20, "ALT PULL", "sim/autopilot/level_change"},
        {21, "VS DEC", "sim/autopilot/vertical_speed_down"},
        {22, "VS INC", "sim/autopilot/vertical_speed_up"},
        {23, "VS PUSH", ""},
        {24, "VS PULL", "sim/autopilot/alt_vs"},
        {25, "ALT 100", "", FCUEfisDatarefType::SET_VALUE, 0.0},  // Set to 0 for 100ft increments
        {26, "ALT 1000", "AirbusFBW/ALT100_1000", FCUEfisDatarefType::SET_VALUE, 1.0}, // Set to 1 for 1000ft increments

        // Brightness control buttons
        // These are handled internally via brightness callback system
        // {27, "BRIGHT", "AirbusFBW/SupplLightLevelRehostats[0]"},  // Panel brightness
        // {27, "BRIGHT_LCD", "AirbusFBW/SupplLightLevelRehostats[1]"},  // LCD brightness

        // LED control buttons for monitoring only (no direct button presses)
        // {28, "APPR_LED", "AirbusFBW/APPRilluminated"},  // Monitored via LED callback
        // {29, "ATHR_LED", "AirbusFBW/ATHRmode"},  // Monitored via LED callback
        // {30, "LOC_LED", "AirbusFBW/LOCilluminated"},  // Monitored via LED callback
        // Button 31 reserved

        // EFIS Right (FO) buttons (32-63)
        {32, "R_FD", "sim/autopilot/fdir2_command_bars_toggle"},
        {33, "R_LS", "laminar/A333/buttons/fo_ils_bars_push"},
        {34, "R_CSTR", "laminar/A333/buttons/fo_EFIS_CSTR"},
        {35, "R_WPT", "sim/instruments/EFIS_copilot_fix"},
        {36, "R_VOR.D", "sim/instruments/EFIS_copilot_vor"},
        {37, "R_NDB", "sim/instruments/EFIS_copilot_ndb"},
        {38, "R_ARPT", "sim/instruments/EFIS_copilot_apt"},
        {39, "R_STD PUSH", "laminar/A333/push/baro/fo_std"},
        {40, "R_STD PULL", "laminar/A333/pull/baro/fo_std"},
        {41, "R_PRESS DEC", "custom", FCUEfisDatarefType::BAROMETER_FO, -1.0},
        {42, "R_PRESS INC", "custom", FCUEfisDatarefType::BAROMETER_FO, 1.0},
        {43, "R_inHg", "laminar/A333/knob/baro/fo_hPa", FCUEfisDatarefType::SET_VALUE, 0.0},                        // Set to 0 for inHg
        {44, "R_hPa", "AirbusFBW/BaroUnitFO", FCUEfisDatarefType::SET_VALUE, 1.0},                         // Set to 1 for hPa
        {45, "R_MODE LS", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 0.0},                       // LS mode
        {46, "R_MODE VOR", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 1.0},                      // VOR mode
        {47, "R_MODE NAV", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 2.0},                      // NAV mode
        {48, "R_MODE ARC", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 3.0},                      // ARC mode
        {49, "R_MODE PLAN", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 4.0},                     // PLAN mode
        {50, "R_RANGE 10", "sim/cockpit2/EFIS/map_range_copilot", FCUEfisDatarefType::SET_VALUE, 0.0},                     // 10nm range
        {51, "R_RANGE 20", "sim/cockpit2/EFIS/map_range_copilot", FCUEfisDatarefType::SET_VALUE, 1.0},                     // 20nm range
        {52, "R_RANGE 40", "sim/cockpit2/EFIS/map_range_copilot", FCUEfisDatarefType::SET_VALUE, 2.0},                     // 40nm range
        {53, "R_RANGE 80", "sim/cockpit2/EFIS/map_range_copilot", FCUEfisDatarefType::SET_VALUE, 3.0},                     // 80nm range
        {54, "R_RANGE 160", "sim/cockpit2/EFIS/map_range_copilot", FCUEfisDatarefType::SET_VALUE, 4.0},                    // 160nm range
        {55, "R_RANGE 320", "sim/cockpit2/EFIS/map_range_copilot", FCUEfisDatarefType::SET_VALUE, 5.0},                    // 320nm range
        {56, "R_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0}, // VOR1
        {57, "R_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0}, // OFF1
        {58, "R_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0}, // ADF1
        {59, "R_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0}, // VOR2
        {60, "R_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0}, // OFF2
        {61, "R_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0}, // ADF2
        // Buttons 62-63 reserved

        // EFIS Left (Pilot) buttons (64-95)
        {64, "L_FD", "sim/autopilot/fdir_command_bars_toggle"},
        {65, "L_LS", "laminar/A333/buttons/capt_ils_bars_push"},
        {66, "L_CSTR", "laminar/A333/buttons/capt_EFIS_CSTR"},
        {67, "L_WPT", "sim/instruments/EFIS_fix"},
        {68, "L_VOR.D", "sim/instruments/EFIS_vor"},
        {69, "L_NDB", "sim/instruments/EFIS_ndb"},
        {70, "L_ARPT", "sim/instruments/EFIS_apt"},
        {71, "L_STD PUSH", "laminar/A333/push/baro/capt_std"},
        {72, "L_STD PULL", "laminar/A333/pull/baro/capt_std"},
        {73, "L_PRESS DEC", "custom", FCUEfisDatarefType::BAROMETER_PILOT, -1.0},
        {74, "L_PRESS INC", "custom", FCUEfisDatarefType::BAROMETER_PILOT, 1.0},
        {75, "L_inHg", "laminar/A333/barometer/capt_inHg_hPa_pos", FCUEfisDatarefType::SET_VALUE, 0.0},                    // laminar/A333/barometer/capt_inHg_hPa_pos Set to 0 for inHg
        {76, "L_hPa", "laminar/A333/knob/baro/capt_hPa", FCUEfisDatarefType::SET_VALUE, 1.0},                     // Set to 1 for hPa
        {77, "L_MODE LS", "laminar/A333/knobs/capt_EFIS_knob_right", FCUEfisDatarefType::SET_VALUE, 0.0},                   // LS mode
        {78, "L_MODE VOR", "laminar/A333/knobs/capt_EFIS_knob_left", FCUEfisDatarefType::SET_VALUE, 1.0},                  // VOR mode
        {79, "L_MODE NAV", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 2.0},                  // NAV mode
        {80, "L_MODE ARC", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 3.0},                  // ARC mode
        {81, "L_MODE PLAN", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 4.0},                 // PLAN mode
        {82, "L_RANGE 10", "sim/cockpit2/EFIS/map_range", FCUEfisDatarefType::SET_VALUE, 0.0},                 // 10nm range
        {83, "L_RANGE 20", "sim/cockpit2/EFIS/map_range", FCUEfisDatarefType::SET_VALUE, 1.0},                 // 20nm range
        {84, "L_RANGE 40", "sim/cockpit2/EFIS/map_range", FCUEfisDatarefType::SET_VALUE, 2.0},                 // 40nm range
        {85, "L_RANGE 80", "sim/cockpit2/EFIS/map_range", FCUEfisDatarefType::SET_VALUE, 3.0},                 // 80nm range
        {86, "L_RANGE 160", "sim/cockpit2/EFIS/map_range", FCUEfisDatarefType::SET_VALUE, 4.0},                // 160nm range
        {87, "L_RANGE 320", "sim/cockpit2/EFIS/map_range", FCUEfisDatarefType::SET_VALUE, 5.0},                // 320nm range
        {88, "L_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0}, // ADF1
        {89, "L_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0}, // OFF1
        {90, "L_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0}, // VOR1
        {91, "L_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0}, // ADF2
        {92, "L_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0}, // OFF2
        {93, "L_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0}, // VOR2
        // Buttons 94-95 reserved
    };
    return buttons;
}

void LaminarFCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    auto datarefManager = Dataref::getInstance();

    // Set managed mode indicators - using validated int datarefs (1 or 0)
    data.spdManaged = datarefManager->getCached<bool>("AirbusFBW/SPDmanaged");
    data.hdgManaged = datarefManager->getCached<bool>("AirbusFBW/HDGmanaged");
    data.altManaged = datarefManager->getCached<bool>("AirbusFBW/ALTmanaged");

    // Speed/Mach mode - using sim/cockpit/autopilot/airspeed_is_mach (int, 1 or 0)
    data.spdMach = datarefManager->getCached<bool>("sim/cockpit/autopilot/airspeed_is_mach");
    float speed = datarefManager->getCached<float>("sim/cockpit2/autopilot/airspeed_dial_kts_mach");

    if (speed > 0 && datarefManager->getCached<bool>("sim/cockpit2/autopilot/vnav_speed_window_open")) {
        std::stringstream ss;
        if (data.spdMach) {
            // In Mach mode, format as 0.XX -> "0XX" (e.g., 0.40 -> "040", 0.82 -> "082")
            int machHundredths = static_cast<int>(std::round(speed * 100));
            ss << std::setfill('0') << std::setw(3) << machHundredths;
        } else {
            // In speed mode, format as regular integer
            ss << std::setfill('0') << std::setw(3) << static_cast<int>(speed);
        }
        data.speed = ss.str();
    } else {
        data.speed = "---";
    }

    // Format FCU heading display - using sim/cockpit/autopilot/heading_mag (float)
    float heading = datarefManager->getCached<float>("sim/cockpit/autopilot/heading_mag");
    if (heading >= 0 && datarefManager->getCached<bool>("AirbusFBW/HDGdashed") == false) {
        // Convert 360 to 0 for display
        int hdgDisplay = static_cast<int>(heading) % 360;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << hdgDisplay;
        data.heading = ss.str();
    } else {
        data.heading = "---";
    }

    // Format FCU altitude display - using sim/cockpit/autopilot/altitude (float)
    float altitude = datarefManager->getCached<float>("sim/cockpit/autopilot/altitude");
    if (altitude >= 0) {
        int altInt = static_cast<int>(altitude);
        std::stringstream ss;
        // Always show full altitude value
        ss << std::setfill('0') << std::setw(5) << altInt;
        data.altitude = ss.str();
    } else {
        data.altitude = "-----";
    }

    // Format vertical speed display - using sim/cockpit/autopilot/vertical_velocity (float)
    float vs = datarefManager->getCached<float>("sim/cockpit/autopilot/vertical_velocity");
    bool vsDashed = datarefManager->getCached<int>("sim/cockpit2/autopilot/speed_status") == 2;

    // HDG/TRK mode
    data.hdgTrk = datarefManager->getCached<bool>("sim/cockpit2/autopilot/trk_fpa");
    data.vsMode = !data.hdgTrk; // VS mode when HDG mode
    data.fpaMode = data.hdgTrk; // FPA mode when TRK mode

    if (vsDashed) {
        // When dashed, show 5 dashes with minus sign
        data.verticalSpeed = "-----";
        data.vsSign = false;          // Show minus sign for dashes
        data.fpaComma = data.fpaMode; // Show decimal point only in FPA mode
    } else if (data.fpaMode) {
        // In FPA mode (TRK mode), display format X.Y
        // Dataref value 600 corresponds to indication +0.6
        // Convert dataref value to FPA display: divide by 1000
        float fpa = vs / 1000.0f; // 600 -> 0.6, -100 -> -0.1

        float absFpa = std::abs(fpa);

        // Format FPA with only significant digits
        // 0.0 should be "00  " to display as "0.0"
        // 0.6 should be "06  " to display as "0.6"
        // 1.2 should be "12  " to display as "1.2"
        // 2.5 should be "25  " to display as "2.5"

        int fpaTenths = static_cast<int>(std::round(absFpa * 10)); // 0.0->0, 0.6->6, 1.2->12, 2.5->25

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << fpaTenths << "  "; // 2 digits + 2 spaces

        data.verticalSpeed = ss.str();

        data.fpaComma = true;     // Enable decimal point display
        data.vsSign = (fpa >= 0); // Control sign display for FPA
    } else {
        // Normal VS mode: Format with proper padding to 4 digits (no sign in string)
        std::stringstream ss;
        int vsInt = static_cast<int>(std::round(vs));
        int absVs = std::abs(vsInt);

        // If VS is a multiple of 100, show last two digits as "##"
        if (absVs % 100 == 0) {
            ss << std::setfill('0') << std::setw(2) << (absVs / 100) << "##";
        } else {
            // Show full value for non-multiples of 100
            ss << std::setfill('0') << std::setw(4) << absVs;
        }
        data.verticalSpeed = ss.str();

        data.vsSign = (vs >= 0); // Control sign display for VS
        data.fpaComma = false;   // No decimal point in VS mode
    }

    // Set VS/FPA indication flags based on current mode
    data.vsIndication = data.vsMode;   // fvs flag - show VS indication when in VS mode
    data.fpaIndication = data.fpaMode; // ffpa2 flag - show FPA indication when in FPA mode

    // VS vertical line
    // Only show vertical line in VS mode when not dashed
    data.vsVerticalLine = data.vsMode && (data.verticalSpeed != "-----");

    // LAT mode - Typically always on for Airbus
    data.latMode = true;

    for (int i = 0; i < 2; i++) {
        bool isCaptain = i == 0;

        bool isStd = datarefManager->getCached<bool>(isCaptain ? "AirbusFBW/BaroStdCapt" : "AirbusFBW/BaroStdFO");
        bool isBaroHpa = datarefManager->getCached<bool>(isCaptain ? "AirbusFBW/BaroUnitCapt" : "AirbusFBW/BaroUnitFO");
        float baroValue = datarefManager->getCached<float>(isCaptain ? "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot" : "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot");

        EfisDisplayValue value = {
            .baro = "",
            .isStd = isStd,
            .unitIsInHg = false};

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

void LaminarFCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();

    if (phase == xplm_CommandBegin && (button->datarefType == FCUEfisDatarefType::BAROMETER_PILOT || button->datarefType == FCUEfisDatarefType::BAROMETER_FO)) {
        bool isCaptain = button->datarefType == FCUEfisDatarefType::BAROMETER_PILOT;
        bool isStd = datarefManager->getCached<bool>(isCaptain ? "AirbusFBW/BaroStdCapt" : "AirbusFBW/BaroStdFO");
        if (isStd) {
            return;
        }

        bool isBaroHpa = datarefManager->getCached<bool>(isCaptain ? "AirbusFBW/BaroUnitCapt" : "AirbusFBW/BaroUnitFO");
        const char *datarefName = isCaptain ? "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot" : "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot";
        float baroValue = datarefManager->getCached<float>(datarefName);
        bool increase = button->value > 0;

        if (isBaroHpa) {
            float hpaValue = baroValue * 33.8639f;
            hpaValue += increase ? 1.0f : -1.0f;
            baroValue = hpaValue / 33.8639f;
        } else {
            baroValue += increase ? 0.01f : -0.01f;
        }

        datarefManager->set<float>(datarefName, baroValue);
    } else if (phase == xplm_CommandBegin && (button->datarefType == FCUEfisDatarefType::SET_VALUE || button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE)) {
        bool wantsToggle = button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE;

        if (wantsToggle) {
            int currentValue = datarefManager->get<int>(button->dataref.c_str());
            int newValue = currentValue ? 0 : 1;
            datarefManager->set<int>(button->dataref.c_str(), newValue);
        } else {
            datarefManager->set<float>(button->dataref.c_str(), button->value);
        }

        return;
    } else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    }
}
