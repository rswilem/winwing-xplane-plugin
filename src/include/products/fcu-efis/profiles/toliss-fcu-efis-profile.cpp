#include "toliss-fcu-efis-profile.h"
#include "product-fcu-efis.h"
#include "dataref.h"
#include "appstate.h"
#include <XPLMUtilities.h>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <bitset>
#include <cmath>

TolissFCUEfisProfile::TolissFCUEfisProfile(ProductFCUEfis *product) : FCUEfisAircraftProfile(product) {
    
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/SupplLightLevelRehostats", [product](std::vector<float> brightness) {
        if (brightness.size() < 2) {
            return;
        }
        
        uint8_t target = brightness[0] * 255.0f;
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EXPED_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISR_FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISL_FLAG_GREEN, target);
        
        uint8_t screenBrightness = brightness[1] * 255.0f;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);
    });
    
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/AP1Engage", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/AP2Engage", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged ? 255 : 0);
    });
    
    // Force initial LED state sync since monitorExistingDataref only triggers on changes
    if (Dataref::getInstance()->exists("AirbusFBW/AP1Engage")) {
        bool ap1Value = Dataref::getInstance()->get<bool>("AirbusFBW/AP1Engage");
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, ap1Value ? 255 : 0);
    }
    if (Dataref::getInstance()->exists("AirbusFBW/AP2Engage")) {
        bool ap2Value = Dataref::getInstance()->get<bool>("AirbusFBW/AP2Engage");
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, ap2Value ? 255 : 0);
    }
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ATHRmode", [product](int mode) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, mode > 0 ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/LOCilluminated", [product](bool illuminated) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, illuminated ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/APPRilluminated", [product](bool illuminated) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, illuminated ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/APVerticalMode", [product](int vsMode) {
        bool expedEnabled = vsMode >= 0 && vsMode & 0b00010000;
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, expedEnabled ? 255 : 0);
    });
    
    // Monitor EFIS Right (Captain) LED states
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/FD2Engage", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/ILSonFO", [product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISR_LS_GREEN, on ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowCSTRFO", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowWPTFO", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowVORDFO", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowNDBFO", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_NDB_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowARPTFO", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, show ? 255 : 0);
    });
    
    // Monitor EFIS Left (First Officer) LED states
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/FD1Engage", [product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/ILSonCapt", [product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISL_LS_GREEN, on ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowCSTRCapt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowWPTCapt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowVORDCapt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowNDBCapt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_NDB_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/NDShowARPTCapt", [product](bool show) {
        product->setLedBrightness(FCUEfisLed::EFISL_ARPT_GREEN, show ? 255 : 0);
    });
}

TolissFCUEfisProfile::~TolissFCUEfisProfile() {
    // Unbind brightness control datarefs
    Dataref::getInstance()->unbind("AirbusFBW/SupplLightLevelRehostats");
    
    // Unbind FCU datarefs
    Dataref::getInstance()->unbind("AirbusFBW/AP1Engage");
    Dataref::getInstance()->unbind("AirbusFBW/AP2Engage");
    Dataref::getInstance()->unbind("AirbusFBW/ATHRmode");
    Dataref::getInstance()->unbind("AirbusFBW/LOCilluminated");
    Dataref::getInstance()->unbind("AirbusFBW/APPRilluminated");
    Dataref::getInstance()->unbind("AirbusFBW/APVerticalMode");
    
    // Unbind EFIS Right datarefs
    Dataref::getInstance()->unbind("AirbusFBW/FD2Engage");
    Dataref::getInstance()->unbind("AirbusFBW/ILSonFO");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowCSTRFO");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowWPTFO");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowVORDFO");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowNDBFO");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowARPTFO");
    
    // Unbind EFIS Left datarefs
    Dataref::getInstance()->unbind("AirbusFBW/FD1Engage");
    Dataref::getInstance()->unbind("AirbusFBW/ILSonCapt");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowCSTRCapt");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowWPTCapt");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowVORDCapt");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowNDBCapt");
    Dataref::getInstance()->unbind("AirbusFBW/NDShowARPTCapt");
}

bool TolissFCUEfisProfile::IsEligible() {
    bool eligible = Dataref::getInstance()->exists("AirbusFBW/FCUAvail");
    return eligible;
}

