#include "bridge.h"
#include "usbcontroller.h"
#include "dataref.h"
#include "product-ursa-minor-joystick.h"
#include "product-mcdu.h"
#include "product-pfp.h"
#include <vector>
#include <string>
#include <cstring>

void clearDatarefCache() {
    Dataref::getInstance()->clearCache();
}

void setDatarefHexC(const char* ref, const uint8_t* hexD, int len) {
    const std::vector<uint8_t>& hex = std::vector<uint8_t>(hexD, hexD + len);
    std::string s;
    for (uint8_t c : hex) {
        if (c == 0x00) break;
        s += static_cast<char>(c);
    }
    
    Dataref::getInstance()->set<std::string>(ref, s, true);
}

void update() {
    for (const auto& device : USBController::getInstance()->devices) {
        if (device->connected) {
            device->update();
        }
    }
    
    //Dataref::getInstance()->update();
}

void disconnectAll() {
    for (const auto& device : USBController::getInstance()->devices) {
        device->disconnect();
    }
}

int enumerateDevices(char *buffer, int bufferLen) {
    //USBController::getInstance()->reloadDevices();
    
    int count = 0;
    std::string result;
    for (const auto& device : USBController::getInstance()->devices) {
        if (!result.empty()) result += "\n";
        result += device->productName;
        count++;
    }
    if ((int)result.size() + 1 > bufferLen) {
        // Not enough space in buffer
        return -1;
    }
    std::strncpy(buffer, result.c_str(), bufferLen);
    buffer[bufferLen-1] = '\0';
    return count;
}

// Device handle access functions
void* getDeviceHandle(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    return devices[deviceIndex];
}

void* getJoystickHandle(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    return dynamic_cast<ProductUrsaMinorJoystick*>(devices[deviceIndex]);
}

void* getMCDUHandle(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    return dynamic_cast<ProductMCDU*>(devices[deviceIndex]);
}

void* getPFPHandle(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    return dynamic_cast<ProductPFP*>(devices[deviceIndex]);
}

// Generic device functions via handle
bool device_connect(void* deviceHandle) {
    if (!deviceHandle) return false;
    auto device = static_cast<USBDevice*>(deviceHandle);
    return device->connect();
}

void device_disconnect(void* deviceHandle) {
    if (!deviceHandle) return;
    auto device = static_cast<USBDevice*>(deviceHandle);
    device->disconnect();
}

void device_update(void* deviceHandle) {
    if (!deviceHandle) return;
    auto device = static_cast<USBDevice*>(deviceHandle);
    device->update();
}

// Joystick functions via handle
bool joystick_setVibration(void* joystickHandle, uint8_t vibration) {
    if (!joystickHandle) return false;
    auto joystick = static_cast<ProductUrsaMinorJoystick*>(joystickHandle);
    return joystick->setVibration(vibration);
}

bool joystick_setLedBrightness(void* joystickHandle, uint8_t brightness) {
    if (!joystickHandle) return false;
    auto joystick = static_cast<ProductUrsaMinorJoystick*>(joystickHandle);
    return joystick->setLedBrightness(brightness);
}

// MCDU functions via handle
void mcdu_clear2(void* mcduHandle, int displayId) {
    if (!mcduHandle) return;
    auto mcdu = static_cast<ProductMCDU*>(mcduHandle);
    mcdu->clear2(displayId);
}

bool mcdu_setLed(void* mcduHandle, int ledId, uint8_t value) {
    if (!mcduHandle) return false;
    auto mcdu = static_cast<ProductMCDU*>(mcduHandle);
    mcdu->setLedBrightness(static_cast<MCDULed>(ledId), value);
    return true;
}

// Additional MCDU functions via handle
void mcdu_clear(void* mcduHandle) {
    if (!mcduHandle) return;
    auto mcdu = static_cast<ProductMCDU*>(mcduHandle);
    mcdu->clear();
}

