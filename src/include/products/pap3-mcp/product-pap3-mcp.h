#ifndef PRODUCT_PAP3MCP_H
#define PRODUCT_PAP3MCP_H

#include "pap3-mcp-aircraft-profile.h"
#include "usbdevice.h"

#include <map>
#include <set>
#include <vector>

class ProductPAP3MCP : public USBDevice {
    private:
        uint8_t packetNumber = 1;
        PAP3MCPAircraftProfile *profile;
        PAP3MCPDisplayData displayData;
        int lastUpdateCycle;
        int displayUpdateFrameCounter = 0;
        std::set<int> pressedButtonIndices;

        uint64_t lastButtonStateLo = 0;
        uint32_t lastButtonStateHi = 0;

        void setProfileForCurrentAircraft();
        void updateDisplays();

    public:
        ProductPAP3MCP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        ~ProductPAP3MCP();

        static constexpr unsigned char IdentifierByte = 0x0C;

        const char *classIdentifier() override;
        bool connect() override;
        void disconnect() override;
        void update() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;
        void forceStateSync() override;

        void setLedBrightness(PAP3MCPLed led, uint8_t brightness);

        void initializeDisplays();
        void clearDisplays();
        void sendLCDDisplay(const std::string &speed, int heading, int altitude, const std::string &vs, int crsCapt, int crsFo);
        void sendLCDCommit();
};

#endif
