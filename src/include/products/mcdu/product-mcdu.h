#ifndef PRODUCT_MCDU_H
#define PRODUCT_MCDU_H

#include "usbdevice.h"
#include "mcdu-aircraft-profile.h"
#include <map>
#include <regex>
#include <memory>

class ProductMCDU: public USBDevice {
    
private:
    std::vector<std::vector<char>> page;
    std::vector<std::vector<char>> previousPage;
    std::map<std::string, std::string> cachedDatarefValues;
    McduAircraftProfile *profile;
    
    void updatePage();
    void draw(const std::vector<std::vector<char>> *pagePtr = nullptr);
    std::pair<uint8_t, uint8_t> dataFromColFont(char color, bool fontSmall = false);
    std::vector<int> pressedButtonIndices = {};
    
    void setProfileForCurrentAircraft();

public:
    ProductMCDU(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductMCDU();

    const char* classIdentifier() override;
    bool connect() override;
    void disconnect() override;
    void update() override;
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
    
    void monitorDatarefs();
    void setLedBrightness(MCDULed led, uint8_t brightness);
    
    void clear();
    void clear2(unsigned char variant);
};

#endif
