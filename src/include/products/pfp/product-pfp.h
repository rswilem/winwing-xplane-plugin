#ifndef PRODUCT_PFP_H
#define PRODUCT_PFP_H

#include "usbdevice.h"

enum class PFPLed : int {
    BACKLIGHT = 0,
    SCREEN_BACKLIGHT = 1,
    CALL = 8,
    FAIL = 9,
    MSG = 10,
    OFST = 11,
    UNKNOWN1 = 12,
    UNKNOWN2 = 13,
    UNKNOWN3 = 14,
    UNKNOWN4 = 15,
    UNKNOWN5 = 16
};

class ProductPFP: public USBDevice {
    
private:
    bool didInitializeDatarefs = false;
    std::vector<std::vector<char>> page;
    void updatePage();
    void writeLineToPage(int line, int pos, const std::string &text, char color = 'W', bool fontSmall = false);
    void draw(const std::vector<std::vector<char>> *pagePtr = nullptr);
    std::pair<uint8_t, uint8_t> dataFromColFont(char color, bool fontSmall = false);
    std::vector<int> pressedButtonIndices = {};
    
    void initializeDatarefs();

public:
    ProductPFP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductPFP();

    bool connect() override;
    void disconnect() override;
    void update() override;
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
    
    void setLedBrightness(PFPLed led, uint8_t brightness);
    
    void clear();
    void clear2(unsigned char variant);
};

#endif
