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
        /*def winwing_fcu_set_led(ep, led, brightness):
         if led.value < 100: # FCU
             data = [0x02, 0x10, 0xbb, 0, 0, 3, 0x49, led.value, brightness, 0,0,0,0,0]
         elif led.value < 200 and device_config & DEVICEMASK.EFISR: # EFIS_R
             data = [0x02, 0x0e, 0xbf, 0, 0, 3, 0x49, led.value - 100, brightness, 0,0,0,0,0]
         elif led.value >= 200 and led.value < 300 and device_config & DEVICEMASK.EFISL: # EFIS_L
             data = [0x02, 0x0d, 0xbf, 0, 0, 3, 0x49, led.value - 200, brightness, 0,0,0,0,0]
         if 'data' in locals():
           cmd = bytes(data)
           ep.write(cmd)


     def lcd_init(ep):
         data = [0xf0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0] # init packet
         cmd = bytes(data)
         ep.write(cmd)
         */
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
    // Initialize FCU display
    std::vector<uint8_t> initCmd = {0x10, 0x3c, 0x00, 0x00, 0x32, 0x00, 0x14, 0x00, 0x00, 0x00};
    writeData(initCmd);
    
    // Initialize EFIS Right display  
    initCmd = {0x10, 0x3d, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};
    writeData(initCmd);
    
    // Initialize EFIS Left display
    initCmd = {0x10, 0x3e, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};
    writeData(initCmd);
}

void ProductFCUEfis::sendFCUDisplay(const std::string& speed, const std::string& heading, 
                                   const std::string& altitude, const std::string& vs) {
    std::vector<uint8_t> data = {0x10, 0x3c};
    
    // Convert and add speed (5 digits)
    std::string speedStr = speed.substr(0, 5);
    speedStr.resize(5, ' ');
    for (char c : speedStr) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Convert and add heading (3 digits)  
    std::string headingStr = heading.substr(0, 3);
    headingStr.resize(3, ' ');
    for (char c : headingStr) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Convert and add altitude (5 digits)
    std::string altitudeStr = altitude.substr(0, 5);
    altitudeStr.resize(5, ' ');
    for (char c : altitudeStr) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Convert and add vertical speed (4 digits with +/- sign)
    std::string vsStr = vs;
    if (vsStr.length() > 0 && vsStr[0] != '+' && vsStr[0] != '-') {
        vsStr = "+" + vsStr;
    }
    vsStr = vsStr.substr(0, 5);
    vsStr.resize(5, ' ');
    for (char c : vsStr) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Pad to required length
    while (data.size() < 64) {
        data.push_back(0x00);
    }
    
    writeData(data);
}

void ProductFCUEfis::sendEfisRightDisplay(const std::string& baro) {
    std::vector<uint8_t> data = {0x10, 0x3d};
    
    // Convert and add barometric pressure (7 characters max)
    std::string baroStr = baro.substr(0, 7);
    baroStr.resize(7, ' ');
    for (char c : baroStr) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Pad to required length
    while (data.size() < 64) {
        data.push_back(0x00);
    }
    
    writeData(data);
}

void ProductFCUEfis::sendEfisLeftDisplay(const std::string& baro) {
    std::vector<uint8_t> data = {0x10, 0x3e};
    
    // Convert and add barometric pressure (7 characters max)
    std::string baroStr = baro.substr(0, 7);
    baroStr.resize(7, ' ');
    for (char c : baroStr) {
        data.push_back(static_cast<uint8_t>(c));
    }
    
    // Pad to required length
    while (data.size() < 64) {
        data.push_back(0x00);
    }
    
    writeData(data);
}

void ProductFCUEfis::setLedBrightness(FCUEfisLed led, uint8_t brightness) {
    std::vector<uint8_t> data = {0x20, static_cast<uint8_t>(led), brightness};
    
    // Pad to required length
    while (data.size() < 64) {
        data.push_back(0x00);
    }
    
    writeData(data);
}

void ProductFCUEfis::didReceiveData(int reportId, uint8_t *report, int reportLength) {
    if (!profile) {
        debug("[%s] No profile loaded, ignoring input\n", classIdentifier());
        return;
    }

    if (!report || reportLength <= 0) {
        debug("[%s] Invalid report data\n", classIdentifier());
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
