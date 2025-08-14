#ifndef PRODUCT_FCUEFIS_H
#define PRODUCT_FCUEFIS_H

#include "usbdevice.h"
#include "fcu-efis-aircraft-profile.h"
#include <map>
#include <set>

class ProductFCUEfis: public USBDevice {
    
private:
    FCUEfisAircraftProfile* profile;
    FCUDisplayData displayData;
    int lastUpdateCycle;
    std::set<int> pressedButtonIndices;
    std::map<std::string, int> selectorPositions;  // Track rotary selector positions by dataref
    
    void setProfileForCurrentAircraft();
    void updateDisplays();

public:
    ProductFCUEfis(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductFCUEfis();
    
    static constexpr unsigned char IdentifierByte = 0x10;
    
    const char* classIdentifier() override;
    bool connect() override;
    void disconnect() override;
    void update() override;
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
    
    void setLedBrightness(FCUEfisLed led, uint8_t brightness);
    void monitorDatarefs();
    
    void initializeDisplays();
    void sendFCUDisplay(const std::string& speed, const std::string& heading, const std::string& altitude, const std::string& vs);
    void sendEfisDisplayWithFlags(EfisDisplayValue *data, bool isRightSide);
    
    // Getter for display data (for testing)
    FCUDisplayData& getDisplayData() { return displayData; }
};

#endif
