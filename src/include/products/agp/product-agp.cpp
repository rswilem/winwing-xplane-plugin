#include "product-agp.h"

#include "appstate.h"
#include "config.h"
#include "dataref.h"
#include "packet-utils.h"
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
            {.name = "Text: 88:88", .content = [this](int menuId) {
                 setLCDText("8888", "888888", "8888");
             }},
            {.name = "Text: 12:34", .content = [this](int menuId) {
                 setLCDText("1234", "567890", "test");
             }},
            //            {.name = "Test 15:46:16", .content = [this](int menuId) {
            //                 //15:46:16, no CHR
            //                 //f0 00 00 35 80 bb 00 00 02 01 00 00 ff ff 00 00 00 24 00 00 00 00 00 00 00 a0 02 00 00 50 01 00 00 f0 03 00 00 a0 02 00 00 80 02 00 00 e0 02 00 00 e0 02 00 00 e0 01 00 00 00 00 00 00 00 00 00
            //                 std::vector<uint8_t> data1 = {
            //                     0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00,
            //                     0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x02, 0x00, 0x00, 0x50, 0x01, 0x00,
            //                     0x00, 0xF0, 0x03, 0x00, 0x00, 0xA0, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0xE0, 0x02, 0x00,
            //                     0x00, 0xE0, 0x02, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //                 writeData(data1);
            //
            //                 std::vector<uint8_t> data2 = {
            //                     0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00};
            //
            //                 while (data2.size() < 64) {
            //                     data2.push_back(0x00);
            //                 }
            //
            //                 writeData(data2);
            //             }},
            //            {.name = "Test 15:47:08  CHR 00:03", .content = [this](int menuId) {
            //                 //15:47:08 (CHR 00:03)
            //                 //f0 00 4d 35 80 bb 00 00 02 01 00 00 c3 87 02 00 00 24 00 00 00 00 00 00 00 af 03 00 00 df 03 00 00 ff 03 00 00 2f 03 00 00 07 03 00 00 67 03 00 00 68 02 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //                 std::vector<uint8_t>
            //                     data1 = {
            //                         0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //                         0xFF, 0xFF, 0x00, 0x00,
            //                         0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x03, 0x00, 0x00, 0xDF, 0x03, 0x00,
            //                         0x00, 0xFF, 0x03, 0x00, 0x00, 0x2F, 0x03, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x67, 0x03, 0x00,
            //                         0x00, 0x68, 0x02, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //                 writeData(data1);
            //
            //                 std::vector<uint8_t> data2 = {
            //                     0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00};
            //
            //                 while (data2.size() < 64) {
            //                     data2.push_back(0x00);
            //                 }
            //
            //                 writeData(data2);
            //             }},
            //            {.name = "Test 15:47:10 CHR 00:05", .content = [this](int menuId) {
            //                 //
            //                 //15:47:10 (CHR 00:05)
            //                 //f0 00 6d 35 80 bb 00 00 02 01 00 00 6d 8e 02 00 00 24 00 00 00 00 00 00 00 af 02 00 00 d7 03 00 00 ff 03 00 00 2f 02 00 00 07 02 00 00 6f 02 00 00 68 00 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //                 std::vector<uint8_t> data1 = {
            //                     0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00,
            //                     0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x02, 0x00, 0x00, 0xD7, 0x03, 0x00,
            //                     0x00, 0xFF, 0x03, 0x00, 0x00, 0x2F, 0x02, 0x00, 0x00, 0x07, 0x02, 0x00, 0x00, 0x6F, 0x02, 0x00,
            //                     0x00, 0x68, 0x00, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //                 writeData(data1);
            //
            //                 std::vector<uint8_t> data2 = {
            //                     0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00};
            //
            //                 while (data2.size() < 64) {
            //                     data2.push_back(0x00);
            //                 }
            //
            //                 writeData(data2);
            //             }},
            //            {.name = "Test 15:47:06 CHR 00:01", .content = [this](int menuId) {
            //                 //
            //                 //15:47:06 (CHR 00:01)
            //                 //f0 00 1b 35 80 bb 00 00 02 01 00 00 69 7d 02 00 00 24 00 00 00 00 00 00 00 a7 03 00 00 df 01 00 00 ff 03 00 00 27 03 00 00 07 03 00 00 67 03 00 00 60 02 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //                 std::vector<uint8_t> data1 = {
            //                     0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00,
            //                     0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7, 0x03, 0x00, 0x00, 0xDF, 0x01, 0x00,
            //                     0x00, 0xFF, 0x03, 0x00, 0x00, 0x27, 0x03, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x67, 0x03, 0x00,
            //                     0x00, 0x60, 0x02, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //                 writeData(data1);
            //
            //                 std::vector<uint8_t> data2 = {
            //                     0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00};
            //
            //                 while (data2.size() < 64) {
            //                     data2.push_back(0x00);
            //                 }
            //
            //                 writeData(data2);
            //             }},
            //            {.name = "Test 15:47:11 CHR 00:06", .content = [this](int menuId) {
            //                 //
            //                 //15:47:11 (CHR 00:06)
            //                 //f0 00 7b 35 80 bb 00 00 02 01 00 00 57 91 02 00 00 24 00 00 00 00 00 00 00 af 00 00 00 d7 03 00 00 ff 03 00 00 2f 00 00 00 0f 00 00 00 6f 00 00 00 68 00 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //                 std::vector<uint8_t> data1 = {
            //                     0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00,
            //                     0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x00, 0x00, 0x00, 0xD7, 0x03, 0x00,
            //                     0x00, 0xFF, 0x03, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x6F, 0x00, 0x00,
            //                     0x00, 0x68, 0x00, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //                 writeData(data1);
            //
            //                 std::vector<uint8_t> data2 = {
            //                     0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //                     0xFF, 0xFF, 0x00, 0x00};
            //
            //                 while (data2.size() < 64) {
            //                     data2.push_back(0x00);
            //                 }
            //
            //                 writeData(data2);
            //             }},
            // {.name = "Test 15:47:09 CHR 00:04", .content = [this](int menuId) {
            //      //
            //      //f0 00 61 35 80 bb 00 00 02 01 00 00 f2 8b 02 00 00 24 00 00 00 00 00 00 00 a7 03 00 00 df 03 00 00 ff 03 00 00 27 03 00 00 07 01 00 00 6f 03 00 00 68 02 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //      std::vector<uint8_t> data1 = {
            //          0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00,
            //          0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7, 0x03, 0x00, 0x00, 0xDF, 0x03, 0x00,
            //          0x00, 0xFF, 0x03, 0x00, 0x00, 0x27, 0x03, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00, 0x6F, 0x03, 0x00,
            //          0x00, 0x68, 0x02, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //      writeData(data1);

            //      std::vector<uint8_t> data2 = {
            //          0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00};

            //      while (data2.size() < 64) {
            //          data2.push_back(0x00);
            //      }

            //      writeData(data2);
            //  }},
            // {.name = "Test 15:47:09 CHR 00:04", .content = [this](int menuId) {
            //      //
            //      //f0 00 5f 35 80 bb 00 00 02 01 00 00 86 8b 02 00 00 24 00 00 00 00 00 00 00 a7 03 00 00 df 03 00 00 ff 03 00 00 27 03 00 00 07 01 00 00 6f 03 00 00 68 02 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //      std::vector<uint8_t> data1 = {
            //          0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00,
            //          0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7, 0x03, 0x00, 0x00, 0xDF, 0x03, 0x00,
            //          0x00, 0xFF, 0x03, 0x00, 0x00, 0x27, 0x03, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00, 0x6F, 0x03, 0x00,
            //          0x00, 0x68, 0x02, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //      writeData(data1);

            //      std::vector<uint8_t> data2 = {
            //          0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00};

            //      while (data2.size() < 64) {
            //          data2.push_back(0x00);
            //      }

            //      writeData(data2);
            //  }},
            // {.name = "Test 15:47:06 CHR 00:01", .content = [this](int menuId) {
            //      //
            //      //f0 00 27 35 80 bb 00 00 02 01 00 00 e6 7f 02 00 00 24 00 00 00 00 00 00 00 a7 03 00 00 df 01 00 00 ff 03 00 00 27 03 00 00 07 03 00 00 67 03 00 00 60 02 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //      std::vector<uint8_t> data1 = {
            //          0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00,
            //          0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7, 0x03, 0x00, 0x00, 0xDF, 0x01, 0x00,
            //          0x00, 0xFF, 0x03, 0x00, 0x00, 0x27, 0x03, 0x00, 0x00, 0x07, 0x03, 0x00, 0x00, 0x67, 0x03, 0x00,
            //          0x00, 0x60, 0x02, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //      writeData(data1);

            //      std::vector<uint8_t> data2 = {
            //          0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00};

            //      while (data2.size() < 64) {
            //          data2.push_back(0x00);
            //      }

            //      writeData(data2);
            //  }},
            // {.name = "Test 15:47:07 CHR 00:02", .content = [this](int menuId) {
            //      //
            //      //f0 00 35 35 80 bb 00 00 02 01 00 00 c3 82 02 00 00 24 00 00 00 00 00 00 00 af 03 00 00 df 03 00 00 f7 03 00 00 2f 01 00 00 0f 01 00 00 67 01 00 00 68 00 00 00 ec 01 00 00 00 00 00 00 00 00 00
            //      std::vector<uint8_t> data1 = {
            //          0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00,
            //          0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x03, 0x00, 0x00, 0xDF, 0x03, 0x00,
            //          0x00, 0xF7, 0x03, 0x00, 0x00, 0x2F, 0x01, 0x00, 0x00, 0x0F, 0x01, 0x00, 0x00, 0x67, 0x01, 0x00,
            //          0x00, 0x68, 0x00, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            //      writeData(data1);

            //      std::vector<uint8_t> data2 = {
            //          0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
            //          0xFF, 0xFF, 0x00, 0x00};

            //      while (data2.size() < 64) {
            //          data2.push_back(0x00);
            //      }

            //      writeData(data2);
            //  }},
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
        0xF0, 0x00, packetNumber.next(), 0x35, ProductAGP::IdentifierByte,
        0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
        0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    packet.resize(64, 0x00);

    // Row offsets are the starting bytes for segments A, B, C, D, E, F, G
    const int rowOffsets[7] = {25, 29, 33, 37, 41, 45, 49};

    std::string fullText = "";
    fullText += SegmentDisplay::fixStringLength(chrono, 4);      // Bits 0-3
    fullText += SegmentDisplay::fixStringLength(utcTime, 6);     // Bits 4-9
    fullText += SegmentDisplay::fixStringLength(elapsedTime, 4); // Bits 10-13

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
        0xF0, 0x00, packetNumber.current(), 0x11, ProductAGP::IdentifierByte,
        0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    commitPacket.resize(64, 0x00);
    writeData(commitPacket);
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

    if (pressed) {
        debug_force("AGP button pressed: %i\n", hardwareButtonIndex);
    }

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
