#include "toliss-agp-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-agp.h"
#include "segment-display.h"

#include <algorithm>
#include <cmath>

TolissAGPProfile::TolissAGPProfile(ProductAGP *product) : AGPAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
        uint8_t backlightBrightness = hasPower ? brightness * 255 : 0;

        product->setLedBrightness(AGPLed::BACKLIGHT, backlightBrightness);
        product->setLedBrightness(AGPLed::LCD_BRIGHTNESS, backlightBrightness);
        product->setLedBrightness(AGPLed::OVERALL_LEDS_BRIGHTNESS, hasPower ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AnnunMode", [this, product](int annunMode) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/NoseGearInd");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/LeftGearInd");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/RightGearInd");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/TerrainSelectedND1");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/TerrainSelectedND2");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AutoBrkLo");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/BrakeFan");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/BrakeTemperatureArray");

        product->setLedBrightness(AGPLed::LDG_GEAR_LEVER_RED, isAnnunTest() ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/NoseGearInd", [this, product](int indicator) {
        product->setLedBrightness(AGPLed::LDG_GEAR_UNLK_CENTER, (indicator & 1) || isAnnunTest() ? 1 : 0);
        product->setLedBrightness(AGPLed::LDG_GEAR_ARROW_GREEN_CENTER, (indicator & 2) || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/LeftGearInd", [this, product](int indicator) {
        product->setLedBrightness(AGPLed::LDG_GEAR_UNLK_LEFT, (indicator & 1) || isAnnunTest() ? 1 : 0);
        product->setLedBrightness(AGPLed::LDG_GEAR_ARROW_GREEN_LEFT, (indicator & 2) || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/RightGearInd", [this, product](int indicator) {
        product->setLedBrightness(AGPLed::LDG_GEAR_UNLK_RIGHT, (indicator & 1) || isAnnunTest() ? 1 : 0);
        product->setLedBrightness(AGPLed::LDG_GEAR_ARROW_GREEN_RIGHT, (indicator & 2) || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/TerrainSelectedND1", [this, product](bool enabled) {
        if (product->terrainNDPreference == AGPTerrainNDPreference::CAPTAIN) {
            product->setLedBrightness(AGPLed::TERRAIN_ON, enabled || isAnnunTest() ? 1 : 0);
        }
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/TerrainSelectedND2", [this, product](bool enabled) {
        if (product->terrainNDPreference == AGPTerrainNDPreference::FIRST_OFFICER) {
            product->setLedBrightness(AGPLed::TERRAIN_ON, enabled || isAnnunTest() ? 1 : 0);
        }
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AutoBrkLo", [this, product](int brakeMinimumIndicator) {
        int brakeMediumIndicator = Dataref::getInstance()->getCached<int>("AirbusFBW/AutoBrkMed");
        int brakeMaximumIndicator = Dataref::getInstance()->getCached<int>("AirbusFBW/AutoBrkMax");
        product->setLedBrightness(AGPLed::AUTOBRK_LO_ON, isAnnunTest() || brakeMinimumIndicator >= 1);
        product->setLedBrightness(AGPLed::AUTOBRK_MED_ON, isAnnunTest() || brakeMediumIndicator >= 1);
        product->setLedBrightness(AGPLed::AUTOBRK_HI_ON, isAnnunTest() || brakeMaximumIndicator >= 1);
        product->setLedBrightness(AGPLed::AUTOBRK_DECEL_LO, isAnnunTest() || brakeMinimumIndicator == 2);
        product->setLedBrightness(AGPLed::AUTOBRK_DECEL_MED, isAnnunTest() || brakeMediumIndicator == 2);
        product->setLedBrightness(AGPLed::AUTOBRK_DECEL_HI, isAnnunTest() || brakeMaximumIndicator == 2);
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AutoBrkMed", [](int indicator) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AutoBrkLo");
    });

    Dataref::getInstance()->monitorExistingDataref<int>("AirbusFBW/AutoBrkMax", [](int indicator) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/AutoBrkLo");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/BrakeFan", [this, product](bool enabled) {
        product->setLedBrightness(AGPLed::BRAKE_FAN_ON, enabled || isAnnunTest() ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/BrakeTemperatureArray", [this, product](std::vector<float> temperatures) {
        bool anyHot = std::any_of(temperatures.begin(), temperatures.end(), [](float temp) {
            return temp > 300.0f;
        });
        product->setLedBrightness(AGPLed::BRAKE_FAN_HOT, anyHot || isAnnunTest() ? 1 : 0);
    });
}

TolissAGPProfile::~TolissAGPProfile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("AirbusFBW/NoseGearInd");
    Dataref::getInstance()->unbind("AirbusFBW/LeftGearInd");
    Dataref::getInstance()->unbind("AirbusFBW/RightGearInd");
    Dataref::getInstance()->unbind("AirbusFBW/TerrainSelectedND1");
    Dataref::getInstance()->unbind("AirbusFBW/TerrainSelectedND2");
    Dataref::getInstance()->unbind("AirbusFBW/AutoBrkLo");
    Dataref::getInstance()->unbind("AirbusFBW/AutoBrkMed");
    Dataref::getInstance()->unbind("AirbusFBW/AutoBrkMax");
    Dataref::getInstance()->unbind("AirbusFBW/BrakeFan");
    Dataref::getInstance()->unbind("AirbusFBW/BrakeTemperatureArray");
}

