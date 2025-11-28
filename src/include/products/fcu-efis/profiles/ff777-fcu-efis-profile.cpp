#include "ff777-fcu-efis-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-fcu-efis.h"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <XPLMUtilities.h>

FF777FCUEfisProfile::FF777FCUEfisProfile(ProductFCUEfis *product) : FCUEfisAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/ckpt/lights/glareshield", [product](float brightness) {
        bool hasPower = Dataref::getInstance()->getCached<bool>("1-sim/output/mcp/ok");
        uint8_t target = hasPower ? brightness * 255 : 0;
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EXPED_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);

        uint8_t screenBrightness = hasPower ? 200 : 0;
        product->setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISR_SCREEN_BACKLIGHT, screenBrightness);
        product->setLedBrightness(FCUEfisLed::EFISL_SCREEN_BACKLIGHT, screenBrightness);

        uint8_t ledBrightness = 255;
        product->setLedBrightness(FCUEfisLed::OVERALL_GREEN, hasPower ? ledBrightness : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_OVERALL_GREEN, hasPower ? ledBrightness : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_OVERALL_GREEN, hasPower ? ledBrightness : 0);

        product->forceStateSync();
    });

    // We abuse the GPU hatch dataref to trigger an update when the UI is closed.
    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/anim/hatchGPU", [product](bool gpuHatchOpen) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/output/mcp/ok");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/output/mcp/ok", [product](bool hasPower) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lights/glareshield");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/mcpCaptAP", [this, product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/mcpFoAP", [this, product](bool engaged) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, engaged || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/output/mcp/autothrottle_arm", [this, product](bool armed) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/ckpt/lampsGlow/mcpLOC", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::LOC_GREEN, armed || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/ckpt/lampsGlow/mcpAPP", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/ckpt/lampsGlow/mcpAT", [this, product](int armed) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, armed || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/mcpFdLSwitch/anim", [this, product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISL_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/mcpFdRSwitch/anim", [this, product](bool on) {
        product->setLedBrightness(FCUEfisLed::EFISR_FD_GREEN, on || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/cptCAUTION", [this, product](bool isCaution) {
        bool isWarning = Dataref::getInstance()->getCached<bool>("1-sim/ckpt/lampsGlow/cptWARNING");
        product->setLedBrightness(FCUEfisLed::EFISL_CSTR_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_WPT_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_VORD_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_NDB_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISL_ARPT_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/foCAUTION", [this, product](bool isCaution) {
        bool isWarning = Dataref::getInstance()->getCached<bool>("1-sim/ckpt/lampsGlow/foWARNING");
        product->setLedBrightness(FCUEfisLed::EFISR_CSTR_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_WPT_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_VORD_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_NDB_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
        product->setLedBrightness(FCUEfisLed::EFISR_ARPT_GREEN, isCaution || isWarning || isTestMode() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/cptWARNING", [this, product](bool on) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/cptCAUTION");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lampsGlow/foWARNING", [this, product](bool on) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/foCAUTION");
    });

    Dataref::getInstance()->monitorExistingDataref<int>("1-sim/ckpt/indLightTestSwitch/anim", [this, product](int isTest) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/output/mcp/ok");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/mcpCaptAP");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/mcpFoAP");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/output/mcp/autothrottle_arm");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/mcpLOC");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/mcpAPP");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/mcpAT");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/mcpFdLSwitch/anim");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/mcpFdRSwitch/anim");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/cptWARNING");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lampsGlow/foWARNING");
    });

    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/ckpt/cptHsiStdButton/anim", [this, product](float animValue) {
        AppState::getInstance()->executeAfterDebounced("cptStdChanged", 50, [this, product]() {
            isStdCaptain = !isStdCaptain;

            float baroValue = Dataref::getInstance()->get<float>("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot");
            if (isStdCaptain && fabs(baroValue - 29.92f) > std::numeric_limits<float>::epsilon()) {
                isStdCaptain = false;
            }

            product->updateDisplays();
        });
    });

    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/ckpt/foHsiStdButton/anim", [this, product](float animValue) {
        AppState::getInstance()->executeAfterDebounced("foStdChanged", 50, [this, product]() {
            isStdFirstOfficer = !isStdFirstOfficer;

            float baroValue = Dataref::getInstance()->get<float>("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot");
            if (isStdFirstOfficer && fabs(baroValue - 29.92f) > std::numeric_limits<float>::epsilon()) {
                isStdFirstOfficer = false;
            }

            product->updateDisplays();
        });
    });
}

