#ifndef PRODUCT_PFP_H
#define PRODUCT_PFP_H

#include "usbdevice.h"
#include "pfp-aircraft-profile.h"
#include <map>

class ProductPFP: public USBDevice {
    
private:
    PfpAircraftProfile* profile = nullptr;
    std::vector<std::vector<char>> page;
    std::vector<std::vector<char>> previousPage;
    std::map<std::string, std::string> cachedDatarefValues;
    
    void updatePage();
    void draw(const std::vector<std::vector<char>> *pagePtr = nullptr);
    std::pair<uint8_t, uint8_t> dataFromColFont(char color, bool fontSmall = false);
    std::vector<int> pressedButtonIndices = {};
    
    void setProfileForCurrentAircraft();
    void monitorDatarefs();

public:
    ProductPFP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductPFP();

    const char* classIdentifier() override;
    bool connect() override;
    void disconnect() override;
    void update() override;
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
    
    void setLedBrightness(PFPLed led, uint8_t brightness);
    
    void clear();
    void clear2(unsigned char variant);
};

#endif
