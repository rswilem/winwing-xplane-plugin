#include "product-fcu-efis.h"
#include "dataref.h"
#include "appstate.h"
#include "config.h"
#include "profiles/toliss-fcu-efis-profile.h"
#include <XPLMUtilities.h>
#include <XPLMProcessing.h>
#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

uint8_t swapNibbles(uint8_t value) {
    return ((value & 0x0F) << 4) | ((value & 0xF0) >> 4);
}

std::string fixStringLength(const std::string& value, int length) {
    std::string result = value;
    if (result.length() > length) {
        result = result.substr(0, length);
    }
    while (result.length() < length) {
        result = "0" + result;
    }
    return result;
}

std::vector<uint8_t> encodeString(int numSegments, const std::string& str) {
    std::vector<uint8_t> data(numSegments, 0);
    
    for (int i = 0; i < std::min(numSegments, static_cast<int>(str.length())); i++) {
        char c = std::toupper(str[i]);
        auto it = SEGMENT_REPRESENTATIONS.find(c);
        if (it != SEGMENT_REPRESENTATIONS.end()) {
            data[numSegments - 1 - i] = it->second;
        }
    }
    
    return data;
}

std::vector<uint8_t> encodeStringSwapped(int numSegments, const std::string& str) {
    std::vector<uint8_t> data = encodeString(numSegments, str);
    data.push_back(0);  // Add extra byte
    
    // Fix weird segment mapping
    for (int i = 0; i < data.size(); i++) {
        data[i] = swapNibbles(data[i]);
    }
    
    for (int i = 0; i < data.size() - 1; i++) {
        data[numSegments - i] = (data[numSegments - i] & 0x0F) | (data[numSegments - 1 - i] & 0xF0);
        data[numSegments - 1 - i] = data[numSegments - 1 - i] & 0x0F;
    }
    
    return data;
}

std::vector<uint8_t> encodeStringEfis(int numSegments, const std::string& str) {
    std::vector<uint8_t> data = encodeString(numSegments, str);
    std::vector<uint8_t> result(numSegments, 0);
    
    // Fix weird segment mapping for EFIS displays
    for (int i = 0; i < data.size(); i++) {
        result[i] |= (data[i] & 0x08) ? 0x01 : 0;  // Upper left -> bit 0
        result[i] |= (data[i] & 0x04) ? 0x02 : 0;  // Middle -> bit 1
        result[i] |= (data[i] & 0x02) ? 0x04 : 0;  // Lower left -> bit 2
        result[i] |= (data[i] & 0x10) ? 0x08 : 0;  // Bottom -> bit 3
        result[i] |= (data[i] & 0x80) ? 0x10 : 0;  // Top -> bit 4
        result[i] |= (data[i] & 0x40) ? 0x20 : 0;  // Upper right -> bit 5
        result[i] |= (data[i] & 0x20) ? 0x40 : 0;  // Lower right -> bit 6
        result[i] |= (data[i] & 0x01) ? 0x80 : 0;  // Dot -> bit 7
    }
    
    return result;
}

ProductFCUEfis::ProductFCUEfis(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) 
    : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    profile = nullptr;
    connect();
}

ProductFCUEfis::~ProductFCUEfis() {
    if (profile) {
        profile->ledBrightnessCallback = nullptr;
        delete profile;
        profile = nullptr;
    }
    disconnect();
}

void ProductFCUEfis::setProfileForCurrentAircraft() {
    if (TolissFCUEfisProfile::IsEligible()) {
        debug("Using Toliss FCU-EFIS profile for %s.\n", classIdentifier());
        profile = new TolissFCUEfisProfile();
        monitorDatarefs();
        profileReady = true;
    }
    else {
        debug("No eligible profiles found for %s. Has the aircraft finished loading?\n", classIdentifier());
        setLedBrightness(FCUEfisLed::FLAG_GREEN, 255);
    }
}

const char* ProductFCUEfis::classIdentifier() {
    return "Product-FCU-EFIS";
}

bool ProductFCUEfis::connect() {
    if (USBDevice::connect()) {
        initializeDisplays();
        
        // Set initial LED brightness
        setLedBrightness(FCUEfisLed::BACKLIGHT, 180);
        setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, 180);
        
        if (!profile) {
            setProfileForCurrentAircraft();
        }
        
        return true;
    }
    
    return false;
}

