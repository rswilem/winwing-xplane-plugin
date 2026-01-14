#ifndef PRODUCT_ECAM32_H
#define PRODUCT_ECAM32_H

#include "ecam32-aircraft-profile.h"
#include "usbdevice.h"

#include <set>

enum class ECAM32Led : int {
    BACKLIGHT = 0,
    OVERALL_LEDS_BRIGHTNESS = 1,
    //UNUSED = 2,
    EMER_CANC_BRIGHTNESS = 3,

    _START = 4,
    ENG = 4,
    BLEED = 5,
    PRESS = 6,
    ELEC = 7,
    HYD = 8,
    FUEL = 9,
    APU = 10,
    COND = 11,
    DOOR = 12,
    WHEEL = 13,
    F_CTL = 14,
    CLR_LEFT = 15,
    STS = 16,
    CLR_RIGHT = 17,
    _END = 17,
};

class ProductECAM32 : public USBDevice {
    private:
        ECAM32AircraftProfile *profile;
        int menuItemId;
        uint64_t lastButtonStateLo;
        uint32_t lastButtonStateHi;
        std::set<int> pressedButtonIndices;

        void setProfileForCurrentAircraft();

    public:
        ProductECAM32(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        ~ProductECAM32();

        static constexpr unsigned char IdentifierByte = 0x70;

        const char *classIdentifier() override;
        bool connect() override;
        void blackout() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;

        void setAllLedsEnabled(bool enabled);
        void setLedBrightness(ECAM32Led led, uint8_t brightness);
};

#endif