FF777FCUEfisProfile::~FF777FCUEfisProfile() {
    Dataref::getInstance()->unbind("1-sim/ckpt/lights/glareshield");
    Dataref::getInstance()->unbind("1-sim/anim/hatchGPU");
    Dataref::getInstance()->unbind("1-sim/output/mcp/ok");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/mcpCaptAP");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/mcpFoAP");
    Dataref::getInstance()->unbind("1-sim/output/mcp/autothrottle_arm");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/mcpLOC");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/mcpAPP");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/mcpAT");
    Dataref::getInstance()->unbind("1-sim/ckpt/mcpFdLSwitch/anim");
    Dataref::getInstance()->unbind("1-sim/ckpt/mcpFdRSwitch/anim");
    Dataref::getInstance()->unbind("1-sim/ckpt/indLightTestSwitch/anim");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/cptCAUTION");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/foCAUTION");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/cptWARNING");
    Dataref::getInstance()->unbind("1-sim/ckpt/lampsGlow/foWARNING");
    Dataref::getInstance()->unbind("1-sim/ckpt/cptHsiStdButton/anim");
    Dataref::getInstance()->unbind("1-sim/ckpt/foHsiStdButton/anim");
}

bool FF777FCUEfisProfile::IsEligible() {
    return Dataref::getInstance()->exists("1-sim/ckpt/mcpApLButton/anim") ||
           Dataref::getInstance()->exists("1-sim/ckpt/cptHsiRangeSwitch/anim") ||
           Dataref::getInstance()->exists("1-sim/output/mcp/ok");
}

const std::vector<std::string> &FF777FCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        //"T7Avionics/mcp_power",
        "1-sim/output/mcp/ok",

        // MCP - Speed
        //"sim/cockpit2/autopilot/airspeed_dial_kts_mach",
        //"sim/cockpit/autopilot/airspeed_is_mach",
        "1-sim/output/mcp/isMachTrg",
        "1-sim/output/mcp/isHdgTrg",
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
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot",
        "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot",
        // "777/displays/captain_baro_unit", // 0=inHg, 1=hPa
        // "777/displays/fo_baro_unit",
        "1-sim/output/efis/capt/baro_mode", // 0=inHg, 1=hPa
        "1-sim/output/efis/fo/baro_mode",

        // ND Mode and Range
        // "777/displays/captain_nd_mode", // 0=APP, 1=VOR, 2=MAP, 3=PLAN
        // "777/displays/fo_nd_mode",
        // "777/displays/captain_nd_range",
        // "777/displays/fo_nd_range",
        "1-sim/efis/capt/nd_mode", // 0=APP, 1=VOR, 2=MAP, 3=PLAN
        "1-sim/efis/FO/nd_mode",
        "1-sim/efis/capt/nd_range",
        "1-sim/efis/fo/nd_range",

        "1-sim/ckpt/indLightTestSwitch/anim",
    };

    return datarefs;
}

