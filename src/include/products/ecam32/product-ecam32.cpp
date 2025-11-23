#include "product-ecam32.h"

#include "appstate.h"
#include "dataref.h"
#include "plugins-menu.h"
#include "profiles/toliss-ecam32-profile.h"

#include <algorithm>
#include <cmath>

ProductECAM32::ProductECAM32(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    lastButtonStateLo = 0;
    lastButtonStateHi = 0;
    pressedButtonIndices = {};

    connect();
}

ProductECAM32::~ProductECAM32() {
    disconnect();
}

const char *ProductECAM32::classIdentifier() {
    return "ECAM32";
}

void ProductECAM32::setProfileForCurrentAircraft() {
    if (TolissECAM32Profile::IsEligible()) {
        profile = new TolissECAM32Profile(this);
        profileReady = true;
    } else {
        profile = nullptr;
        profileReady = false;
    }
}

bool ProductECAM32::connect() {
    if (!USBDevice::connect()) {
        return false;
    }

    setLedBrightness(ECAM32Led::BACKLIGHT, 128);
    setLedBrightness(ECAM32Led::EMER_CANC_BRIGHTNESS, 128);
    setLedBrightness(ECAM32Led::OVERALL_LEDS_BRIGHTNESS, 255);
    setAllLedsEnabled(false);

    setProfileForCurrentAircraft();

    menuItemId = PluginsMenu::getInstance()->addItem(
        classIdentifier(),
        std::vector<MenuItem>{
            {.name = "Identify", .content = [this](int menuId) {
                 setLedBrightness(ECAM32Led::BACKLIGHT, 128);
                 setLedBrightness(ECAM32Led::EMER_CANC_BRIGHTNESS, 128);
                 setLedBrightness(ECAM32Led::OVERALL_LEDS_BRIGHTNESS, 255);
                 setAllLedsEnabled(true);
                 AppState::getInstance()->executeAfter(2000, [this]() {
                     setAllLedsEnabled(false);
                 });
             }},
        });

    return true;
}

void ProductECAM32::disconnect() {
    setLedBrightness(ECAM32Led::BACKLIGHT, 0);
    setLedBrightness(ECAM32Led::EMER_CANC_BRIGHTNESS, 0);
    setLedBrightness(ECAM32Led::OVERALL_LEDS_BRIGHTNESS, 0);

    PluginsMenu::getInstance()->removeItem(menuItemId);

    if (profile) {
        delete profile;
        profile = nullptr;
    }

    USBDevice::disconnect();
}

void ProductECAM32::update() {
    if (!connected) {
        return;
    }

    USBDevice::update();
}

void ProductECAM32::setAllLedsEnabled(bool enable) {
    unsigned char start = static_cast<unsigned char>(ECAM32Led::_START);
    unsigned char end = static_cast<unsigned char>(ECAM32Led::_END);

    for (unsigned char i = start; i <= end; ++i) {
        ECAM32Led led = static_cast<ECAM32Led>(i);
        setLedBrightness(led, enable ? 1 : 0);
    }
}

void ProductECAM32::setLedBrightness(ECAM32Led led, uint8_t brightness) {
    writeData({0x02, ProductECAM32::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(led), brightness, 0x00, 0x00, 0x00, 0x00, 0x00});
}

void ProductECAM32::didReceiveData(int reportId, uint8_t *report, int reportLength) {
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

void ProductECAM32::didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count) {
    USBDevice::didReceiveButton(hardwareButtonIndex, pressed, count);

    auto &buttons = profile->buttonDefs();
    auto it = buttons.find(hardwareButtonIndex);
    if (it == buttons.end()) {
        return;
    }

    const ECAM32ButtonDef *buttonDef = &it->second;

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
