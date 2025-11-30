#include "product-agp.h"

#include "appstate.h"
#include "config.h"
#include "dataref.h"
#include "plugins-menu.h"
#include "profiles/toliss-agp-profile.h"
#include "segment-display.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <XPLMUtilities.h>

ProductAGP::ProductAGP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    lastButtonStateLo = 0;
    lastButtonStateHi = 0;
    pressedButtonIndices = {};

    connect();
}

ProductAGP::~ProductAGP() {
    disconnect();
}

const char *ProductAGP::classIdentifier() {
    return "AGP Metal";
}

void ProductAGP::setProfileForCurrentAircraft() {
    if (TolissAGPProfile::IsEligible()) {
        profile = new TolissAGPProfile(this);
        profileReady = true;
    } else {
        profile = nullptr;
        profileReady = false;
    }
}

bool ProductAGP::connect() {
    if (!USBDevice::connect()) {
        return false;
    }

    setLedBrightness(AGPLed::BACKLIGHT, 128);
    setLedBrightness(AGPLed::LCD_BRIGHTNESS, 128);
    setLedBrightness(AGPLed::OVERALL_LEDS_BRIGHTNESS, 255);
    setAllLedsEnabled(false);

    setProfileForCurrentAircraft();

    std::string terrainPreference = AppState::getInstance()->readPreference("AGPTerrainND", "first_officer");
    if (terrainPreference == "captain") {
        terrainNDPreference = AGPTerrainNDPreference::CAPTAIN;
    } else {
        terrainNDPreference = AGPTerrainNDPreference::FIRST_OFFICER;
    }

    menuItemId = PluginsMenu::getInstance()->addItem(
        classIdentifier(),
        std::vector<MenuItem>{
            {.name = "Identify", .content = [this](int menuId) {
                 setLedBrightness(AGPLed::BACKLIGHT, 128);
                 setLedBrightness(AGPLed::LCD_BRIGHTNESS, 255);
                 setLedBrightness(AGPLed::OVERALL_LEDS_BRIGHTNESS, 255);
                 setAllLedsEnabled(true);

                 AppState::getInstance()->executeAfter(2000, [this]() {
                     setAllLedsEnabled(false);
                 });
             }},
            {.name = "Terrain on ND", .content = std::vector<MenuItem>{
                                          {.name = "ND1 (Captain)", .checked = terrainPreference == "captain", .content = [this](int itemId) {
                                               AppState::getInstance()->writePreference("AGPTerrainND", "captain");
                                               terrainNDPreference = AGPTerrainNDPreference::CAPTAIN;
                                               PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                               PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                           }},
                                          {.name = "ND2 (First officer)", .checked = terrainPreference == "first_officer", .content = [this](int itemId) {
                                               AppState::getInstance()->writePreference("AGPTerrainND", "first_officer");
                                               terrainNDPreference = AGPTerrainNDPreference::FIRST_OFFICER;
                                               PluginsMenu::getInstance()->uncheckSubmenuSiblings(itemId);
                                               PluginsMenu::getInstance()->setItemChecked(itemId, true);
                                           }},
                                      }},
        });

    return true;
}

void ProductAGP::disconnect() {
    setLedBrightness(AGPLed::BACKLIGHT, 0);
    setLedBrightness(AGPLed::LCD_BRIGHTNESS, 0);
    setLedBrightness(AGPLed::OVERALL_LEDS_BRIGHTNESS, 0);

    PluginsMenu::getInstance()->removeItem(menuItemId);

    if (profile) {
        delete profile;
        profile = nullptr;
    }

    USBDevice::disconnect();
}

void ProductAGP::update() {
    if (!connected) {
        return;
    }

    USBDevice::update();

    if (++displayUpdateFrameCounter >= getDisplayUpdateFrameInterval()) {
        displayUpdateFrameCounter = 0;

        if (profile) {
            profile->updateDisplays();
        }
    }
}

void ProductAGP::setAllLedsEnabled(bool enable) {
    unsigned char start = static_cast<unsigned char>(AGPLed::_START);
    unsigned char end = static_cast<unsigned char>(AGPLed::_END);

    for (unsigned char i = start; i <= end; ++i) {
        AGPLed led = static_cast<AGPLed>(i);
        setLedBrightness(led, enable ? 1 : 0);
    }
}

void ProductAGP::setLedBrightness(AGPLed led, uint8_t brightness) {
    writeData({0x02, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(led), brightness, 0x00, 0x00, 0x00, 0x00, 0x00});
}

void ProductAGP::setLCDText(const std::string &chrono, const std::string &utcTime, const std::string &elapsedTime) {
    std::vector<uint8_t> packet = {
        0xF0, 0x00, packetNumber, 0x35, ProductAGP::IdentifierByte,
        0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
        0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    packet.resize(64, 0x00);

    // Row offsets are the starting bytes for segments A, B, C, D, E, F, G
    const int rowOffsets[7] = {25, 29, 33, 37, 41, 45, 49};

    std::string fullText = "";
    fullText += SegmentDisplay::fixStringLength(chrono, 4, ' ');      // Bits 0-3
    fullText += SegmentDisplay::fixStringLength(utcTime, 6, ' ');     // Bits 4-9
    fullText += SegmentDisplay::fixStringLength(elapsedTime, 4, ' '); // Bits 10-13

    for (int digitIndex = 0; digitIndex < 14; ++digitIndex) {
        char c = fullText[digitIndex];
        uint8_t charMask = SegmentDisplay::getAGPSegmentMask(c);

        // Iterate through the 7 segments (rows)
        for (int segIndex = 0; segIndex < 7; ++segIndex) {
            // Check if this segment (A..G) is active for this character
            if (charMask & (1 << segIndex)) {
                // Calculate Target Byte
                // If digitIndex < 8, use the base offset (Low Byte)
                // If digitIndex >= 8, use base offset + 1 (High Byte)
                int byteOffset = rowOffsets[segIndex] + (digitIndex / 8);

                // Calculate Bit Position (0-7)
                int bitPos = digitIndex % 8;

                // Set the bit
                packet[byteOffset] |= (1 << bitPos);
            }
        }
    }

    writeData(packet);

    std::vector<uint8_t> commitPacket = {
        0xF0, 0x00, packetNumber, 0x11, ProductAGP::IdentifierByte,
        0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    commitPacket.resize(64, 0x00);
    writeData(commitPacket);
    if (++packetNumber == 0) {
        packetNumber = 1;
    }
}

void ProductAGP::didReceiveData(int reportId, uint8_t *report, int reportLength) {
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

void ProductAGP::didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count) {
    USBDevice::didReceiveButton(hardwareButtonIndex, pressed, count);

    auto &buttons = profile->buttonDefs();
    auto it = buttons.find(hardwareButtonIndex);
    if (it == buttons.end()) {
        return;
    }

    const AGPButtonDef *buttonDef = &it->second;

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
