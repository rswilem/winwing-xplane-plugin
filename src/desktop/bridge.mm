#include "bridge.h"
#include "usbcontroller.h"
#include "appstate.h"
#include "dataref.h"
#include "product-ursa-minor-joystick.h"
#include "product-mcdu.h"
#include "product-pfp.h"
#include "product-fcu-efis.h"
#include <vector>
#include <string>
#include <cstring>

// Forward declaration for mock dataref creation function
typedef void* XPLMDataRef;
typedef int XPLMDataTypeID;
#define xplmType_Data 32
#define xplmType_Float 2
#define xplmType_Int 1
#define xplmType_FloatArray 8
#define xplmType_IntArray 16

XPLMDataRef XPLMFindDataRef(const char* name);
XPLMDataRef createMockDataRefWithInference(const char* name, XPLMDataTypeID preferredType);
void clearAllMockDataRefs();


// Helper function to ensure dataref exists before setting
void ensureDatarefExists(const char* ref, XPLMDataTypeID preferredType) {
    if (!XPLMFindDataRef(ref)) {
        createMockDataRefWithInference(ref, preferredType);
    }
}

void clearDatarefCache() {
    Dataref::getInstance()->clearCache();
    clearAllMockDataRefs();
}

void setDatarefHexC(const char* ref, const uint8_t* hexD, int len) {
    const std::vector<uint8_t>& hex = std::vector<uint8_t>(hexD, hexD + len);
    
    // Check if this is a style dataref - if so, store as vector<unsigned char>
    std::string refStr(ref);
    if (refStr.find("style_line") != std::string::npos || refStr.find("ixeg/") != std::string::npos) {
        // Ensure dataref exists first
        ensureDatarefExists(ref, xplmType_Data);
        
        std::vector<unsigned char> styleBytes;
        for (uint8_t c : hex) {
            styleBytes.push_back(c);
        }
        Dataref::getInstance()->set<std::vector<unsigned char>>(ref, styleBytes, false);
    } else {
        // Ensure dataref exists first
        ensureDatarefExists(ref, xplmType_Data);
        
        // For text datarefs, convert to string (stopping at null terminator)
        std::string s;
        for (uint8_t c : hex) {
            if (c == 0x00) break;
            s += static_cast<char>(c);
        }
        Dataref::getInstance()->set<std::string>(ref, s, false);
    }
}

void setDatarefFloat(const char* ref, float value) {
    // Ensure dataref exists first
    ensureDatarefExists(ref, xplmType_Float);
    
    Dataref::getInstance()->set<float>(ref, value, false);
}

void setDatarefInt(const char* ref, int value) {
    // Ensure dataref exists first
    ensureDatarefExists(ref, xplmType_Int);
    
    Dataref::getInstance()->set<int>(ref, value, false);
}

void setDatarefFloatVector(const char* ref, const float* values, int count) {
    // Ensure dataref exists first
    ensureDatarefExists(ref, xplmType_FloatArray);
    
    std::vector<float> floatVector(values, values + count);
    Dataref::getInstance()->set<std::vector<float>>(ref, floatVector, false);
}

void setDatarefFloatVectorRepeated(const char* ref, float value, int count) {
    // Ensure dataref exists first
    ensureDatarefExists(ref, xplmType_FloatArray);
    
    std::vector<float> floatVector(count, value);
    Dataref::getInstance()->set<std::vector<float>>(ref, floatVector, false);
}

void setDatarefIntVector(const char* ref, const int* values, int count) {
    // Ensure dataref exists first
    ensureDatarefExists(ref, xplmType_IntArray);
    
    std::vector<int> intVector(values, values + count);
    Dataref::getInstance()->set<std::vector<int>>(ref, intVector, false);
}