const std::vector<std::string>& TolissFCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // FCU display datarefs
        "sim/cockpit2/autopilot/airspeed_dial_kts_mach", // float
        "AirbusFBW/SPDmanaged", // int, 1 or 0
        "AirbusFBW/SPDdashed", // int, 1 or 0

        "sim/cockpit/autopilot/heading_mag", // float
        "AirbusFBW/HDGmanaged", // int, 1 or 0
        "AirbusFBW/HDGdashed", // int, 1 or 0

        "sim/cockpit/autopilot/altitude", // float
        "AirbusFBW/ALTmanaged", // int, 1 or 0

        "sim/cockpit/autopilot/vertical_velocity", // float
        "AirbusFBW/VSdashed", // int, 1 or 0
        
        "sim/cockpit/autopilot/airspeed_is_mach", // int, 1 or 0
        "AirbusFBW/HDGTRKmode", // HDG=VS,TRK=FPA, // int, 1 or 0
        
        // FCU button state datarefs for LED monitoring
        "AirbusFBW/AP1Engage", // int, 1 or 0
        "AirbusFBW/AP2Engage", // int, 1 or 0
        
        // EFIS barometric pressure datarefs
        "AirbusFBW/BaroStdCapt", // int, 1 or 0
        "AirbusFBW/BaroUnitCapt", // int, 1 for hPa, 0 for inHg
        "AirbusFBW/BaroStdFO", // int, 1 or 0
        "AirbusFBW/BaroUnitFO", // int, 1 for hPa, 0 for inHg
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", // float, inHg
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot", // float, inHg
    };

    return datarefs;
}

