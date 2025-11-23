#ifndef PRODUCT_URSA_MINOR_JOYSTICK_H
#define PRODUCT_URSA_MINOR_JOYSTICK_H

#include "ursa-minor-joystick-aircraft-profile.h"
#include "usbdevice.h"

class ProductUrsaMinorJoystick : public USBDevice {
    private:
        UrsaMinorJoystickAircraftProfile *profile;
        int menuItemId;

        void setProfileForCurrentAircraft();
        void loadVibrationFactor(const std::string &preference);

    public:
        ProductUrsaMinorJoystick(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName, unsigned char identifierByte);
        ~ProductUrsaMinorJoystick();

        const unsigned char identifierByte;
        float vibrationFactor;

        const char *classIdentifier() override;
        bool connect() override;
        void disconnect() override;
        void update() override;

        void setVibration(uint8_t vibration);
        void setLedBrightness(uint8_t brightness);
};

#endif
