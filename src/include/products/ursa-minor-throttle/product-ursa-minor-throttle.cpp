#include "product-ursa-minor-throttle.h"

#include "appstate.h"
#include "dataref.h"
#include "plugins-menu.h"
#include "profiles/toliss-ursa-minor-throttle-profile.h"

#include <algorithm>
#include <cmath>

ProductUrsaMinorThrottle::ProductUrsaMinorThrottle(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    lastButtonStateLo = 0;
    lastButtonStateHi = 0;
    pressedButtonIndices = {};

    connect();
}

ProductUrsaMinorThrottle::~ProductUrsaMinorThrottle() {
    disconnect();
}

const char *ProductUrsaMinorThrottle::classIdentifier() {
    return "UrsaMinorThrottle";
}

void ProductUrsaMinorThrottle::setProfileForCurrentAircraft() {
    if (TolissUrsaMinorThrottleProfile::IsEligible()) {
        profile = new TolissUrsaMinorThrottleProfile(this);
        profileReady = true;
    } else {
        profile = nullptr;
        profileReady = false;
    }
}

bool ProductUrsaMinorThrottle::connect() {
    if (!USBDevice::connect()) {
        return false;
    }

    setLedBrightness(UrsaMinorThrottleLed::BACKLIGHT, 128);
    setLedBrightness(UrsaMinorThrottleLed::OVERALL_LEDS_BRIGHTNESS, 255);
    setAllLedsEnabled(false);

    setProfileForCurrentAircraft();

    std::string vibrationPreference = AppState::getInstance()->readPreference("ThrottleVibration", "normal");
    loadVibrationSetting(vibrationPreference);

    menuItemId = PluginsMenu::getInstance()->addItem(
        classIdentifier(),
        std::vector<MenuItem>{
            {.name = "Identify", .content = [this](int menuId) {
                 setLedBrightness(UrsaMinorThrottleLed::BACKLIGHT, 128);
                 setLedBrightness(UrsaMinorThrottleLed::OVERALL_LEDS_BRIGHTNESS, 255);
                 setAllLedsEnabled(true);
                 AppState::getInstance()->executeAfter(2000, [this]() {
                     setAllLedsEnabled(false);
                 });
             }},
            {.name = "Vibration", .content = std::vector<MenuItem>{

                                      {.name = "Disabled", .checked = vibrationPreference == "disabled", .content = [this](int itemId) {
                                           AppState::getInstance()->writePreference("ThrottleVibration", "disabled");
                                           loadVibrationSetting("disabled");
                                           PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                           PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                       }},
                                      {.name = "Normal", .checked = vibrationPreference == "normal", .content = [this](int itemId) {
                                           AppState::getInstance()->writePreference("ThrottleVibration", "normal");
                                           loadVibrationSetting("normal");
                                           PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                           PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                       }},
                                      {.name = "Strong", .checked = vibrationPreference == "strong", .content = [this](int itemId) {
                                           AppState::getInstance()->writePreference("ThrottleVibration", "strong");
                                           loadVibrationSetting("strong");
                                           PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                           PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                       }},
                                  }},
        });

    return true;
}

void ProductUrsaMinorThrottle::disconnect() {
    setLedBrightness(UrsaMinorThrottleLed::BACKLIGHT, 0);
    setLedBrightness(UrsaMinorThrottleLed::OVERALL_LEDS_BRIGHTNESS, 0);

    PluginsMenu::getInstance()->removeItem(menuItemId);

    if (profile) {
        delete profile;
        profile = nullptr;
    }

    USBDevice::disconnect();
}

void ProductUrsaMinorThrottle::update() {
    if (!connected) {
        return;
    }

    USBDevice::update();

    if (profile) {
        profile->update();
    }
}