const std::vector<FCUEfisButtonDef>& TolissFCUEfisProfile::buttonDefs() const {
    static const std::vector<FCUEfisButtonDef> buttons = {
        {0, "MACH", "toliss_airbus/ias_mach_button_push"},
        {1, "LOC", "AirbusFBW/LOCbutton"},
        {2, "TRK", "toliss_airbus/hdgtrk_button_push"},
        {3, "AP1", "AirbusFBW/AP1Engage", FCUEfisDatarefType::TOGGLE_VALUE},
        {4, "AP2", "AirbusFBW/AP2Engage", FCUEfisDatarefType::TOGGLE_VALUE},
        {5, "A/THR", "AirbusFBW/ATHRbutton"},
        {6, "EXPED", "AirbusFBW/EXPEDbutton"},
        {7, "METRIC", "toliss_airbus/metric_alt_button_push"},
        {8, "APPR", "AirbusFBW/APPRbutton"},
        {9, "SPD DEC", "sim/autopilot/airspeed_down"},
        {10, "SPD INC", "sim/autopilot/airspeed_up"},
        {11, "SPD PUSH", "AirbusFBW/PushSPDSel"},
        {12, "SPD PULL", "AirbusFBW/PullSPDSel"},
        {13, "HDG DEC", "sim/autopilot/heading_down"},
        {14, "HDG INC", "sim/autopilot/heading_up"},
        {15, "HDG PUSH", "AirbusFBW/PushHDGSel"},
        {16, "HDG PULL", "AirbusFBW/PullHDGSel"},
        {17, "ALT DEC", "sim/autopilot/altitude_down"},
        {18, "ALT INC", "sim/autopilot/altitude_up"},
        {19, "ALT PUSH", "AirbusFBW/PushAltitude"},
        {20, "ALT PULL", "AirbusFBW/PullAltitude"},
        {21, "VS DEC", "sim/autopilot/vertical_speed_down"},
        {22, "VS INC", "sim/autopilot/vertical_speed_up"},
        {23, "VS PUSH", "AirbusFBW/PushVSSel"},
        {24, "VS PULL", "AirbusFBW/PullVSSel"},
        {25, "ALT 100", "AirbusFBW/ALT100_1000", FCUEfisDatarefType::SET_VALUE, 0.0},        // Set to 0 for 100ft increments
        {26, "ALT 1000", "AirbusFBW/ALT100_1000", FCUEfisDatarefType::SET_VALUE, 1.0},       // Set to 1 for 1000ft increments
        
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
        {32, "R_FD", "toliss_airbus/fd2_push"},
        {33, "R_LS", "toliss_airbus/dispcommands/CoLSButtonPush"},
        {34, "R_CSTR", "toliss_airbus/dispcommands/CoCstrPushButton"},
        {35, "R_WPT", "toliss_airbus/dispcommands/CoWptPushButton"},
        {36, "R_VOR.D", "toliss_airbus/dispcommands/CoVorDPushButton"},
        {37, "R_NDB", "toliss_airbus/dispcommands/CoNdbPushButton"},
        {38, "R_ARPT", "toliss_airbus/dispcommands/CoArptPushButton"},
        {39, "R_STD PUSH", "toliss_airbus/copilot_baro_push"},
        {40, "R_STD PULL", "toliss_airbus/copilot_baro_pull"},
        {41, "R_PRESS DEC", "sim/instruments/barometer_copilot_down"},
        {42, "R_PRESS INC", "sim/instruments/barometer_copilot_up"},
        {43, "R_inHg", "AirbusFBW/BaroUnitFO", FCUEfisDatarefType::SET_VALUE, 0.0},          // Set to 0 for inHg
        {44, "R_hPa", "AirbusFBW/BaroUnitFO", FCUEfisDatarefType::SET_VALUE, 1.0},           // Set to 1 for hPa
        {45, "R_MODE LS", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 0.0},         // LS mode
        {46, "R_MODE VOR", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 1.0},        // VOR mode
        {47, "R_MODE NAV", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 2.0},        // NAV mode
        {48, "R_MODE ARC", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 3.0},        // ARC mode
        {49, "R_MODE PLAN", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::SET_VALUE, 4.0},       // PLAN mode
        {50, "R_RANGE 10", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::SET_VALUE, 0.0},       // 10nm range
        {51, "R_RANGE 20", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::SET_VALUE, 1.0},       // 20nm range
        {52, "R_RANGE 40", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::SET_VALUE, 2.0},       // 40nm range
        {53, "R_RANGE 80", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::SET_VALUE, 3.0},       // 80nm range
        {54, "R_RANGE 160", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::SET_VALUE, 4.0},      // 160nm range
        {55, "R_RANGE 320", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::SET_VALUE, 5.0},      // 320nm range
        {56, "R_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0},    // VOR1
        {57, "R_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0},    // OFF1
        {58, "R_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0},    // ADF1
        {59, "R_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 2.0},    // VOR2
        {60, "R_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 1.0},    // OFF2
        {61, "R_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::SET_VALUE, 0.0},    // ADF2
        // Buttons 62-63 reserved
        
        // EFIS Left (Pilot) buttons (64-95)
        {64, "L_FD", "toliss_airbus/fd1_push"},
        {65, "L_LS", "toliss_airbus/dispcommands/CaptLSButtonPush"},
        {66, "L_CSTR", "toliss_airbus/dispcommands/CaptCstrPushButton"},
        {67, "L_WPT", "toliss_airbus/dispcommands/CaptWptPushButton"},
        {68, "L_VOR.D", "toliss_airbus/dispcommands/CaptVorDPushButton"},
        {69, "L_NDB", "toliss_airbus/dispcommands/CaptNdbPushButton"},
        {70, "L_ARPT", "toliss_airbus/dispcommands/CaptArptPushButton"},
        {71, "L_STD PUSH", "toliss_airbus/capt_baro_push"},
        {72, "L_STD PULL", "toliss_airbus/capt_baro_pull"},
        {73, "L_PRESS DEC", "sim/instruments/barometer_down"},
        {74, "L_PRESS INC", "sim/instruments/barometer_up"},
        {75, "L_inHg", "AirbusFBW/BaroUnitCapt", FCUEfisDatarefType::SET_VALUE, 0.0},        // Set to 0 for inHg
        {76, "L_hPa", "AirbusFBW/BaroUnitCapt", FCUEfisDatarefType::SET_VALUE, 1.0},         // Set to 1 for hPa
        {77, "L_MODE LS", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 0.0},       // LS mode
        {78, "L_MODE VOR", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 1.0},      // VOR mode
        {79, "L_MODE NAV", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 2.0},      // NAV mode
        {80, "L_MODE ARC", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 3.0},      // ARC mode
        {81, "L_MODE PLAN", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::SET_VALUE, 4.0},     // PLAN mode
        {82, "L_RANGE 10", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::SET_VALUE, 0.0},     // 10nm range
        {83, "L_RANGE 20", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::SET_VALUE, 1.0},     // 20nm range
        {84, "L_RANGE 40", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::SET_VALUE, 2.0},     // 40nm range
        {85, "L_RANGE 80", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::SET_VALUE, 3.0},     // 80nm range
        {86, "L_RANGE 160", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::SET_VALUE, 4.0},    // 160nm range
        {87, "L_RANGE 320", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::SET_VALUE, 5.0},    // 320nm range
        {88, "L_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0},      // ADF1
        {89, "L_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0},      // OFF1
        {90, "L_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0},      // VOR1
        {91, "L_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 0.0},      // ADF2
        {92, "L_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 1.0},      // OFF2
        {93, "L_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::SET_VALUE, 2.0},      // VOR2
        // Buttons 94-95 reserved
    };
    return buttons;
}

void TolissFCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    auto datarefManager = Dataref::getInstance();
  
    // Set managed mode indicators - using validated int datarefs (1 or 0)
    data.spdManaged = datarefManager->getCached<bool>("AirbusFBW/SPDmanaged");
    data.hdgManaged = datarefManager->getCached<bool>("AirbusFBW/HDGmanaged");
    data.altManaged = datarefManager->getCached<bool>("AirbusFBW/ALTmanaged");
  
    // Speed/Mach mode - using sim/cockpit/autopilot/airspeed_is_mach (int, 1 or 0)
    data.spdMach = datarefManager->getCached<bool>("sim/cockpit/autopilot/airspeed_is_mach");
    
    // Format FCU speed display - using sim/cockpit2/autopilot/airspeed_dial_kts_mach (float)
    float speed = datarefManager->getCached<float>("sim/cockpit2/autopilot/airspeed_dial_kts_mach");
    bool isMach = datarefManager->getCached<bool>("sim/cockpit/autopilot/airspeed_is_mach");
    
    if (speed > 0 && datarefManager->getCached<bool>("AirbusFBW/SPDdashed") == false) {
        std::stringstream ss;
        if (isMach) {
            // In Mach mode, format as 0.XX -> "0XX" (e.g., 0.40 -> "040", 0.82 -> "082")
            int machHundredths = static_cast<int>(std::round(speed * 100));
            ss << std::setfill('0') << std::setw(3) << machHundredths;
            data.machComma = true; // Enable decimal point after first digit
        } else {
            // In speed mode, format as regular integer
            ss << std::setfill('0') << std::setw(3) << static_cast<int>(speed);
            data.machComma = false;
        }
        data.speed = ss.str();
    } else {
        data.speed = "---";
        data.machComma = false;
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
    bool vsDashed = datarefManager->getCached<bool>("AirbusFBW/VSdashed");
  
    // HDG/TRK mode - using AirbusFBW/HDGTRKmode (int, HDG=0, TRK=1)
    data.hdgTrk = datarefManager->getCached<bool>("AirbusFBW/HDGTRKmode");
    data.vsMode = !data.hdgTrk; // VS mode when HDG mode
    data.fpaMode = data.hdgTrk; // FPA mode when TRK mode
    
    if (vsDashed == 1) {
        // When dashed, show 5 dashes with minus sign
        data.verticalSpeed = "-----";
        data.vsSign = false; // Show minus sign for dashes
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
        
        data.fpaComma = true; // Enable decimal point display
        data.vsSign = (fpa >= 0); // Control sign display for FPA
    } else if (vs != 0) {
        // Normal VS mode: Format with proper padding to 4 digits (no sign in string)
        std::stringstream ss;
        int vsInt = static_cast<int>(std::round(vs));
        int absVs = std::abs(vsInt);
        
        // If VS is a multiple of 100, show last two digits as "##"
        if (absVs % 100 == 0 && absVs >= 100) {
            ss << std::setfill('0') << std::setw(2) << (absVs / 100) << "##";
        } else {
            // Show full value for non-multiples of 100
            ss << std::setfill('0') << std::setw(4) << absVs;
        }
        data.verticalSpeed = ss.str();
        
        data.vsSign = (vs >= 0); // Control sign display for VS
        data.fpaComma = false; // No decimal point in VS mode
    } else {
        // When VS is exactly 0, show 5 dashes
        data.verticalSpeed = "-----";
        data.vsSign = false; // Show minus sign for dashes
        data.fpaComma = data.fpaMode; // Show decimal point in FPA mode, not in VS mode
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
            .baro = "STD ", // Four characters
            .unitIsInHg = false
        };
        
        if (!isStd && baroValue > 0) {
            value.setBaro(baroValue, !isBaroHpa);
        }
        
        if (isCaptain) {
            data.efisLeft = value;
        }
        else {
            data.efisRight = value;
        }
    }
}

void TolissFCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }
    
    if (phase == xplm_CommandBegin && (button->datarefType == FCUEfisDatarefType::SET_VALUE || button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE)) {
        bool wantsToggle = button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE;
        
        if (wantsToggle) {
            int currentValue = Dataref::getInstance()->get<int>(button->dataref.c_str());
            int newValue = currentValue ? 0 : 1;
            Dataref::getInstance()->set<int>(button->dataref.c_str(), newValue);
        }
        else {
            Dataref::getInstance()->set<float>(button->dataref.c_str(), button->value);
        }
        return;
    }
    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::EXECUTE_CMD_ONCE) {
        Dataref::getInstance()->executeCommand(button->dataref.c_str());
    }
}
