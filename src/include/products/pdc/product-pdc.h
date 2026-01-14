#ifndef PRODUCT_PDC_H
#define PRODUCT_PDC_H

#include "pdc-aircraft-profile.h"
#include "usbdevice.h"

#include <set>

enum class PDCLed : int {
    BACKLIGHT = 0
};

class ProductPDC : public USBDevice {
    private:
        PDCAircraftProfile *profile;
        int menuItemId;
        uint64_t lastButtonStateLo;
        uint32_t lastButtonStateHi;
        std::set<int> pressedButtonIndices;

        void setProfileForCurrentAircraft();

    public:
        ProductPDC(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName, PDCDeviceVariant variant, unsigned char identifierByte);
        ~ProductPDC();

        const unsigned char identifierByte;
        const PDCDeviceVariant deviceVariant;

        const char *classIdentifier() override;
        bool connect() override;
        void blackout() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;

        void setLedBrightness(PDCLed led, uint8_t brightness);
};

#endif