void update() {
    AppState::getInstance()->pluginInitialized = true;
    AppState::Update(0.0f, 0.0f, 1, nullptr);
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

void* getFCUEfisHandle(int deviceIndex) {
    auto& devices = USBController::getInstance()->devices;
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return nullptr;
    }
    return dynamic_cast<ProductFCUEfis*>(devices[deviceIndex]);
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
    } else if (dynamic_cast<ProductFCUEfis*>(device)) {
        return "fcu-efis";
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

// FCU-EFIS functions via handle
void fcuefis_clear(void* fcuefisHandle) {
    if (!fcuefisHandle) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    // FCU-EFIS doesn't have a clear function like MCDU/PFP
    // Instead, we can set displays to show test values or blank
    fcuefis->initializeDisplays();
}

bool fcuefis_setLed(void* fcuefisHandle, int ledId, uint8_t value) {
    if (!fcuefisHandle) return false;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    fcuefis->setLedBrightness(static_cast<FCUEfisLed>(ledId), value);
    return true;
}

void fcuefis_setLedBrightness(void* fcuefisHandle, int ledId, uint8_t brightness) {
    if (!fcuefisHandle) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    fcuefis->setLedBrightness(static_cast<FCUEfisLed>(ledId), brightness);
}

void fcuefis_testDisplay(void* fcuefisHandle, const char* testType) {
    if (!fcuefisHandle || !testType) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    
    std::string test(testType);
    if (test == "SPEED") {
        fcuefis->sendFCUDisplay("250", "088", "12000", "1800");
    } else if (test == "HEADING") {
        fcuefis->sendFCUDisplay("120", "360", "35000", "2400");
    } else if (test == "ALTITUDE") {
        fcuefis->sendFCUDisplay("180", "270", "41000", "0000");
    } else if (test == "VS") {
        fcuefis->sendFCUDisplay("160", "180", "25000", "3000");
    } else if (test == "EFIS_R") {
        fcuefis->sendEfisRightDisplay("1013");
    } else if (test == "EFIS_L") {
        fcuefis->sendEfisLeftDisplay("1013");
    } else if (test == "ALL") {
        fcuefis->sendFCUDisplay("888", "888", "88888", "8888");
        fcuefis->sendEfisRightDisplay("8888");
        fcuefis->sendEfisLeftDisplay("8888");
    }
}

void fcuefis_efisRightTestDisplay(void* fcuefisHandle, const char* testType) {
    if (!fcuefisHandle || !testType) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    
    std::string test(testType);
    if (test == "QNH_1013") {
        // hPa: QNH mode but no decimal point
        fcuefis->sendEfisRightDisplayWithFlags("1013", true, false);
    } else if (test == "QNH_2992") {
        // inHg: show decimal point to display "29.92"
        fcuefis->sendEfisRightDisplayWithFlags("2992", true, true);
    } else if (test == "STD") {
        // STD: no decimal point
        fcuefis->sendEfisRightDisplayWithFlags("STD ", false, false);
    }
}

void fcuefis_efisLeftTestDisplay(void* fcuefisHandle, const char* testType) {
    if (!fcuefisHandle || !testType) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    
    std::string test(testType);
    if (test == "QNH_1013") {
        // hPa: QNH mode but no decimal point
        fcuefis->sendEfisLeftDisplayWithFlags("1013", true, false);
    } else if (test == "QNH_2992") {
        // inHg: show decimal point to display "29.92"
        fcuefis->sendEfisLeftDisplayWithFlags("2992", true, true);
    } else if (test == "STD") {
        // STD: no decimal point
        fcuefis->sendEfisLeftDisplayWithFlags("STD ", false, false);
    }
}

void fcuefis_efisRightClear(void* fcuefisHandle) {
    if (!fcuefisHandle) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    fcuefis->sendEfisRightDisplay("    ");  // Clear with 4 spaces
}

void fcuefis_efisLeftClear(void* fcuefisHandle) {
    if (!fcuefisHandle) return;
    auto fcuefis = static_cast<ProductFCUEfis*>(fcuefisHandle);
    fcuefis->sendEfisLeftDisplay("    ");  // Clear with 4 spaces
}