void ProductFCUEfis::disconnect() {
    // Turn off all LEDs
    setLedBrightness(FCUEfisLed::BACKLIGHT, 0);
    setLedBrightness(FCUEfisLed::SCREEN_BACKLIGHT, 0);
    setLedBrightness(FCUEfisLed::AP1_GREEN, 0);
    setLedBrightness(FCUEfisLed::AP2_GREEN, 0);
    setLedBrightness(FCUEfisLed::ATHR_GREEN, 0);
    setLedBrightness(FCUEfisLed::LOC_GREEN, 0);
    setLedBrightness(FCUEfisLed::APPR_GREEN, 0);
    setLedBrightness(FCUEfisLed::EXPED_GREEN, 0);
    
    if (profile) {
        profile->ledBrightnessCallback = nullptr;
        delete profile;
        profile = nullptr;
    }
    
    USBDevice::disconnect();
    cachedDatarefValues.clear();
}

void ProductFCUEfis::update() {
    if (!connected) {
        return;
    }
    
    if (!profile) {
        setProfileForCurrentAircraft();
        return;
    }
    
    USBDevice::update();
    updateDisplays();
}

void ProductFCUEfis::updateDisplays() {
    if (!profile) {
        return;
    }
    
    bool anyDatarefChanged = false;
    const std::vector<std::string>& currentDatarefs = profile->displayDatarefs();
    for (const std::string &ref : currentDatarefs) {
        std::string newValue = Dataref::getInstance()->getCached<std::string>(ref.c_str());
        auto it = cachedDatarefValues.find(ref);
        if (it == cachedDatarefValues.end() || it->second != newValue) {
            cachedDatarefValues[ref] = newValue;
            anyDatarefChanged = true;
        }
    }

    if (!anyDatarefChanged) {
        return;
    }

    FCUDisplayData newDisplayData = displayData;
    profile->updateDisplayData(newDisplayData, cachedDatarefValues);
    
    // Update FCU display if data changed
    if (newDisplayData.speed != displayData.speed || 
        newDisplayData.heading != displayData.heading ||
        newDisplayData.altitude != displayData.altitude ||
        newDisplayData.verticalSpeed != displayData.verticalSpeed ||
        newDisplayData.spdMach != displayData.spdMach ||
        newDisplayData.hdgTrk != displayData.hdgTrk ||
        newDisplayData.altManaged != displayData.altManaged ||
        newDisplayData.spdManaged != displayData.spdManaged ||
        newDisplayData.hdgManaged != displayData.hdgManaged ||
        newDisplayData.vsMode != displayData.vsMode ||
        newDisplayData.fpaMode != displayData.fpaMode) {
        
        sendFCUDisplay(newDisplayData.speed, newDisplayData.heading, 
                      newDisplayData.altitude, newDisplayData.verticalSpeed);
    }
    
    // Update EFIS Right display if data changed
    if (profile->hasEfisRight() && 
        (newDisplayData.efisRBaro != displayData.efisRBaro ||
         newDisplayData.efisRQnh != displayData.efisRQnh ||
         newDisplayData.efisRHpaDec != displayData.efisRHpaDec)) {
        sendEfisRightDisplay(newDisplayData.efisRBaro);
    }
    
    // Update EFIS Left display if data changed
    if (profile->hasEfisLeft() && 
        (newDisplayData.efisLBaro != displayData.efisLBaro ||
         newDisplayData.efisLQnh != displayData.efisLQnh ||
         newDisplayData.efisLHpaDec != displayData.efisLHpaDec)) {
        sendEfisLeftDisplay(newDisplayData.efisLBaro);
    }
    
    displayData = newDisplayData;
}

