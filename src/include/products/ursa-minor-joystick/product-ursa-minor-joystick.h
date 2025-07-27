#ifndef PRODUCT_URSA_MINOR_JOYSTICK_H
#define PRODUCT_URSA_MINOR_JOYSTICK_H

#include "usbdevice.h"

class ProductUrsaMinorJoystick: public USBDevice {
private:
    bool didInitializeDatarefs = false;
    int lastVibration;
    float lastGForce;

public:
    ProductUrsaMinorJoystick(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductUrsaMinorJoystick();
    
    const char* classIdentifier() override;
    bool connect() override;
    void disconnect() override;
    void update() override;

    bool setVibration(uint8_t vibration);
    bool setLedBrightness(uint8_t brightness);
    
    void initializeDatarefs();
};

#endif