const std::unordered_map<uint16_t, FCUEfisButtonDef> &FF777FCUEfisProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, FCUEfisButtonDef> buttons = {

        {0, {"SPD", "1-sim/command/mcpIasMachButton_button"}},
        {1, {"LOC", "1-sim/command/mcpLocButton_button"}},
        {2, {"HDG/TRK", "1-sim/command/mcpHdgTrkButton_button"}},
        {3, {"AP1", "1-sim/command/mcpApLButton_button"}},
        {4, {"AP2", "1-sim/command/mcpVnavButton_button"}},
        {5, {"A/THR", "1-sim/command/mcpAtButton_button"}},
        {6, {"EXPED", "1-sim/command/mcpApLButton_button"}},
        {7, {"VS/FPA", "1-sim/command/mcpVsFpaButton_button"}},
        {8, {"APP", "1-sim/command/mcpAppButton_button"}},
        // {"CMD R", ""}
        // {"FLCH", ""}

        // Rotary encoders - Speed
        {9, {"SPD DEC", "1-sim/command/mcpSpdRotary_rotary-"}},
        {10, {"SPD INC", "1-sim/command/mcpSpdRotary_rotary+"}},
        {11, {"SPD", "1-sim/command/mcpSpdRotary_push"}},
        //{12, },

        // Rotary encoders - Heading
        {13, {"HDG DEC", "1-sim/command/mcpHdgRotary_rotary-"}},
        {14, {"HDG INC", "1-sim/command/mcpHdgRotary_rotary+"}},
        {15, {"HDG PUSH", "1-sim/command/mcpLnavButton_button"}},
        {16, {"HDG HOLD", "1-sim/command/mcpHdgHoldButton_button"}},
        // Commande orpheline :
        // {"CO", ""}

        // Rotary encoders - Altitude
        {17, {"ALT DEC", "1-sim/command/mcpAltRotary_rotary-"}},
        {18, {"ALT INC", "1-sim/command/mcpAltRotary_rotary+"}},
        {19, {"ALT PUSH", "1-sim/command/mcpAltRotary_push"}},
        {20, {"ALT HOLD", "1-sim/command/mcpAltHoldButton_button"}},

        // Rotary encoders - Vertical Speed
        {21, {"VS DEC", "1-sim/command/mcpVsRotary_rotary-"}},
        {22, {"VS INC", "1-sim/command/mcpVsRotary_rotary+"}},
        {23, {"V/S", "1-sim/command/mcpVsButton_button"}},
        //{24, },

        // Altitude par 100/1000
        {25, {"ALT 100", "1-sim/command/mcpAltModeSwitch_set_0"}},
        {26, {"ALT 1000", "1-sim/command/mcpAltModeSwitch_set_1"}},

        {32, {"L_FD", "1-sim/command/mcpFdLSwitch_trigger"}},
        //{33, },

        // {"R_MINIMUMS", ""}

        // ND Options
        {34, {"L_DATA", "1-sim/command/cptHsiDataButton_button"}},
        {35, {"L_WPT", "1-sim/command/cptHsiWptButton_button"}},
        {36, {"L_STA", "1-sim/command/cptHsiStaButton_button"}},
        //{37, },
        {38, {"L_ARPT", "1-sim/command/cptHsiArpButton_button"}},

        // BARO
        {39, {"L_BARO PUSH", "1-sim/command/cptHsiStdButton_button"}},
        {40, {"R_BARO PULL", "1-sim/command/cptHsiStdButton_button"}},
        {41, {"L_BARO DEC", "1-sim/command/cptHsiBaroRotary_rotary-"}},
        {42, {"L_BARO INC", "1-sim/command/cptHsiBaroRotary_rotary+"}},
        {43, {"L_inHg", "1-sim/command/cptHsiBaroModeRotary_set_0"}},
        {44, {"L_hPa", "1-sim/command/cptHsiBaroModeRotary_set_1"}},

        // ND Mode selector
        {45, {"L_MODE APP", "1-sim/command/cptHsiModeSwitch_set_0"}},
        {46, {"L_MODE VOR", "1-sim/command/cptHsiModeSwitch_set_1"}},
        //{47, },
        {48, {"L_MODE MAP", "1-sim/command/cptHsiModeSwitch_set_2"}},
        {49, {"L_MODE PLAN", "1-sim/command/cptHsiModeSwitch_set_3"}},

        // ND Range selector
        {50, {"L_RANGE 10", "1-sim/command/cptHsiRangeSwitch_set_0"}},
        {51, {"L_RANGE 20", "1-sim/command/cptHsiRangeSwitch_set_1"}},
        {52, {"L_RANGE 40", "1-sim/command/cptHsiRangeSwitch_set_2"}},
        {53, {"L_RANGE 80", "1-sim/command/cptHsiRangeSwitch_set_3"}},
        {54, {"L_RANGE 160", "1-sim/command/cptHsiRangeSwitch_set_4"}},
        {55, {"L_RANGE 320", "1-sim/command/cptHsiRangeSwitch_set_5"}},
        // {"R_RANGE 5", }                                              // Position inexistante dans le B777
        // {"R_RANGE 640", }                                            // Position inexistante sur l'EFIS

        // VOR/ADF selectors
        {56, {"L_VORL VOR", "1-sim/command/cptHsiVorLSwitch_trigger"}},
        {57, {"L_VORL OFF", "1-sim/command/cptHsiVorLSwitch_trigger"}},
        {58, {"L_VORL ADF", "1-sim/command/cptHsiVorLSwitch_trigger"}},
        {59, {"L_VORR VOR", "1-sim/command/cptHsiVorRSwitch_trigger"}},
        {60, {"L_VORR OFF", "1-sim/command/cptHsiVorRSwitch_trigger"}},
        {61, {"L_VORR ADF", "1-sim/command/cptHsiVorRSwitch_trigger"}},
        // {XX, {"R_POS",  ""}},
        // {XX, {"R_TERR", ""}},

        //{62, },
        //{63, },

        {64, {"R_FD", "1-sim/command/mcpFdRSwitch_trigger"}},
        //{65, },
        // {"R_MINIMUMS", "777/displays/fo_minimums_toggle"}

        // ND Options

        {66, {"R_DATA", "1-sim/command/foHsiDataButton_button"}},
        {67, {"R_WPT", "1-sim/command/foHsiWptButton_button"}},
        {68, {"R_STA", "1-sim/command/foHsiStaButton_button"}},
        //{69, },
        {70, {"R_ARPT", "1-sim/command/foHsiArpButton_button"}},

        // BARO
        {71, {"R_BARO PUSH", "1-sim/command/foHsiStdButton_button"}},
        {72, {"R_BARO PULL", "1-sim/command/foHsiStdButton_button"}},
        {73, {"R_BARO DEC", "1-sim/command/foHsiBaroRotary_rotary-"}},
        {74, {"R_BARO INC", "1-sim/command/foHsiBaroRotary_rotary+"}},
        {75, {"R_inHg", "1-sim/command/foHsiBaroModeRotary_set_0"}},
        {76, {"R_hPa", "1-sim/command/foHsiBaroModeRotary_set_1"}},

        // ND Mode selector

        {77, {"R_MODE APP", "1-sim/command/foHsiModeSwitch_set_0"}},
        {78, {"R_MODE VOR", "1-sim/command/foHsiModeSwitch_set_1"}},
        //{79, },
        {80, {"R_MODE MAP", "1-sim/command/foHsiModeSwitch_set_2"}},
        {81, {"R_MODE PLAN", "1-sim/command/foHsiModeSwitch_set_3"}},

        // ND Range selector

        {82, {"R_RANGE 10", "1-sim/command/foHsiRangeSwitch_set_0"}},
        {83, {"R_RANGE 20", "1-sim/command/foHsiRangeSwitch_set_1"}},
        {84, {"R_RANGE 40", "1-sim/command/foHsiRangeSwitch_set_2"}},
        {85, {"R_RANGE 80", "1-sim/command/foHsiRangeSwitch_set_3"}},
        {86, {"R_RANGE 160", "1-sim/command/foHsiRangeSwitch_set_4"}},
        {87, {"R_RANGE 320", "1-sim/command/foHsiRangeSwitch_set_5"}},
        // {"R_RANGE 5", }
        // {"R_RANGE 640", }

        // VOR/ADF selectors

        {88, {"R_VORL VOR", "1-sim/command/foHsiVorLSwitch_trigger"}},
        {89, {"R_VORL OFF", "1-sim/command/foHsiVorLSwitch_trigger"}},
        {90, {"R_VORL ADF", "1-sim/command/foHsiVorLSwitch_trigger"}},
        {91, {"R_VORR VOR", "1-sim/command/foHsiVorRSwitch_trigger"}},
        {92, {"R_VORR OFF", "1-sim/command/foHsiVorRSwitch_trigger"}},
        {93, {"R_VORR ADF", "1-sim/command/foHsiVorRSwitch_trigger"}},
        // {XX, {"R_POS",  ""}},
        // {XX, {"R_TERR", ""}},

    };
    return buttons;
}

