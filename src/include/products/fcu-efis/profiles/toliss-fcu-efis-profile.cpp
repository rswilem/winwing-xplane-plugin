#include "toliss-fcu-efis-profile.h"
#include "product-fcu-efis.h"
#include "dataref.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

TolissFCUEfisProfile::TolissFCUEfisProfile(ProductFCUEfis *product) : FCUEfisAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/SupplLightLevelRehostats", [product](std::vector<float> brightness) {
        uint8_t target = brightness[0] * 255.0f;
        debug_force("[TolissFCUEfisProfile] Brightness update: panel=%f, target=%d\n", brightness[0], target);
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::FLAG_GREEN, target);
        product->setLedBrightness(FCUEfisLed::EFISR_FLAG_GREEN, target);
        debug_force("[TolissFCUEfisProfile] Setting EFISL_FLAG_GREEN (202) to brightness %d\n", target);
        product->setLedBrightness(FCUEfisLed::EFISL_FLAG_GREEN, target);
        
        uint8_t screenBrightness = brightness[1] * 255.0f;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AP1Engage", [product](int engaged) {
        debug_force("[TolissFCUEfisProfile] AP1Engage changed: %d\n", engaged);
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AP2Engage", [product](int engaged) {
        debug_force("[TolissFCUEfisProfile] AP2Engage changed: %d\n", engaged);
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ATHRmode", [product](int mode) {
        debug_force("[TolissFCUEfisProfile] ATHRmode changed: %d\n", mode);
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, mode > 0 ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/LOCilluminated", [product](int illuminated) {
        debug_force("[TolissFCUEfisProfile] LOCilluminated changed: %d\n", illuminated);
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, illuminated ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/APPRilluminated", [product](int illuminated) {
        debug_force("[TolissFCUEfisProfile] APPRilluminated changed: %d\n", illuminated);
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, illuminated ? 255 : 0);
    });
    
    // EXPED LEDs controlled by APVerticalMode like in Python reference
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/APVerticalMode", [product](int vsMode) {
        debug_force("[TolissFCUEfisProfile] APVerticalMode changed: %d\n", vsMode);
        // Both EXPED LEDs use the same condition: vsMode >= 112
        uint8_t brightness = vsMode >= 112 ? 255 : 0;
        product->setLedBrightness(FCUEfisLed::EXPED_GREEN, brightness);
        product->setLedBrightness(FCUEfisLed::EXPED_YELLOW, brightness);
    });
    
    // Monitor EFIS Right (Captain) LED states
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/FD2Engage", [product](int engaged) {
        debug_force("[TolissFCUEfisProfile] EFIS-R FD2Engage changed: %d\n", engaged);
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ILSonFO", [product](int on) {
        debug_force("[TolissFCUEfisProfile] EFIS-R ILSonFO changed: %d\n", on);
        product->setLedBrightness(FCUEfisLed::EFISR_LS_GREEN, on ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowCSTRFO", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-R NDShowCSTRFO changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowWPTFO", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-R NDShowWPTFO changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowVORDFO", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-R NDShowVORDFO changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowNDBFO", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-R NDShowNDBFO changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISR_NDB_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowARPTFO", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-R NDShowARPTFO changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, show ? 255 : 0);
    });
    
    // Monitor EFIS Left (First Officer) LED states
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/FD1Engage", [product](int engaged) {
        debug_force("[TolissFCUEfisProfile] EFIS-L FD1Engage changed: %d\n", engaged);
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, engaged ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/ILSonCapt", [product](int on) {
        debug_force("[TolissFCUEfisProfile] EFIS-L ILSonCapt changed: %d\n", on);
        product->setLedBrightness(FCUEfisLed::EFISL_LS_GREEN, on ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowCSTRCapt", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-L NDShowCSTRCapt changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowWPTCapt", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-L NDShowWPTCapt changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowVORDCapt", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-L NDShowVORDCapt changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowNDBCapt", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-L NDShowNDBCapt changed: %d\n", show);
        product->setLedBrightness(FCUEfisLed::EFISL_NDB_GREEN, show ? 255 : 0);
    });
    
    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NDShowARPTCapt", [product](int show) {
        debug_force("[TolissFCUEfisProfile] EFIS-L NDShowARPTCapt changed: %d\n", show);
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
    // Note: APVerticalMode is used for both EXPED LEDs, so it's unbound with the EXPED_YELLOW monitoring
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
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::vector<std::string>& TolissFCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // FCU display datarefs (based on Python reference script)
        "sim/cockpit/autopilot/airspeed_is_mach",
        "sim/cockpit2/autopilot/airspeed_dial_kts_mach",
        "sim/cockpit/autopilot/heading_mag",
        "sim/cockpit/autopilot/altitude",
        "sim/cockpit/autopilot/vertical_velocity",
        
        // Airbus FBW managed modes and display states
        "AirbusFBW/SPDmanaged",
        "AirbusFBW/HDGmanaged",
        "AirbusFBW/ALTmanaged",
        "AirbusFBW/HDGTRKmode",
        "AirbusFBW/APVerticalMode",
        "AirbusFBW/HDGdashed",
        "AirbusFBW/SPDdashed",
        "AirbusFBW/VSdashed",
        
        // EFIS barometric pressure datarefs
        "AirbusFBW/BaroStdCapt",
        "AirbusFBW/BaroUnitCapt",
        "AirbusFBW/BaroStdFO",
        "AirbusFBW/BaroUnitFO",
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot",
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot",
        
        // EFIS selection datarefs for ADF/VOR monitoring
        "sim/cockpit2/EFIS/EFIS_1_selection_pilot",
        "sim/cockpit2/EFIS/EFIS_2_selection_pilot",
        "sim/cockpit2/EFIS/EFIS_1_selection_copilot", 
        "sim/cockpit2/EFIS/EFIS_2_selection_copilot",
        
        // HDG/TRK mode switching dataref
        "AirbusFBW/HDGTRKmode",
    };
    return datarefs;
}

