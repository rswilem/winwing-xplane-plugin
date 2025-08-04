#include "toliss-fcu-efis-profile.h"
#include "product-fcu-efis.h"
#include "dataref.h"
#include <XPLMUtilities.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

TolissFCUEfisProfile::TolissFCUEfisProfile(ProductFCUEfis *product) : FCUEfisAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/SupplLightLevelRehostats", [product](std::vector<float> brightness) {
        uint8_t target = brightness[0] * 255.0f;
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISR_FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISL_FLAG_GREEN, target);
        
        uint8_t screenBrightness = brightness[1] * 255.0f;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AP1Engage", [product](int engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AP2Engage", [product](int engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ATHRmode", [product](int mode) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, mode > 0 ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/LOCilluminated", [product](int illuminated) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, illuminated ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/APPRilluminated", [product](int illuminated) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, illuminated ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/APVerticalMode", [product](int vsMode) {
        // TODO: Verify: is yellow the backlight?
        product->setLedBrightness(FCUEfisLed::EXPED_YELLOW, vsMode >= 112 ? 255 : 0);

        bool expedEnabled = vsMode & 0b00010000;
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, expedEnabled ? 255 : 0);
    });
    
    // Monitor EFIS Right (Captain) LED states
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/FD2Engage", [product](int engaged) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ILSonFO", [product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISR_LS_GREEN, on ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowCSTRFO", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowWPTFO", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowVORDFO", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowNDBFO", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISR_NDB_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowARPTFO", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, show ? 255 : 0);
    });
    
    // Monitor EFIS Left (First Officer) LED states
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/FD1Engage", [product](int engaged) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ILSonCapt", [product](int on) {
        product->setLedBrightness(FCUEfisLed::EFISL_LS_GREEN, on ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowCSTRCapt", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowWPTCapt", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowVORDCapt", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowNDBCapt", [product](int show) {
        product->setLedBrightness(FCUEfisLed::EFISL_NDB_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowARPTCapt", [product](int show) {
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
    return Dataref::getInstance()->exists("AirbusFBW/FCUAvail");
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
        {3, "AP1", "AirbusFBW/AP1Engage"},
        {4, "AP2", "AirbusFBW/AP2Engage"},
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
        {25, "ALT 100", "AirbusFBW/ALT100_1000", 0.0},        // Set to 0 for 100ft increments
        {26, "ALT 1000", "AirbusFBW/ALT100_1000", 1.0},       // Set to 1 for 1000ft increments
        
        // Brightness control buttons
        // These are handled internally via brightness callback system
        // {27, "BRIGHT", "AirbusFBW/SupplLightLevelRehostats[0]"},  // Panel brightness
        // {27, "BRIGHT_LCD", "AirbusFBW/SupplLightLevelRehostats[1]"},  // LCD brightness
        
        // LED control buttons for monitoring only (no direct button presses)
        // {28, "APPR_LED", "AirbusFBW/APPRilluminated"},  // Monitored via LED callback
        // {29, "ATHR_LED", "AirbusFBW/ATHRmode"},  // Monitored via LED callback  
        // {30, "LOC_LED", "AirbusFBW/LOCilluminated"},  // Monitored via LED callback
        // Button 31 reserved
        
        // EFIS Right (Captain) buttons (32-63)
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
        {43, "R_inHg", "AirbusFBW/BaroUnitFO", 0.0},          // Set to 0 for inHg
        {44, "R_hPa", "AirbusFBW/BaroUnitFO", 1.0},           // Set to 1 for hPa
        {45, "R_MODE LS", "AirbusFBW/NDmodeFO", 0.0},         // LS mode
        {46, "R_MODE VOR", "AirbusFBW/NDmodeFO", 1.0},        // VOR mode
        {47, "R_MODE NAV", "AirbusFBW/NDmodeFO", 2.0},        // NAV mode
        {48, "R_MODE ARC", "AirbusFBW/NDmodeFO", 3.0},        // ARC mode
        {49, "R_MODE PLAN", "AirbusFBW/NDmodeFO", 4.0},       // PLAN mode
        {50, "R_RANGE 10", "AirbusFBW/NDrangeFO", 0.0},       // 10nm range
        {51, "R_RANGE 20", "AirbusFBW/NDrangeFO", 1.0},       // 20nm range
        {52, "R_RANGE 40", "AirbusFBW/NDrangeFO", 2.0},       // 40nm range
        {53, "R_RANGE 80", "AirbusFBW/NDrangeFO", 3.0},       // 80nm range
        {54, "R_RANGE 160", "AirbusFBW/NDrangeFO", 4.0},      // 160nm range
        {55, "R_RANGE 320", "AirbusFBW/NDrangeFO", 5.0},      // 320nm range
        {56, "R_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", 2.0},    // VOR1
        {57, "R_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", 1.0},    // OFF1
        {58, "R_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", 0.0},    // ADF1
        {59, "R_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", 2.0},    // VOR2
        {60, "R_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", 1.0},    // OFF2
        {61, "R_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", 0.0},    // ADF2
        // Buttons 62-63 reserved
        
        // EFIS Left (First Officer) buttons (64-95)
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
        {75, "L_inHg", "AirbusFBW/BaroUnitCapt", 0.0},        // Set to 0 for inHg
        {76, "L_hPa", "AirbusFBW/BaroUnitCapt", 1.0},         // Set to 1 for hPa
        {77, "L_MODE LS", "AirbusFBW/NDmodeCapt", 0.0},       // LS mode
        {78, "L_MODE VOR", "AirbusFBW/NDmodeCapt", 1.0},      // VOR mode
        {79, "L_MODE NAV", "AirbusFBW/NDmodeCapt", 2.0},      // NAV mode
        {80, "L_MODE ARC", "AirbusFBW/NDmodeCapt", 3.0},      // ARC mode
        {81, "L_MODE PLAN", "AirbusFBW/NDmodeCapt", 4.0},     // PLAN mode
        {82, "L_RANGE 10", "AirbusFBW/NDrangeCapt", 0.0},     // 10nm range
        {83, "L_RANGE 20", "AirbusFBW/NDrangeCapt", 1.0},     // 20nm range
        {84, "L_RANGE 40", "AirbusFBW/NDrangeCapt", 2.0},     // 40nm range
        {85, "L_RANGE 80", "AirbusFBW/NDrangeCapt", 3.0},     // 80nm range
        {86, "L_RANGE 160", "AirbusFBW/NDrangeCapt", 4.0},    // 160nm range
        {87, "L_RANGE 320", "AirbusFBW/NDrangeCapt", 5.0},    // 320nm range
        {88, "L_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", 0.0},      // ADF1
        {89, "L_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", 1.0},      // OFF1
        {90, "L_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", 2.0},      // VOR1
        {91, "L_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", 0.0},      // ADF2
        {92, "L_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", 1.0},      // OFF2
        {93, "L_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", 2.0},      // VOR2
        // Buttons 94-95 reserved
    };
    return buttons;
}

void TolissFCUEfisProfile::updateDisplayData(FCUDisplayData& data) {
    auto datarefManager = Dataref::getInstance();
    
    // Format FCU speed display - using sim/cockpit2/autopilot/airspeed_dial_kts_mach (float)
    float speed = datarefManager->getCached<float>("sim/cockpit2/autopilot/airspeed_dial_kts_mach");
    if (speed > 0 && datarefManager->getCached<int>("AirbusFBW/SPDdashed") == 0) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << static_cast<int>(speed);
        data.speed = ss.str();
    } else {
        data.speed = "---";
    }
    
    // Format FCU heading display - using sim/cockpit/autopilot/heading_mag (float)
    float heading = datarefManager->getCached<float>("sim/cockpit/autopilot/heading_mag");
    if (heading >= 0 && datarefManager->getCached<int>("AirbusFBW/HDGdashed") == 0) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << static_cast<int>(heading);
        data.heading = ss.str();
    } else {
        data.heading = "---";
    }
    
    // Format FCU altitude display - using sim/cockpit/autopilot/altitude (float)
    float altitude = datarefManager->getCached<float>("sim/cockpit/autopilot/altitude");
    if (altitude >= 0) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << static_cast<int>(altitude);
        data.altitude = ss.str();
    } else {
        data.altitude = "-----";
    }
    
    // Format vertical speed display - using sim/cockpit/autopilot/vertical_velocity (float)
    float vs = datarefManager->getCached<float>("sim/cockpit/autopilot/vertical_velocity");
    if (datarefManager->getCached<int>("AirbusFBW/VSdashed") == 0 && vs != 0) {
        std::stringstream ss;
        if (vs > 0) {
            ss << "+" << std::setfill('0') << std::setw(4) << static_cast<int>(vs);
        } else {
            ss << "-" << std::setfill('0') << std::setw(4) << static_cast<int>(std::abs(vs));
        }
        data.verticalSpeed = ss.str();
    } else {
        data.verticalSpeed = "     ";
    }
    
    // Set managed mode indicators - using validated int datarefs (1 or 0)
    data.spdManaged = (datarefManager->getCached<int>("AirbusFBW/SPDmanaged") == 1);
    data.hdgManaged = (datarefManager->getCached<int>("AirbusFBW/HDGmanaged") == 1);
    data.altManaged = (datarefManager->getCached<int>("AirbusFBW/ALTmanaged") == 1);
    
    // Speed/Mach mode - using sim/cockpit/autopilot/airspeed_is_mach (int, 1 or 0)
    data.spdMach = (datarefManager->getCached<int>("sim/cockpit/autopilot/airspeed_is_mach") == 1);
    
    // HDG/TRK mode - using AirbusFBW/HDGTRKmode (int, HDG=0, TRK=1)
    data.hdgTrk = (datarefManager->getCached<int>("AirbusFBW/HDGTRKmode") == 1);
    
    // VS/FPA mode - based on HDG/TRK mode (when TRK=1, we use FPA instead of VS)
    data.vsMode = (datarefManager->getCached<int>("AirbusFBW/HDGTRKmode") == 0); // VS mode when HDG mode
    data.fpaMode = (datarefManager->getCached<int>("AirbusFBW/HDGTRKmode") == 1); // FPA mode when TRK mode
    
    // Format EFIS barometric pressure displays - Captain side (Right display)
    int baroStdCapt = datarefManager->getCached<int>("AirbusFBW/BaroStdCapt"); // int, 1 or 0
    int baroUnitCapt = datarefManager->getCached<int>("AirbusFBW/BaroUnitCapt"); // int, 1 for hPa, 0 for inHg
    
    if (baroStdCapt == 1) {
        data.efisRBaro = "STD ";  // 4 characters for STD mode
        data.efisRQnh = false;
        data.efisRHpaDec = false;
    } else {
        // Get actual barometric pressure value (float, inHg)
        float baroValue = datarefManager->getCached<float>("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot");
        if (baroValue > 0) {
            if (baroUnitCapt == 0) {  // inHg mode - need to send as "2992" with decimal flag
                int scaledValue = static_cast<int>(baroValue * 100);  // 29.92 -> 2992
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << scaledValue;
                data.efisRBaro = ss.str();
                data.efisRQnh = true;     // QNH mode indicator
                data.efisRHpaDec = true;  // Show decimal point
            } else {  // hPa mode (baroUnitCapt == 1)
                int hpaValue = static_cast<int>(baroValue * 33.8639);  // Convert inHg to hPa
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << hpaValue;
                data.efisRBaro = ss.str();
                data.efisRQnh = true;     // QNH mode indicator
                data.efisRHpaDec = false; // No decimal point for hPa
            }
        } else {
            data.efisRBaro = "2992";  // Default fallback for 29.92
            data.efisRQnh = true;
            data.efisRHpaDec = (baroUnitCapt == 0);  // Decimal only for inHg
        }
    }
    
    // Format EFIS barometric pressure displays - First Officer side (Left display)
    int baroStdFO = datarefManager->getCached<int>("AirbusFBW/BaroStdFO"); // int, 1 or 0
    int baroUnitFO = datarefManager->getCached<int>("AirbusFBW/BaroUnitFO"); // int, 1 for hPa, 0 for inHg
    
    if (baroStdFO == 1) {
        data.efisLBaro = "STD ";  // 4 characters for STD mode
        data.efisLQnh = false;
        data.efisLHpaDec = false;
    } else {
        // Get actual barometric pressure value (float, inHg)
        float baroValue = datarefManager->getCached<float>("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot");
        if (baroValue > 0) {
            if (baroUnitFO == 0) {  // inHg mode - need to send as "2992" with decimal flag
                int scaledValue = static_cast<int>(baroValue * 100);  // 29.92 -> 2992
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << scaledValue;
                data.efisLBaro = ss.str();
                data.efisLQnh = true;     // QNH mode indicator
                data.efisLHpaDec = true;  // Show decimal point
            } else {  // hPa mode (baroUnitFO == 1)
                int hpaValue = static_cast<int>(baroValue * 33.8639);  // Convert inHg to hPa
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << hpaValue;
                data.efisLBaro = ss.str();
                data.efisLQnh = true;     // QNH mode indicator
                data.efisLHpaDec = false; // No decimal point for hPa
            }
        } else {
            data.efisLBaro = "2992";  // Default fallback for 29.92
            data.efisLQnh = true;
            data.efisLHpaDec = (baroUnitFO == 0);  // Decimal only for inHg
        }
    }
}

void TolissFCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (button->value >= 0) {
        Dataref::getInstance()->set<float>(button->dataref.c_str(), button->value);
    } else {
        Dataref::getInstance()->executeCommand(button->dataref.c_str(), xplm_CommandBegin);
    }
}
