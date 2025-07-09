#include "product-ursa-minor-joystick.h"
#include "dataref.h"

ProductUrsaMinorJoystick::ProductUrsaMinorJoystick(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    printf("Found one ursa minor UrsaMinorJoystick.\n");
    connect();
}

ProductUrsaMinorJoystick::~ProductUrsaMinorJoystick() {
    printf("Cleaning up joystick\n");
    disconnect();
}

bool ProductUrsaMinorJoystick::connect() {
    if (USBDevice::connect()) {
        setLedBrightness(0);
        return true;
    }

    return false;
}

void ProductUrsaMinorJoystick::disconnect() {
    setLedBrightness(0);
    
    USBDevice::disconnect();
    
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/DUBrightness");
    Dataref::getInstance()->unbind("AirbusFBW/MCDUIntegBrightness");
    didInitializeDatarefs = false;
}

void ProductUrsaMinorJoystick::update() {
    if (!connected) {
        return;
    }
    
    if (!didInitializeDatarefs) {
        initializeDatarefs();
    }
    USBDevice::update();
}

bool ProductUrsaMinorJoystick::setVibration(uint8_t vibration) {
    return writeData({0x02, 7, 191, 0, 0, 3, 0x49, 0, vibration, 0, 0, 0, 0, 0});
}

bool ProductUrsaMinorJoystick::setLedBrightness(uint8_t brightness) {
    // https://github.com/schenlap/winwing_fcu/blob/main/documentation/README.md

    // Winwing HID Output Report for Backlight Control
    // Packet structure (14 bytes):
    // [0]  0x02      : Report ID (always 0x02 for output)
    // [1]  0x20      : ComponentID(1) (0x20 = FCU/Panel, see device docs)
    // [2]  0xBB      : ComponentID(0) (0xBB = Backlight/LED, see device docs)
    // [3]  0x00      : Reserved / Unused
    // [4]  0x00      : Reserved / Unused
    // [5]  0x03      : Length of the data (3 bytes for backlight control)
    // [6]  0x49      : Command ID (0x49 = Set Backlight, see device docs)
    // [7]  0x00      : Led number
    // [8]  brightness: Backlight value (0-255)
    // [9]  0x00      : Reserved / Unused
    // [10] 0x00      : Reserved / Unused
    // [11] 0x00      : Reserved / Unused
    // [12] 0x00      : Reserved / Unused
    // [13] 0x00      : Reserved / Unused
    return writeData({0x02, 0x20, 0xbb, 0, 0, 3, 0x49, 0, brightness, 0, 0, 0, 0, 0});
}


void ProductUrsaMinorJoystick::initializeDatarefs() {
    if (!Dataref::getInstance()->exists("AirbusFBW/DUBrightness")) {
        return;
    }
    
    didInitializeDatarefs = true;
    
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/DUBrightness", [this](std::vector<float> brightness) {
        if (brightness.size() < 8) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[0] * 255.0f : 0;
        setLedBrightness(target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/DUBrightness");
    });
}