void ProductUrsaMinorThrottle::setAllLedsEnabled(bool enable) {
    unsigned char start = static_cast<unsigned char>(UrsaMinorThrottleLed::_START);
    unsigned char end = static_cast<unsigned char>(UrsaMinorThrottleLed::_END);

    for (unsigned char i = start; i <= end; ++i) {
        UrsaMinorThrottleLed led = static_cast<UrsaMinorThrottleLed>(i);
        setLedBrightness(led, enable ? 1 : 0);
    }
}

void ProductUrsaMinorThrottle::setLedBrightness(UrsaMinorThrottleLed led, uint8_t brightness) {
    writeData({0x02, ProductUrsaMinorThrottle::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(led), brightness, 0x00, 0x00, 0x00, 0x00, 0x00});
}

void ProductUrsaMinorThrottle::setVibration(uint8_t vibration) {
    writeData({0x02, ProductUrsaMinorThrottle::IdentifierByte, 0xBF, 0x00, 0x00, 0x03, 0x49, 0x00, vibration, 0x00, 0x00, 0x00, 0x00, 0x00});
}

void ProductUrsaMinorThrottle::didReceiveData(int reportId, uint8_t *report, int reportLength) {
    if (!connected || !profile || !report || reportLength <= 0) {
        return;
    }

    if (reportId != 1 || reportLength < 13) {
#if DEBUG
//        printf("[%s] Ignoring reportId %d, length %d\n", classIdentifier(), reportId, reportLength);
//        printf("[%s] Data (hex): ", classIdentifier());
//        for (int i = 0; i < reportLength; ++i) {
//            printf("%02X ", report[i]);
//        }
//        printf("\n");
#endif
        return;
    }

    uint64_t buttonsLo = 0;
    uint32_t buttonsHi = 0;
    for (int i = 0; i < 8; ++i) {
        buttonsLo |= ((uint64_t) report[i + 1]) << (8 * i);
    }
    for (int i = 0; i < 4; ++i) {
        buttonsHi |= ((uint32_t) report[i + 9]) << (8 * i);
    }

    if (buttonsLo == lastButtonStateLo && buttonsHi == lastButtonStateHi) {
        return;
    }

    lastButtonStateLo = buttonsLo;
    lastButtonStateHi = buttonsHi;

    for (int i = 0; i < 96; ++i) {
        bool pressed;

        if (i < 64) {
            pressed = (buttonsLo >> i) & 1;
        } else {
            pressed = (buttonsHi >> (i - 64)) & 1;
        }

        didReceiveButton(i, pressed);
    }
}

void ProductUrsaMinorThrottle::didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count) {
    USBDevice::didReceiveButton(hardwareButtonIndex, pressed, count);

    if (pressed) {
        debug_force("Throttle button pressed: %i\n", hardwareButtonIndex);
    }

    auto &buttons = profile->buttonDefs();
    auto it = buttons.find(hardwareButtonIndex);
    if (it == buttons.end()) {
        return;
    }

    const UrsaMinorThrottleButtonDef *buttonDef = &it->second;

    if (buttonDef->dataref.empty()) {
        return;
    }

    bool pressedButtonIndexExists = pressedButtonIndices.find(hardwareButtonIndex) != pressedButtonIndices.end();
    if (pressed && !pressedButtonIndexExists) {
        pressedButtonIndices.insert(hardwareButtonIndex);
        profile->buttonPressed(buttonDef, xplm_CommandBegin);
    } else if (pressed && pressedButtonIndexExists) {
        profile->buttonPressed(buttonDef, xplm_CommandContinue);
    } else if (!pressed && pressedButtonIndexExists) {
        pressedButtonIndices.erase(hardwareButtonIndex);
        profile->buttonPressed(buttonDef, xplm_CommandEnd);
    }
}

void ProductUrsaMinorThrottle::loadVibrationSetting(const std::string &preference) {
    if (preference == "disabled") {
        vibrationMultiplier = 0.0f;
    } else if (preference == "strong") {
        vibrationMultiplier = 1400.0f;
    } else {
        vibrationMultiplier = 800.0f;
    }
}
