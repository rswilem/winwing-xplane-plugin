#ifndef PRODUCT_URSA_MINOR_THROTTLE_H
#define PRODUCT_URSA_MINOR_THROTTLE_H

#include "ursa-minor-throttle-aircraft-profile.h"
#include "usbdevice.h"

#include <set>

enum class UrsaMinorThrottleLed : int {
    BACKLIGHT = 0,
    OVERALL_LEDS_BRIGHTNESS = 1,
    //UNUSED = 2,

    _START = 4,
    ENG = 4,
    _END = 5,
};

class ProductUrsaMinorThrottle : public USBDevice {
    private:
        UrsaMinorThrottleAircraftProfile *profile;
        int menuItemId;
        uint64_t lastButtonStateLo;
        uint32_t lastButtonStateHi;
        std::set<int> pressedButtonIndices;

        void setProfileForCurrentAircraft();
        void loadVibrationSetting(const std::string &preference);

    public:
        ProductUrsaMinorThrottle(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        ~ProductUrsaMinorThrottle();

        static constexpr unsigned char IdentifierByte = 0x70;
        float vibrationMultiplier;

        const char *classIdentifier() override;
        bool connect() override;
        void disconnect() override;
        void update() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;

        void setAllLedsEnabled(bool enabled);
        void setLedBrightness(UrsaMinorThrottleLed led, uint8_t brightness);
        void setVibration(uint8_t vibration);
};

#endif
