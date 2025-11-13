#include "product-ursa-minor-joystick.h"

#include "appstate.h"
#include "dataref.h"
#include "plugins-menu.h"
#include "profiles/toliss-ursa-minor-joystick-profile.h"
#include "profiles/zibo-ursa-minor-joystick-profile.h"

#include <algorithm>
#include <cmath>

ProductUrsaMinorJoystick::ProductUrsaMinorJoystick(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName, unsigned char identifierByte) :
    USBDevice(hidDevice, vendorId, productId, vendorName, productName), identifierByte(identifierByte) {
    connect();
}

ProductUrsaMinorJoystick::~ProductUrsaMinorJoystick() {
    disconnect();
}

const char *ProductUrsaMinorJoystick::classIdentifier() {
    return "Ursa Minor Joystick";
}

void ProductUrsaMinorJoystick::setProfileForCurrentAircraft() {
    if (TolissUrsaMinorJoystickProfile::IsEligible()) {
        profile = new TolissUrsaMinorJoystickProfile(this);
        profileReady = true;
    } else if (ZiboUrsaMinorJoystickProfile::IsEligible()) {
        profile = new ZiboUrsaMinorJoystickProfile(this);
        profileReady = true;
    } else {
        profile = nullptr;
        profileReady = false;
    }
}

bool ProductUrsaMinorJoystick::connect() {
    if (!USBDevice::connect()) {
        return false;
    }

    setLedBrightness(0);
    setVibration(0);

    setProfileForCurrentAircraft();

    std::string vibrationPreference = AppState::getInstance()->readPreference("JoystickVibration", "normal");
    loadVibrationFactor(vibrationPreference);

    menuItemId = PluginsMenu::getInstance()->addItem(
        classIdentifier(),
        std::vector<MenuItem>{
            {.name = "Identify", .content = [this](int menuId) {
                 setLedBrightness(255);
                 setVibration(128);
                 AppState::getInstance()->executeAfter(2000, [this]() {
                     setLedBrightness(0);
                     setVibration(0);
                 });
             }},
            {.name = "Vibration", .content = std::vector<MenuItem>{
                                      {.name = "Disabled", .checked = vibrationPreference == "disabled", .content = [this](int itemId) {
                                           AppState::getInstance()->writePreference("JoystickVibration", "disabled");
                                           loadVibrationFactor("disabled");
                                           PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                           PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                       }},
                                      {.name = "Normal", .checked = vibrationPreference == "normal", .content = [this](int itemId) {
                                           AppState::getInstance()->writePreference("JoystickVibration", "normal");
                                           loadVibrationFactor("normal");
                                           PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                           PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                       }},
                                      {.name = "Strong", .checked = vibrationPreference == "strong", .content = [this](int itemId) {
                                           AppState::getInstance()->writePreference("JoystickVibration", "strong");
                                           loadVibrationFactor("strong");
                                           PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                           PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                       }},
                                  }},
        });

    return true;
}

void ProductUrsaMinorJoystick::disconnect() {
    setLedBrightness(0);
    setVibration(0);
    PluginsMenu::getInstance()->removeItem(menuItemId);

    if (profile) {
        profile->cleanup();
        delete profile;
        profile = nullptr;
    }

    USBDevice::disconnect();
}

void ProductUrsaMinorJoystick::update() {
    if (!connected) {
        return;
    }

    USBDevice::update();

    if (profile) {
        profile->update();
    }
}

bool ProductUrsaMinorJoystick::setVibration(uint8_t vibration) {
    return writeData({0x02, identifierByte, 0xBF, 0x00, 0x00, 0x03, 0x49, 0x00, vibration, 0x00, 0x00, 0x00, 0x00, 0x00});
}

bool ProductUrsaMinorJoystick::setLedBrightness(uint8_t brightness) {
    return writeData({0x02, 0x20, 0xBB, 0x00, 0x00, 0x03, 0x49, 0x00, brightness, 0x00, 0x00, 0x00, 0x00, 0x00});
}

void ProductUrsaMinorJoystick::loadVibrationFactor(const std::string &preference) {
    if (preference == "disabled") {
        vibrationFactor = 0.0f;
    } else if (preference == "strong") {
        vibrationFactor = 1400.0f;
    } else {
        vibrationFactor = 800.0f;
    }
}