void mcdu_setLedBrightness(void* mcduHandle, int ledId, uint8_t brightness) {
    if (!mcduHandle) return;
    auto mcdu = static_cast<ProductMCDU*>(mcduHandle);
    mcdu->setLedBrightness(static_cast<MCDULed>(ledId), brightness);
}

// PFP functions via handle
void pfp_clear2(void* pfpHandle, int displayId) {
    if (!pfpHandle) return;
    auto pfp = static_cast<ProductPFP*>(pfpHandle);
    pfp->clear2(displayId);
}

bool pfp_setLed(void* pfpHandle, int ledId, uint8_t value) {
    if (!pfpHandle) return false;
    auto pfp = static_cast<ProductPFP*>(pfpHandle);
    pfp->setLedBrightness(static_cast<PFPLed>(ledId), value);
    return true;
}

void pfp_clear(void* pfpHandle) {
    if (!pfpHandle) return;
    auto pfp = static_cast<ProductPFP*>(pfpHandle);
    pfp->clear();
}

void pfp_setLedBrightness(void* pfpHandle, int ledId, uint8_t brightness) {
    if (!pfpHandle) return;
    auto pfp = static_cast<ProductPFP*>(pfpHandle);
    pfp->setLedBrightness(static_cast<PFPLed>(ledId), brightness);
}

// Device enumeration and info functions
int getDeviceCount() {
    return static_cast<int>(USBController::getInstance()->devices.size());
}

const char* getDeviceName(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    return devices[deviceIndex]->productName.c_str();
}

const char* getDeviceType(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    
    auto device = devices[deviceIndex];
    if (dynamic_cast<ProductUrsaMinorJoystick*>(device)) {
        return "joystick";
    } else if (dynamic_cast<ProductMCDU*>(device)) {
        return "mcdu";
    } else if (dynamic_cast<ProductPFP*>(device)) {
        return "pfp";
    }
    return "unknown";
}

uint16_t getDeviceProductId(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return 0;
    }
    return devices[deviceIndex]->productId;
}

bool isDeviceConnected(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    return devices[deviceIndex]->connected;
}

// Joystick-specific functions
bool joystick_setVibration(int deviceIndex, uint8_t vibration) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    
    auto joystick = dynamic_cast<ProductUrsaMinorJoystick*>(devices[deviceIndex]);
    if (joystick) {
        return joystick->setVibration(vibration);
    }
    return false;
}

bool joystick_setLedBrightness(int deviceIndex, uint8_t brightness) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    
    auto joystick = dynamic_cast<ProductUrsaMinorJoystick*>(devices[deviceIndex]);
    if (joystick) {
        return joystick->setLedBrightness(brightness);
    }
    return false;
}

// MCDU-specific functions
bool mcdu_clearDisplay(int deviceIndex, int displayId) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    
    auto mcdu = dynamic_cast<ProductMCDU*>(devices[deviceIndex]);
    if (mcdu) {
        mcdu->clear2(displayId);
        return true;
    }
    return false;
}

bool mcdu_setBacklight(int deviceIndex, uint8_t brightness) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    
    auto mcdu = dynamic_cast<ProductMCDU*>(devices[deviceIndex]);
    if (mcdu) {
        mcdu->setLedBrightness(MCDULed::BACKLIGHT, brightness);
        return true;
    }
    return false;
}

bool mcdu_setScreenBacklight(int deviceIndex, uint8_t brightness) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    
    auto mcdu = dynamic_cast<ProductMCDU*>(devices[deviceIndex]);
    if (mcdu) {
        mcdu->setLedBrightness(MCDULed::SCREEN_BACKLIGHT, brightness);
        return true;
    }
    return false;
}

bool mcdu_setLed(int deviceIndex, int ledId, bool state) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return false;
    }
    
    auto mcdu = dynamic_cast<ProductMCDU*>(devices[deviceIndex]);
    if (mcdu) {
        mcdu->setLedBrightness(MCDULed(ledId), state ? 1 : 0);
        return true;
    }
    return false;
}
