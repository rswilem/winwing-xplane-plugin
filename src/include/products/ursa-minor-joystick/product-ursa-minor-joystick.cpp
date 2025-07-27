#include "product-ursa-minor-joystick.h"
#include "dataref.h"
#include "appstate.h"
#include <algorithm>

ProductUrsaMinorJoystick::ProductUrsaMinorJoystick(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    connect();
}

ProductUrsaMinorJoystick::~ProductUrsaMinorJoystick() {
    disconnect();
}

const char* ProductUrsaMinorJoystick::classIdentifier() {
    return "Product-UrsaMinorJoystick";
}

bool ProductUrsaMinorJoystick::connect() {
    if (USBDevice::connect()) {
        setLedBrightness(0);
        profileReady = true;
        return true;
    }

    return false;
}

void ProductUrsaMinorJoystick::disconnect() {
    setLedBrightness(0);
    setVibration(0);
    
    USBDevice::disconnect();
    
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("sim/flightmodel/failures/onground_any");
    didInitializeDatarefs = false;
    AppState::getInstance()->fastUpdate = false;
}

void ProductUrsaMinorJoystick::update() {
    if (!connected) {
        return;
    }
    
    if (!didInitializeDatarefs) {
        initializeDatarefs();
    }
    
    USBDevice::update();
    
    if (Dataref::getInstance()->getCached<bool>("sim/flightmodel/failures/onground_any") && Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/avionics_on")) {
        float gForce = Dataref::getInstance()->get<float>("sim/flightmodel/forces/g_nrml");
        float delta = fabs(gForce - lastGForce);
        lastGForce = gForce;
        
        uint8_t vibration = (uint8_t)std::min(255.0f, delta * 400.0f);
        if (vibration < 8) {
            vibration = 0;
        }

        setVibration(vibration);
        lastVibration = vibration;
    }
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
    if (!Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel")) {
        return;
    }
    
    didInitializeDatarefs = true;
    
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [this](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
        setLedBrightness(target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/flightmodel/failures/onground_any", [this](bool wheelsOnGround) {
        AppState::getInstance()->fastUpdate = wheelsOnGround;
        
        if (!wheelsOnGround && lastVibration > 0) {
            lastVibration = 0;
            setVibration(lastVibration);
        }
    });
}