bool TolissAGPProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::unordered_map<uint16_t, AGPButtonDef> &TolissAGPProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, AGPButtonDef> buttons = {
        {0, {"Brake fan on", "AirbusFBW/BrakeFan", AGPDatarefType::SET_VALUE, 1}},
        {1, {"Brake fan off", "AirbusFBW/BrakeFan", AGPDatarefType::SET_VALUE, 0}},
        {2, {"Autobrake Lo", "AirbusFBW/AbrkLo"}},
        {3, {"Autobrake Medium", "AirbusFBW/AbrkMed"}},
        {4, {"Autobrake Max", "AirbusFBW/AbrkMax"}},
        {5, {"Antiskid on", "AirbusFBW/NWSnAntiSkid", AGPDatarefType::SET_VALUE, 1}},
        {6, {"Antiskid off", "AirbusFBW/NWSnAntiSkid", AGPDatarefType::SET_VALUE, 0}},
        {7, {"RST Turn left", ""}},
        {8, {"RST Press", "toliss_airbus/chrono/ChronoResetPush"}},
        {9, {"RST Turn right", ""}},
        {10, {"CHR Turn left", ""}},
        {11, {"CHR Press", "toliss_airbus/chrono/ChronoStartStopPush"}},
        {12, {"CHR Turn right", ""}},
        {13, {"Date Turn left", ""}},
        {14, {"Date Press", "toliss_airbus/chrono/datePush"}},
        {15, {"Date Turn right", ""}},
        {16, {"UTC switch GPS", "ckpt/clock/gpsKnob/anim", AGPDatarefType::SET_VALUE, 0}},
        {17, {"UTC switch INT", "ckpt/clock/gpsKnob/anim", AGPDatarefType::SET_VALUE, 1}},
        {18, {"UTC switch SET", "ckpt/clock/gpsKnob/anim", AGPDatarefType::SET_VALUE, 2}},
        {19, {"ET switch RUN", "AirbusFBW/ClockETSwitch", AGPDatarefType::SET_VALUE, 0}},
        {20, {"ET switch STP", "AirbusFBW/ClockETSwitch", AGPDatarefType::SET_VALUE, 1}},
        {21, {"ET switch RST", "AirbusFBW/ClockETSwitch", AGPDatarefType::SET_VALUE, 2}},
        {22, {"TERR ON ND", "AirbusFBW/TerrainSelectedND1", AGPDatarefType::TERRAIN_ON_ND}},
        {23, {"GEAR UP", "sim/flight_controls/landing_gear_up", AGPDatarefType::LANDING_GEAR, 0}},
        {24, {"GEAR DOWN", "sim/flight_controls/landing_gear_down", AGPDatarefType::LANDING_GEAR, 1}},
    };

    return buttons;
}

