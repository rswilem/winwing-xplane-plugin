#ifndef PRODUCT_AGP_H
#define PRODUCT_AGP_H

#include "agp-aircraft-profile.h"
#include "packet-utils.h"
#include "usbdevice.h"

#include <set>

enum class AGPLed : int {
    BACKLIGHT = 0,
    LCD_BRIGHTNESS = 1,
    OVERALL_LEDS_BRIGHTNESS = 2,

    _START = 3,
    LDG_GEAR_UNLK_LEFT = 3,
    LDG_GEAR_UNLK_CENTER = 4,
    LDG_GEAR_UNLK_RIGHT = 5,
    BRAKE_FAN_HOT = 6,
    LDG_GEAR_ARROW_GREEN_LEFT = 7,
    LDG_GEAR_ARROW_GREEN_CENTER = 8,
    LDG_GEAR_ARROW_GREEN_RIGHT = 9,
    BRAKE_FAN_ON = 10,
    AUTOBRK_DECEL_LO = 11,
    AUTOBRK_DECEL_MED = 12,
    AUTOBRK_DECEL_HI = 13,
    AUTOBRK_LO_ON = 14,
    AUTOBRK_MED_ON = 15,
    AUTOBRK_HI_ON = 16,
    TERRAIN_ON = 17,
    LDG_GEAR_LEVER_RED = 18,
    _END = 18,
};

enum class AGPTerrainNDPreference {
    CAPTAIN = 0,
    FIRST_OFFICER
};

class ProductAGP : public USBDevice {
    private:
        AGPAircraftProfile *profile;
        int menuItemId;
        int displayUpdateFrameCounter = 0;
        uint64_t lastButtonStateLo;
        uint32_t lastButtonStateHi;
        std::set<int> pressedButtonIndices;
        PacketUtils::PacketNumber packetNumber;

        void setProfileForCurrentAircraft();

    public:
        ProductAGP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        ~ProductAGP();

        static constexpr unsigned char IdentifierByte = 0x80;

        AGPTerrainNDPreference terrainNDPreference;

        const char *classIdentifier() override;
        bool connect() override;
        void disconnect() override;
        void update() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;

        void setAllLedsEnabled(bool enabled);
        void setLedBrightness(AGPLed led, uint8_t brightness);
        void setLCDText(const std::string &chrono, const std::string &utcTime, const std::string &elapsedTime);
};

#endif