const std::vector<FCUEfisButtonDef>& TolissFCUEfisProfile::buttonDefs() const {
    // Button mapping based on Python reference - hardware button index matches button ID
    static const std::vector<FCUEfisButtonDef> buttons = {
        // FCU buttons (0-31) - hardware index matches button function
        {0, "MACH", "toliss_airbus/ias_mach_button_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {1, "LOC", "AirbusFBW/LOCbutton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {2, "TRK", "toliss_airbus/hdgtrk_button_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {3, "AP1", "AirbusFBW/AP1Engage", FCUEfisDatarefType::DATA, FCUEfisButtonType::TOGGLE},
        {4, "AP2", "AirbusFBW/AP2Engage", FCUEfisDatarefType::DATA, FCUEfisButtonType::TOGGLE},
        {5, "A/THR", "AirbusFBW/ATHRbutton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {6, "EXPED", "AirbusFBW/EXPEDbutton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {7, "METRIC", "toliss_airbus/metric_alt_button_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {8, "APPR", "AirbusFBW/APPRbutton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {9, "SPD DEC", "sim/autopilot/airspeed_down", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {10, "SPD INC", "sim/autopilot/airspeed_up", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {11, "SPD PUSH", "AirbusFBW/PushSPDSel", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {12, "SPD PULL", "AirbusFBW/PullSPDSel", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {13, "HDG DEC", "sim/autopilot/heading_down", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {14, "HDG INC", "sim/autopilot/heading_up", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {15, "HDG PUSH", "AirbusFBW/PushHDGSel", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {16, "HDG PULL", "AirbusFBW/PullHDGSel", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {17, "ALT DEC", "sim/autopilot/altitude_down", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {18, "ALT INC", "sim/autopilot/altitude_up", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {19, "ALT PUSH", "AirbusFBW/PushAltitude", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {20, "ALT PULL", "AirbusFBW/PullAltitude", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {21, "VS DEC", "sim/autopilot/vertical_speed_down", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {22, "VS INC", "sim/autopilot/vertical_speed_up", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {23, "VS PUSH", "AirbusFBW/PushVSSel", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {24, "VS PULL", "AirbusFBW/PullVSSel", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {25, "ALT 100", "AirbusFBW/ALT100_1000", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {26, "ALT 1000", "AirbusFBW/ALT100_1000", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {27, "BRIGHT", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},      // Handled by brightness callback
        {28, "BRIGHT_LCD", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},  // Handled by brightness callback
        {29, "APPR_LED", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},    // Monitored via LED callback
        {30, "ATHR_LED", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},    // Monitored via LED callback
        {31, "LOC_LED", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},     // Monitored via LED callback
        
        // EFIS Right buttons (32-63)
        {32, "R_FD", "toliss_airbus/fd2_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::TOGGLE},
        {33, "R_LS", "toliss_airbus/dispcommands/CoLSButtonPush", FCUEfisDatarefType::CMD, FCUEfisButtonType::TOGGLE},
        {34, "R_CSTR", "toliss_airbus/dispcommands/CoCstrPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {35, "R_WPT", "toliss_airbus/dispcommands/CoWptPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {36, "R_VOR.D", "toliss_airbus/dispcommands/CoVorDPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {37, "R_NDB", "toliss_airbus/dispcommands/CoNdbPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {38, "R_ARPT", "toliss_airbus/dispcommands/CoArptPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {39, "R_STD PUSH", "toliss_airbus/copilot_baro_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {40, "R_STD PULL", "toliss_airbus/copilot_baro_pull", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {41, "R_PRESS DEC", "sim/instruments/barometer_copilot_down", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {42, "R_PRESS INC", "sim/instruments/barometer_copilot_up", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {43, "R_inHg", "AirbusFBW/BaroUnitFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {44, "R_hPa", "AirbusFBW/BaroUnitFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {45, "R_MODE LS", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {46, "R_MODE VOR", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {47, "R_MODE NAV", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {48, "R_MODE ARC", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_3},
        {49, "R_MODE PLAN", "AirbusFBW/NDmodeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_4},
        {50, "R_RANGE 10", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {51, "R_RANGE 20", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {52, "R_RANGE 40", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {53, "R_RANGE 80", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_3},
        {54, "R_RANGE 160", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_4},
        {55, "R_RANGE 320", "AirbusFBW/NDrangeFO", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_5},
        {56, "R_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {57, "R_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {58, "R_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_copilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {59, "R_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {60, "R_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {61, "R_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_copilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {62, "R_RESERVED_1", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},
        {63, "R_RESERVED_2", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},
        
        // EFIS Left buttons (64-95)
        {64, "L_FD", "toliss_airbus/fd1_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::TOGGLE},
        {65, "L_LS", "toliss_airbus/dispcommands/CaptLSButtonPush", FCUEfisDatarefType::CMD, FCUEfisButtonType::TOGGLE},
        {66, "L_CSTR", "toliss_airbus/dispcommands/CaptCstrPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {67, "L_WPT", "toliss_airbus/dispcommands/CaptWptPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {68, "L_VOR.D", "toliss_airbus/dispcommands/CaptVorDPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {69, "L_NDB", "toliss_airbus/dispcommands/CaptNdbPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {70, "L_ARPT", "toliss_airbus/dispcommands/CaptArptPushButton", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {71, "L_STD PUSH", "toliss_airbus/capt_baro_push", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {72, "L_STD PULL", "toliss_airbus/capt_baro_pull", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {73, "L_PRESS DEC", "sim/instruments/barometer_down", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {74, "L_PRESS INC", "sim/instruments/barometer_up", FCUEfisDatarefType::CMD, FCUEfisButtonType::SWITCH},
        {75, "L_inHg", "AirbusFBW/BaroUnitCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {76, "L_hPa", "AirbusFBW/BaroUnitCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {77, "L_MODE LS", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {78, "L_MODE VOR", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {79, "L_MODE NAV", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {80, "L_MODE ARC", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_3},
        {81, "L_MODE PLAN", "AirbusFBW/NDmodeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_4},
        {82, "L_RANGE 10", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {83, "L_RANGE 20", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {84, "L_RANGE 40", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {85, "L_RANGE 80", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_3},
        {86, "L_RANGE 160", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_4},
        {87, "L_RANGE 320", "AirbusFBW/NDrangeCapt", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_5},
        {88, "L_1 ADF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {89, "L_1 OFF", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {90, "L_1 VOR", "sim/cockpit2/EFIS/EFIS_1_selection_pilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {91, "L_2 ADF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_0},
        {92, "L_2 OFF", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_1},
        {93, "L_2 VOR", "sim/cockpit2/EFIS/EFIS_2_selection_pilot", FCUEfisDatarefType::DATA, FCUEfisButtonType::SEND_2},
        {94, "L_RESERVED_1", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE},
        {95, "L_RESERVED_2", "", FCUEfisDatarefType::DATA, FCUEfisButtonType::NONE}
    };
    return buttons;
}

void TolissFCUEfisProfile::updateDisplayData(FCUDisplayData& data, const std::map<std::string, std::string>& datarefValues) {
    auto getValue = [&](const std::string& key) -> std::string {
        auto it = datarefValues.find(key);
        return (it != datarefValues.end()) ? it->second : "";
    };
    
    // Format FCU speed display (using standard sim datarefs)
    std::string speedStr = getValue("sim/cockpit2/autopilot/airspeed_dial_kts_mach");
    std::string isMachStr = getValue("sim/cockpit/autopilot/airspeed_is_mach");
    if (!speedStr.empty()) {
        float speed = std::stof(speedStr);
        bool isMach = !isMachStr.empty() && std::stoi(isMachStr) == 1;
        
        if (speed > 0) {
            std::stringstream ss;
            if (isMach) {
                ss << std::fixed << std::setprecision(2) << speed;
                data.speed = ss.str();
                data.spdMach = true;
            } else {
                ss << std::setfill('0') << std::setw(3) << (int)speed;
                data.speed = ss.str();
                data.spdMach = false;
            }
        } else {
            data.speed = "---";
        }
    }
    
    // Format FCU heading display
    std::string headingStr = getValue("sim/cockpit/autopilot/heading_mag");
    if (!headingStr.empty()) {
        float heading = std::stof(headingStr);
        if (heading >= 0) {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(3) << (int)heading;
            data.heading = ss.str();
        } else {
            data.heading = "---";
        }
    }
    
    // Format FCU altitude display
    std::string altitudeStr = getValue("sim/cockpit/autopilot/altitude");
    if (!altitudeStr.empty()) {
        float altitude = std::stof(altitudeStr);
        if (altitude >= 0) {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(5) << (int)altitude;
            data.altitude = ss.str();
        } else {
            data.altitude = "-----";
        }
    }
    
    // Format vertical speed display - handle unit conversion like Python reference
    std::string vsStr = getValue("sim/cockpit/autopilot/vertical_velocity");
    if (!vsStr.empty()) {
        float vsFloat = std::stof(vsStr);
        
        // Debug log to understand what values we're getting
        debug_force("[TolissFCUEfisProfile] VS raw value: %.2f\n", vsFloat);
        
        // Check if in HDG mode vs FPA mode - affects formatting
        std::string hdgTrkStr = getValue("AirbusFBW/HDGTRKmode");
        bool hdgMode = !(hdgTrkStr == "1" || hdgTrkStr == "1.00" || 
                        (!hdgTrkStr.empty() && std::stof(hdgTrkStr) >= 0.5));  // false = TRK mode
        
        int vs;
        if (hdgMode) {
            // In HDG mode, use raw value (feet per minute)
            vs = static_cast<int>(std::round(vsFloat));
        } else {
            // In TRK/FPA mode, divide by 100 like Python reference
            vs = static_cast<int>(std::round(vsFloat / 100.0));
        }
        
        debug_force("[TolissFCUEfisProfile] VS processed value: %d, HDG mode: %s\n", vs, hdgMode ? "true" : "false");
        
        if (vs != 0) {
            // Format like Python reference: 2 digits + padding
            std::stringstream ss;
            char sign = (vs > 0) ? '+' : '-';
            int absVs = std::abs(vs);
            
            if (hdgMode) {
                // HDG mode: pad with spaces and set FPA comma flag
                ss << sign << std::setfill('0') << std::setw(2) << absVs << "  ";
                data.fpaComma = true;
            } else {
                // TRK/FPA mode: pad with # characters (creates dimmed segments)
                ss << sign << std::setfill('0') << std::setw(2) << absVs << "##";
                data.fpaComma = false;
            }
            
            data.verticalSpeed = ss.str();
            
            // Set vertical line flag based on sign
            data.vsVerticalLine = (vs > 0);
        } else {
            data.verticalSpeed = "-----";  // 5 dashes when VS is zero
            data.vsVerticalLine = false;
        }
        
        debug_force("[TolissFCUEfisProfile] VS final string: '%s'\n", data.verticalSpeed.c_str());
    } else {
        data.verticalSpeed = "-----";  // 5 dashes when no data
        data.vsVerticalLine = false;
        debug_force("[TolissFCUEfisProfile] VS dataref empty, using dashes\n");
    }
    
    // Set managed mode indicators (using correct datarefs from Python script)
    data.spdManaged = (getValue("AirbusFBW/SPDmanaged") == "1");
    data.hdgManaged = (getValue("AirbusFBW/HDGmanaged") == "1");
    data.altManaged = (getValue("AirbusFBW/ALTmanaged") == "1");
    data.hdgTrk = (getValue("AirbusFBW/HDGTRKmode") == "1");
    
    // Debug heading managed mode
    debug_force("[TolissFCUEfisProfile] HDGmanaged dataref='%s', hdgManaged=%s\n", 
                getValue("AirbusFBW/HDGmanaged").c_str(), 
                data.hdgManaged ? "true" : "false");
    
    // Check for dashed displays (when values are not available)
    bool spdDashed = (getValue("AirbusFBW/SPDdashed") == "1");
    bool hdgDashed = (getValue("AirbusFBW/HDGdashed") == "1");
    bool vsDashed = (getValue("AirbusFBW/VSdashed") == "1");
    
    if (spdDashed) data.speed = "---";
    if (hdgDashed) data.heading = "---";
    if (vsDashed) data.verticalSpeed = "-----";  // 5 dashes for vertical speed
    
    // Handle HDG/TRK and VS/FPA mode switching like Python reference
    std::string hdgTrkModeStr = getValue("AirbusFBW/HDGTRKmode");
    // Handle both string and float comparisons
    bool trkMode = (hdgTrkModeStr == "1" || hdgTrkModeStr == "1.00" || 
                   (!hdgTrkModeStr.empty() && std::stof(hdgTrkModeStr) >= 0.5));
    data.hdgTrk = trkMode;  // true = TRK mode, false = HDG mode
    
    debug_force("[TolissFCUEfisProfile] HDGTRKmode dataref: '%s', TRK mode: %s\n", 
               hdgTrkModeStr.c_str(), trkMode ? "true" : "false");
    
    // Vertical mode handling and display flags  
    std::string apVerticalMode = getValue("AirbusFBW/APVerticalMode");
    data.vsMode = !apVerticalMode.empty() && apVerticalMode != "0";
    
    // FPA mode is active when in TRK mode (HDG/TRK button switches both heading and vertical modes)
    data.fpaMode = trkMode;  // FPA active when TRK is active
    data.fpaIndication = data.fpaMode;
    
    // Set LAT mode flag - typically active when in lateral navigation modes
    data.latMode = true;  // Most of the time LAT should be visible
    
    // Format EFIS Right barometric pressure display (First Officer side)
    std::string baroStdCapt = getValue("AirbusFBW/BaroStdFO");  // Fixed: FO for right side
    std::string baroUnitCapt = getValue("AirbusFBW/BaroUnitFO"); // Fixed: FO for right side
    if (baroStdCapt == "1") {
        data.efisRBaro = "STD ";
        data.efisRQnh = false;
        data.efisRHpaDec = false;
    } else {
        // Get actual barometric pressure value
        std::string baroValueStr = getValue("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot");
        if (!baroValueStr.empty()) {
            float baroValue = std::stof(baroValueStr);
            if (baroUnitCapt == "0") {  // inHg
                // For inHg, multiply by 100 to get 4-digit value (e.g., 30.20 -> 3020)
                // The decimal point is controlled by the efisRHpaDec flag
                int inHgValue = static_cast<int>(std::round(baroValue * 100));
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << inHgValue;
                data.efisRBaro = ss.str();
                data.efisRQnh = true;   // inHg shows QNH
                data.efisRHpaDec = true; // Enable decimal for inHg
            } else {  // hPa
                int hpaValue = static_cast<int>(std::round(baroValue * 33.8639));  // Better rounding
                std::stringstream ss;
                // Use space padding for hPa values (e.g., " 977" not "0977")
                ss << std::setfill(' ') << std::setw(4) << hpaValue;
                data.efisRBaro = ss.str();
                data.efisRQnh = true;   // hPa also shows QNH
                data.efisRHpaDec = false; // No decimal for hPa
            }
        } else {
            data.efisRBaro = "2992";  // Default fallback (hPa)
            data.efisRQnh = true;     // Show QNH for default
            data.efisRHpaDec = false;
        }
    }
    
    // Format EFIS Left barometric pressure display (Captain side)
    std::string baroStdFO = getValue("AirbusFBW/BaroStdCapt");  // Fixed: Capt for left side
    std::string baroUnitFO = getValue("AirbusFBW/BaroUnitCapt"); // Fixed: Capt for left side
    
    debug_force("[TolissFCUEfisProfile] EFIS-L Baro: StdCapt='%s', UnitCapt='%s'\n", 
               baroStdFO.c_str(), baroUnitFO.c_str());
    if (baroStdFO == "1") {
        data.efisLBaro = "STD ";
        data.efisLQnh = false;
        data.efisLHpaDec = false;
    } else {
        // Get actual barometric pressure value
        std::string baroValueStr = getValue("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot");
        debug_force("[TolissFCUEfisProfile] EFIS-L Baro raw value: '%s'\n", baroValueStr.c_str());
        
        if (!baroValueStr.empty()) {
            float baroValue = std::stof(baroValueStr);
            debug_force("[TolissFCUEfisProfile] EFIS-L Baro float value: %.4f, Unit: %s\n", 
                       baroValue, (baroUnitFO == "0") ? "inHg" : "hPa");
            
            if (baroUnitFO == "0") {  // inHg
                // For inHg, multiply by 100 to get 4-digit value (e.g., 30.20 -> 3020)
                // The decimal point is controlled by the efisLHpaDec flag
                int inHgValue = static_cast<int>(std::round(baroValue * 100));
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(4) << inHgValue;
                data.efisLBaro = ss.str();
                data.efisLQnh = true;   // inHg shows QNH
                data.efisLHpaDec = true; // Enable decimal for inHg
                debug_force("[TolissFCUEfisProfile] EFIS-L inHg result: '%s'\n", data.efisLBaro.c_str());
            } else {  // hPa
                int hpaValue = static_cast<int>(std::round(baroValue * 33.8639));  // Better rounding
                std::stringstream ss;
                // Use space padding for hPa values (e.g., " 977" not "0977")
                ss << std::setfill(' ') << std::setw(4) << hpaValue;
                data.efisLBaro = ss.str();
                data.efisLQnh = true;   // hPa also shows QNH
                data.efisLHpaDec = false; // No decimal for hPa
                debug_force("[TolissFCUEfisProfile] EFIS-L hPa result: '%s'\n", data.efisLBaro.c_str());
            }
        } else {
            data.efisLBaro = "2992";  // Default fallback (hPa)
            data.efisLQnh = true;     // Show QNH for default
            data.efisLHpaDec = false;
        }
    }
}

void TolissFCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    // Only log CommandBegin and CommandEnd, skip CommandContinue to reduce spam
    if (phase != xplm_CommandContinue) {
        const char* phaseStr = (phase == xplm_CommandBegin) ? "BEGIN" : (phase == xplm_CommandEnd) ? "END" : "CONTINUE";
        debug_force("[TolissFCUEfisProfile] Button %s: ID=%d, Name='%s', Dataref='%s'\n", 
              phaseStr, button->id, button->name.c_str(), button->dataref.c_str());
    }
    
    // Special debug logging for ADF buttons
    if (button->name.find("ADF") != std::string::npos) {
        debug_force("[TolissFCUEfisProfile] ADF Button Debug: ID=%d, Name='%s', Phase=%d, ButtonType=%d, DatarefType=%d\n",
                   button->id, button->name.c_str(), phase, static_cast<int>(button->buttonType), static_cast<int>(button->datarefType));
        
        if (button->datarefType == FCUEfisDatarefType::DATA) {
            float currentValue = Dataref::getInstance()->get<float>(button->dataref.c_str());
            debug_force("[TolissFCUEfisProfile] ADF Current dataref value: %s = %.2f\n", 
                       button->dataref.c_str(), currentValue);
        }
    }
    
    // Special debug logging for TRK button
    if (button->name.find("TRK") != std::string::npos) {
        debug_force("[TolissFCUEfisProfile] TRK Button Debug: ID=%d, Name='%s', Phase=%d, Command='%s'\n",
                   button->id, button->name.c_str(), phase, button->dataref.c_str());
        
        // Also check the current HDGTRKmode state
        std::string hdgTrkModeStr = Dataref::getInstance()->exists("AirbusFBW/HDGTRKmode") ? 
                                   std::to_string(Dataref::getInstance()->get<int>("AirbusFBW/HDGTRKmode")) : "N/A";
        debug_force("[TolissFCUEfisProfile] Current HDGTRKmode state: %s\n", hdgTrkModeStr.c_str());
    }
    
    if (button->dataref.empty()) {
        return;
    }
    
    // Handle different button types like Python implementation
    switch (button->buttonType) {
        case FCUEfisButtonType::SWITCH:
            if (button->datarefType == FCUEfisDatarefType::DATA) {
                // Only set value on button press, not on continuous
                if (phase == xplm_CommandBegin) {
                    Dataref::getInstance()->set<float>(button->dataref.c_str(), 1.0f);
                }
            } else {
                // For commands, only send on CommandBegin to avoid multiple triggers
                // This matches the Python implementation behavior
                if (phase == xplm_CommandBegin) {
                    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
                }
            }
            break;
            
        case FCUEfisButtonType::TOGGLE:
            if (button->datarefType == FCUEfisDatarefType::DATA) {
                // Only toggle on button press, not on continuous
                if (phase == xplm_CommandBegin) {
                    float currentValue = Dataref::getInstance()->get<float>(button->dataref.c_str());
                    float newValue = currentValue ? 0.0f : 1.0f;
                    Dataref::getInstance()->set<float>(button->dataref.c_str(), newValue);
                }
            } else {
                // For commands, only send on CommandBegin to avoid multiple triggers
                if (phase == xplm_CommandBegin) {
                    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
                }
            }
            break;
            
        case FCUEfisButtonType::SEND_0:
        case FCUEfisButtonType::SEND_1:
        case FCUEfisButtonType::SEND_2:
        case FCUEfisButtonType::SEND_3:
        case FCUEfisButtonType::SEND_4:
        case FCUEfisButtonType::SEND_5:
            if (button->datarefType == FCUEfisDatarefType::DATA) {
                // For selector switches, we need to handle them specially
                // They stay "pressed" as long as the selector is in that position
                // We only want to set the dataref once when the position changes
                int valueToSend = static_cast<int>(button->buttonType) - static_cast<int>(FCUEfisButtonType::SEND_0);
                
                // Get current value from X-Plane
                float currentValue = Dataref::getInstance()->get<float>(button->dataref.c_str());
                
                // Always send the value on button press, even if already set to that value
                // This ensures rotary selectors work properly when switching positions
                if (phase == xplm_CommandBegin) {
                    debug_force("[TolissFCUEfisProfile] Selector change: %s from %d to %d\n", 
                               button->dataref.c_str(), static_cast<int>(currentValue), valueToSend);
                    Dataref::getInstance()->set<float>(button->dataref.c_str(), static_cast<float>(valueToSend));
                }
            }
            break;
            
        case FCUEfisButtonType::NONE:
            break;
            
        default:
            break;
    }
}