void TolissAGPProfile::buttonPressed(const AGPButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    if (button->datarefType == AGPDatarefType::LANDING_GEAR) {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
        datarefManager->set<int>("ckpt/gearHandle", static_cast<int>(button->value));
    } else if (phase == xplm_CommandBegin && button->datarefType == AGPDatarefType::TERRAIN_ON_ND) {
        std::string dataref = (product->terrainNDPreference == AGPTerrainNDPreference::CAPTAIN) ? "AirbusFBW/TerrainSelectedND1" : "AirbusFBW/TerrainSelectedND2";
        datarefManager->set<int>(dataref.c_str(), datarefManager->get<bool>(dataref.c_str()) ? 0 : 1);
    } else if (phase == xplm_CommandBegin && button->datarefType == AGPDatarefType::SET_VALUE) {
        datarefManager->set<int>(button->dataref.c_str(), static_cast<int>(button->value));
    } else {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
    }
}

void TolissAGPProfile::updateDisplays() {
    if (!product) {
        return;
    }

    auto datarefManager = Dataref::getInstance();

    std::string chrono = "";
    float chronoSeconds = datarefManager->get<float>("AirbusFBW/ClockChronoValue");
    if (chronoSeconds > std::numeric_limits<float>::epsilon()) {
        chrono = SegmentDisplay::fixStringLength(std::to_string((int) std::floor(chronoSeconds)), 4);
    }

    std::string utc = "";
    std::vector<float> buttonAnimations = datarefManager->get<std::vector<float>>("AirbusFBW/ChronoButtonAnimations");
    bool dateButtonPressed = buttonAnimations.size() > 2 && buttonAnimations[2] > std::numeric_limits<float>::epsilon();
    if (dateButtonPressed) {
        int dayOfYear = datarefManager->get<int>("sim/time/local_date_days") + 1;

        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        struct tm *timeinfo = std::localtime(&time);
        int year = timeinfo->tm_year + 1900;

        // Calculate month and day from day of year
        int month = 1;
        int day = dayOfYear;
        int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            daysInMonth[1] = 29;
        }

        for (int i = 0; i < 12; i++) {
            if (day <= daysInMonth[i]) {
                month = i + 1;
                break;
            }
            day -= daysInMonth[i];
        }

        // Format the date as MMDDYY
        utc = (month < 10 ? "0" : "") + std::to_string(month) +
              (day < 10 ? "0" : "") + std::to_string(day) +
              std::to_string(year % 100);
    } else {
        double zuluTime = datarefManager->get<double>("sim/time/zulu_time_sec");

        // Convert zulu time in seconds to HHMMSS
        int hours = static_cast<int>(zuluTime / 3600) % 24;
        int minutes = static_cast<int>(zuluTime / 60) % 60;
        int seconds = static_cast<int>(zuluTime) % 60;

        utc = SegmentDisplay::fixStringLength(std::to_string(hours), 2) +
              SegmentDisplay::fixStringLength(std::to_string(minutes), 2) +
              SegmentDisplay::fixStringLength(std::to_string(seconds), 2);
    }

    std::string elapsedTime = "";
    if (datarefManager->get<bool>("AirbusFBW/ClockShowsET")) {
        int hours = datarefManager->get<int>("AirbusFBW/ClockETHours");
        int minutes = datarefManager->get<int>("AirbusFBW/ClockETMinutes");
        elapsedTime = SegmentDisplay::fixStringLength(std::to_string(hours), 2) + SegmentDisplay::fixStringLength(std::to_string(minutes), 2);
    }

    if (isAnnunTest()) {
        chrono = "8888";
        utc = "888888";
        elapsedTime = "8888";
    }

    product->setLCDText(chrono, utc, elapsedTime);
}

bool TolissAGPProfile::isAnnunTest() {
    return Dataref::getInstance()->get<int>("AirbusFBW/AnnunMode") == 2;
}