void FF777FCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    auto datarefManager = Dataref::getInstance();

    data.displayEnabled = datarefManager->getCached<bool>("1-sim/output/mcp/ok");
    data.displayTest = isTestMode();

    data.spdMach = datarefManager->getCached<bool>("1-sim/output/mcp/isMachTrg");
    float speed = datarefManager->getCached<float>("1-sim/output/mcp/spd");

    if (speed > 0) {
        std::stringstream ss;
        if (data.spdMach) {
            int machHundredths = static_cast<int>(std::round(speed * 100));
            ss << std::setfill('0') << std::setw(3) << machHundredths;
        } else {
            ss << std::setfill('0') << std::setw(3) << static_cast<int>(speed);
        }
        data.speed = ss.str();
    } else {
        data.speed = "---";
    }

    data.spdManaged = false;

    float heading = datarefManager->getCached<float>("1-sim/output/mcp/hdg");
    if (heading >= 0) {
        int hdgDisplay = static_cast<int>(heading) % 360;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << hdgDisplay;
        data.heading = ss.str();
    } else {
        data.heading = "---";
    }

    data.hdgManaged = false;
    data.hdgTrk = datarefManager->getCached<bool>("1-sim/output/mcp/isHdgTrg") == false;

    float altitude = datarefManager->getCached<float>("1-sim/output/mcp/alt");
    if (altitude >= 0) {
        int altInt = static_cast<int>(altitude);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << altInt;
        data.altitude = ss.str();
    } else {
        data.altitude = "-----";
    }

    data.altManaged = false;

    float vs = datarefManager->getCached<float>("1-sim/output/mcp/vs");

    data.vsMode = true;
    data.fpaMode = false;

    std::stringstream ss;
    int vsInt = static_cast<int>(std::round(vs));
    int absVs = std::abs(vsInt);

    ss << std::setfill('0') << std::setw(4) << absVs;
    data.verticalSpeed = ss.str();

    data.vsSign = (vs >= 0);
    data.fpaComma = false;
    data.vsIndication = true;
    data.fpaIndication = false;
    data.vsVerticalLine = true;

    data.latMode = true;

    for (int i = 0; i < 2; i++) {
        bool isCaptain = i == 0;

        bool isBaroHpa = datarefManager->getCached<bool>(isCaptain ? "1-sim/ckpt/cptHsiBaroModeRotary/anim" : "1-sim/ckpt/foHsiBaroModeRotary/anim");
        float baroValue = datarefManager->getCached<float>(isCaptain ? "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot" : "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot");

        EfisDisplayValue value = {
            .displayEnabled = data.displayEnabled,
            .displayTest = data.displayTest,
            .baro = "",
            .unitIsInHg = false,
            .isStd = (isCaptain && isStdCaptain) || (!isCaptain && isStdFirstOfficer),
        };

        if (!value.isStd && baroValue > 0) {
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

    if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::SET_VALUE) {
        datarefManager->set<float>(button->dataref.c_str(), button->value);
    }

    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::TOGGLE_VALUE) {
        int currentValue = datarefManager->get<int>(button->dataref.c_str());
        int newValue = currentValue ? 0 : 1;
        datarefManager->set<int>(button->dataref.c_str(), newValue);
    }

    else if (phase == xplm_CommandBegin && button->datarefType == FCUEfisDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    }
}

bool FF777FCUEfisProfile::isTestMode() {
    return Dataref::getInstance()->get<int>("1-sim/ckpt/indLightTestSwitch/anim") == 2;
}
