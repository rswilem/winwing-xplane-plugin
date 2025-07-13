#ifndef PRODUCT_MCDU_H
#define PRODUCT_MCDU_H

#include "usbdevice.h"
#include <map>
#include <regex>

enum class MCDULed : int {
    BACKLIGHT = 0,
    SCREEN_BACKLIGHT = 1,
    FAIL = 8,
    FM = 9,
    MCDU = 10,
    MENU = 11,
    FM1 = 12,
    IND = 13,
    RDY = 14,
    STATUS = 15,
    FM2 = 16
};

class ProductMCDU: public USBDevice {
    
private:
    bool didInitializeDatarefs = false;
    std::vector<std::vector<char>> page;
    std::vector<std::vector<char>> previousPage;
    std::regex datarefRegex;
    std::map<std::string, std::string> cachedDatarefValues;
    void updatePage();
    void writeLineToPage(int line, int pos, const std::string &text, char color = 'W', bool fontSmall = false);
    void draw(const std::vector<std::vector<char>> *pagePtr = nullptr, int vertslew_key = 0);
    std::pair<uint8_t, uint8_t> dataFromColFont(char color, bool fontSmall = false);
    std::vector<int> pressedButtonIndices = {};
    
    void initializeDatarefs();

public:
    ProductMCDU(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductMCDU();

    bool connect() override;
    void disconnect() override;
    void update() override;
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
    
    void setLedBrightness(MCDULed led, uint8_t brightness);
    
    void clear();
    void clear2(unsigned char variant);
};

#endif