void ProductFCUEfis::initializeDisplays() {
    // Initialize displays with proper init sequence
    std::vector<uint8_t> initCmd = {
        0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    writeData(initCmd);
}

void ProductFCUEfis::sendFCUDisplay(const std::string& speed, const std::string& heading, 
                                   const std::string& altitude, const std::string& vs) {
    // Encode strings to 7-segment data
    auto speedData = encodeString(3, fixStringLength(speed, 3));
    auto headingData = encodeStringSwapped(3, fixStringLength(heading, 3));
    auto altitudeData = encodeStringSwapped(5, fixStringLength(altitude, 5));
    auto vsData = encodeStringSwapped(4, fixStringLength(vs, 4));
    
    // Create flag bytes array
    std::vector<uint8_t> flagBytes(17, 0);
    
    // Set flags based on display data
    if (displayData.spdMach) flagBytes[static_cast<int>(DisplayByteIndex::H3)] |= 0x04;
    if (displayData.spdManaged) flagBytes[static_cast<int>(DisplayByteIndex::H3)] |= 0x02;
    if (!displayData.spdMach) flagBytes[static_cast<int>(DisplayByteIndex::H3)] |= 0x08; // SPD
    
    if (displayData.hdgTrk) flagBytes[static_cast<int>(DisplayByteIndex::H0)] |= 0x40; // TRK
    else flagBytes[static_cast<int>(DisplayByteIndex::H0)] |= 0x80; // HDG
    if (displayData.hdgManaged) flagBytes[static_cast<int>(DisplayByteIndex::H0)] |= 0x10;
    if (displayData.latMode) flagBytes[static_cast<int>(DisplayByteIndex::H0)] |= 0x20; // LAT
    
    if (displayData.altIndication) flagBytes[static_cast<int>(DisplayByteIndex::A4)] |= 0x10; // ALT
    if (displayData.altManaged) flagBytes[static_cast<int>(DisplayByteIndex::V1)] |= 0x10;
    
    if (displayData.vsMode) flagBytes[static_cast<int>(DisplayByteIndex::A5)] |= 0x04; // V/S
    if (displayData.fpaMode) flagBytes[static_cast<int>(DisplayByteIndex::A5)] |= 0x01; // FPA
    if (displayData.hdgTrk) flagBytes[static_cast<int>(DisplayByteIndex::A5)] |= 0x02; // TRK
    if (!displayData.hdgTrk) flagBytes[static_cast<int>(DisplayByteIndex::A5)] |= 0x08; // HDG
    
    if (displayData.vsHorizontalLine) flagBytes[static_cast<int>(DisplayByteIndex::A0)] |= 0x10;
    if (displayData.vsVerticalLine) flagBytes[static_cast<int>(DisplayByteIndex::V2)] |= 0x10;
    if (displayData.lvlChange) flagBytes[static_cast<int>(DisplayByteIndex::A2)] |= 0x10;
    if (displayData.lvlChangeLeft) flagBytes[static_cast<int>(DisplayByteIndex::A3)] |= 0x10;
    if (displayData.lvlChangeRight) flagBytes[static_cast<int>(DisplayByteIndex::A1)] |= 0x10;
    
    if (displayData.vsIndication) flagBytes[static_cast<int>(DisplayByteIndex::V0)] |= 0x40;
    if (displayData.fpaIndication) flagBytes[static_cast<int>(DisplayByteIndex::V0)] |= 0x80;
    if (displayData.fpaComma) flagBytes[static_cast<int>(DisplayByteIndex::V3)] |= 0x10;
    if (displayData.machComma) flagBytes[static_cast<int>(DisplayByteIndex::S1)] |= 0x01;
    
    static uint8_t packageNumber = 1;
    
    // First request - send display data
    std::vector<uint8_t> data1 = {
        0xF0, 0x00, packageNumber, 0x31, 0x10, 0xBB, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0x00,
        0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    // Add speed data (3 bytes)
    data1.push_back(speedData[2]);
    data1.push_back(speedData[1] | flagBytes[static_cast<int>(DisplayByteIndex::S1)]);
    data1.push_back(speedData[0]);
    
    // Add heading data (4 bytes)
    data1.push_back(headingData[3] | flagBytes[static_cast<int>(DisplayByteIndex::H3)]);
    data1.push_back(headingData[2]);
    data1.push_back(headingData[1]);
    data1.push_back(headingData[0] | flagBytes[static_cast<int>(DisplayByteIndex::H0)]);
    
    // Add altitude data (6 bytes)
    data1.push_back(altitudeData[5] | flagBytes[static_cast<int>(DisplayByteIndex::A5)]);
    data1.push_back(altitudeData[4] | flagBytes[static_cast<int>(DisplayByteIndex::A4)]);
    data1.push_back(altitudeData[3] | flagBytes[static_cast<int>(DisplayByteIndex::A3)]);
    data1.push_back(altitudeData[2] | flagBytes[static_cast<int>(DisplayByteIndex::A2)]);
    data1.push_back(altitudeData[1] | flagBytes[static_cast<int>(DisplayByteIndex::A1)]);
    data1.push_back(altitudeData[0] | vsData[4] | flagBytes[static_cast<int>(DisplayByteIndex::A0)]);
    
    // Add vertical speed data (4 bytes)
    data1.push_back(vsData[3] | flagBytes[static_cast<int>(DisplayByteIndex::V3)]);
    data1.push_back(vsData[2] | flagBytes[static_cast<int>(DisplayByteIndex::V2)]);
    data1.push_back(vsData[1] | flagBytes[static_cast<int>(DisplayByteIndex::V1)]);
    data1.push_back(vsData[0] | flagBytes[static_cast<int>(DisplayByteIndex::V0)]);
    
    // Pad to 64 bytes
    while (data1.size() < 64) {
        data1.push_back(0x00);
    }
    
    writeData(data1);
    
    // Second request - commit display data
    std::vector<uint8_t> data2 = {
        0xF0, 0x00, packageNumber, 0x11, 0x10, 0xBB, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0x00
    };
    
    // Pad to 64 bytes
    while (data2.size() < 64) {
        data2.push_back(0x00);
    }
    
    writeData(data2);
    
    // Increment package number for next call
    packageNumber++;
    if (packageNumber == 0) packageNumber = 1;  // Avoid 0
}

void ProductFCUEfis::sendEfisRightDisplay(const std::string& baro) {
    auto baroData = encodeStringEfis(4, fixStringLength(baro, 4));
    
    // Create flag bytes array
    std::vector<uint8_t> flagBytes(17, 0);
    
    // Set EFIS Right flags
    if (displayData.efisRQnh) flagBytes[static_cast<int>(DisplayByteIndex::EFISR_B0)] |= 0x02;
    if (displayData.efisRQfe) flagBytes[static_cast<int>(DisplayByteIndex::EFISR_B0)] |= 0x01;
    if (displayData.efisRHpaDec) flagBytes[static_cast<int>(DisplayByteIndex::EFISR_B2)] |= 0x80;
    
    static uint8_t packageNumber = 1;
    
    // EFIS Right display protocol
    std::vector<uint8_t> data = {
        0xF0, 0x00, packageNumber, 0x1A, 0x0E, 0xBF, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x1D, 0x00,
        0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    // Add barometric data
    data.push_back(baroData[3]);
    data.push_back(baroData[2] | flagBytes[static_cast<int>(DisplayByteIndex::EFISR_B2)]);
    data.push_back(baroData[1]);
    data.push_back(baroData[0]);
    data.push_back(flagBytes[static_cast<int>(DisplayByteIndex::EFISR_B0)]);
    
    // Add second command
    data.insert(data.end(), {0x0E, 0xBF, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x4C, 0x0C, 0x1D});
    
    // Pad to 64 bytes
    while (data.size() < 64) {
        data.push_back(0x00);
    }
    
    writeData(data);
    
    // Increment package number for next call
    packageNumber++;
    if (packageNumber == 0) packageNumber = 1;  // Avoid 0
}

void ProductFCUEfis::sendEfisLeftDisplay(const std::string& baro) {
    auto baroData = encodeStringEfis(4, fixStringLength(baro, 4));
    
    // Create flag bytes array
    std::vector<uint8_t> flagBytes(17, 0);
    
    // Set EFIS Left flags
    if (displayData.efisLQnh) flagBytes[static_cast<int>(DisplayByteIndex::EFISL_B0)] |= 0x02;
    if (displayData.efisLQfe) flagBytes[static_cast<int>(DisplayByteIndex::EFISL_B0)] |= 0x01;
    if (displayData.efisLHpaDec) flagBytes[static_cast<int>(DisplayByteIndex::EFISL_B2)] |= 0x80;
    
    static uint8_t packageNumber = 1;
    
    // EFIS Left display protocol
    std::vector<uint8_t> data = {
        0xF0, 0x00, packageNumber, 0x1A, 0x0D, 0xBF, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x1D, 0x00,
        0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    // Add barometric data
    data.push_back(baroData[3]);
    data.push_back(baroData[2] | flagBytes[static_cast<int>(DisplayByteIndex::EFISL_B2)]);
    data.push_back(baroData[1]);
    data.push_back(baroData[0]);
    data.push_back(flagBytes[static_cast<int>(DisplayByteIndex::EFISL_B0)]);
    
    // Add second command
    data.insert(data.end(), {0x0E, 0xBF, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x4C, 0x0C, 0x1D});
    
    // Pad to 64 bytes
    while (data.size() < 64) {
        data.push_back(0x00);
    }
    
    writeData(data);
    
    // Increment package number for next call
    packageNumber++;
    if (packageNumber == 0) packageNumber = 1;  // Avoid 0
}

void ProductFCUEfis::setLedBrightness(FCUEfisLed led, uint8_t brightness) {
    std::vector<uint8_t> data;
    
    int ledValue = static_cast<int>(led);
    
    if (ledValue < 100) {
        // FCU LEDs
        data = {0x02, 0x10, 0xBB, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(ledValue), brightness, 0x00, 0x00, 0x00, 0x00, 0x00};
    } else if (ledValue < 200) {
        // EFIS Right LEDs
        data = {0x02, 0x0E, 0xBF, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(ledValue - 100), brightness, 0x00, 0x00, 0x00, 0x00, 0x00};
    } else if (ledValue < 300) {
        // EFIS Left LEDs
        data = {0x02, 0x0D, 0xBF, 0x00, 0x00, 0x03, 0x49, static_cast<uint8_t>(ledValue - 200), brightness, 0x00, 0x00, 0x00, 0x00, 0x00};
    }
    
    if (!data.empty()) {
        writeData(data);
    }
}

void ProductFCUEfis::didReceiveData(int reportId, uint8_t *report, int reportLength) {
    if (!connected || !profile || !report || reportLength <= 0) {
        return;
    }
    
    if (reportId != 1 || reportLength < 13) {
#if DEBUG
        printf("[%s] Ignoring reportId %d, length %d\n", classIdentifier(), reportId, reportLength);
        printf("[%s] Data (hex): ", classIdentifier());
        for (int i = 0; i < reportLength; ++i) {
            printf("%02X ", report[i]);
        }
        printf("\n");
#endif
        return;
    }
    
    // Parse button press data - format is specific to FCU-EFIS hardware
    // Bytes 0-11 contain button states for 96 buttons (8 bits per byte)
    for (int byteIndex = 0; byteIndex < std::min(12, reportLength); byteIndex++) {
        uint8_t buttonByte = report[byteIndex];
        
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            int buttonIndex = byteIndex * 8 + bitIndex;
            bool isPressed = (buttonByte & (1 << bitIndex)) != 0;
            
            auto it = pressedButtonIndices.find(buttonIndex);
            bool wasPressed = (it != pressedButtonIndices.end());
            
            if (isPressed && !wasPressed) {
                // Button pressed
                pressedButtonIndices.insert(buttonIndex);
                
                // Find button definition by ID
                const auto& buttons = profile->buttonDefs();
                auto buttonIt = std::find_if(buttons.begin(), buttons.end(), 
                    [buttonIndex](const FCUEfisButtonDef& btn) { return btn.id == buttonIndex; });
                
                if (buttonIt != buttons.end() && !buttonIt->dataref.empty()) {
                    if (buttonIt->value >= 0) {
                        // Set specific dataref value
                        Dataref::getInstance()->set<int>(buttonIt->dataref.c_str(), buttonIt->value);
                        debug("[%s] Button pressed: %d - Set %s = %d\n", classIdentifier(), buttonIndex, buttonIt->dataref.c_str(), buttonIt->value);
                    } else {
                        // Execute command
                        Dataref::getInstance()->executeCommand(buttonIt->dataref.c_str(), xplm_CommandBegin);
                        debug("[%s] Button pressed: %d - %s\n", classIdentifier(), buttonIndex, buttonIt->dataref.c_str());
                    }
                }
            } else if (isPressed && wasPressed) {
                // Button held - only for commands, not for value-setting buttons
                const auto& buttons = profile->buttonDefs();
                auto buttonIt = std::find_if(buttons.begin(), buttons.end(), 
                    [buttonIndex](const FCUEfisButtonDef& btn) { return btn.id == buttonIndex; });
                
                if (buttonIt != buttons.end() && !buttonIt->dataref.empty() && buttonIt->value < 0) {
                    Dataref::getInstance()->executeCommand(buttonIt->dataref.c_str(), xplm_CommandContinue);
                }
            } else if (!isPressed && wasPressed) {
                // Button released - only for commands, not for value-setting buttons
                pressedButtonIndices.erase(buttonIndex);
                
                const auto& buttons = profile->buttonDefs();
                auto buttonIt = std::find_if(buttons.begin(), buttons.end(), 
                    [buttonIndex](const FCUEfisButtonDef& btn) { return btn.id == buttonIndex; });
                
                if (buttonIt != buttons.end() && !buttonIt->dataref.empty() && buttonIt->value < 0) {
                    Dataref::getInstance()->executeCommand(buttonIt->dataref.c_str(), xplm_CommandEnd);
                }
            }
        }
    }
}

void ProductFCUEfis::monitorDatarefs() {
    if (!profile) {
        return;
    }
    
    // Set up LED brightness callback
    profile->ledBrightnessCallback = [this](FCUEfisLed led, uint8_t brightness) {
        setLedBrightness(led, brightness);
    };
    
    // The profile handles dataref monitoring internally via monitorExistingDataref
    // No need to manually subscribe to datarefs here
}
