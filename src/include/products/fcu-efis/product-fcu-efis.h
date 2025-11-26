#ifndef PRODUCT_FCUEFIS_H
#define PRODUCT_FCUEFIS_H

#include "fcu-efis-aircraft-profile.h"
#include "usbdevice.h"
#include "packet-utils.h"

#include <map>
#include <set>

class ProductFCUEfis : public USBDevice {
    private:
        PacketUtils::PacketNumber packetNumber;
        FCUEfisAircraftProfile *profile;
        FCUDisplayData displayData;
        int lastUpdateCycle;
        int displayUpdateFrameCounter = 0;
        std::set<int> pressedButtonIndices;
        std::map<std::string, int> selectorPositions;

        uint64_t lastButtonStateLo = 0;
        uint32_t lastButtonStateHi = 0;

        void setProfileForCurrentAircraft();
        void updateDisplays();

    public:
        ProductFCUEfis(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        ~ProductFCUEfis();

        static constexpr unsigned char FCUIdentifierByte = 0x10;
        static constexpr unsigned char EfisRightIdentifierByte = 0x0E;
        static constexpr unsigned char EfisLeftIdentifierByte = 0x0D;

        const char *classIdentifier() override;
        bool connect() override;
        void disconnect() override;
        void update() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;
        void forceStateSync() override;

        void setLedBrightness(FCUEfisLed led, uint8_t brightness);

        void initializeDisplays();
        void clearDisplays();
        void sendFCUDisplay(const std::string &speed, const std::string &heading, const std::string &altitude, const std::string &vs);
        void sendEfisDisplayWithFlags(EfisDisplayValue *data, bool isRightSide);
};

#endif
