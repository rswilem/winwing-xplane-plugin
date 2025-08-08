#ifndef PRODUCT_MCDU_H
#define PRODUCT_MCDU_H

#include "usbdevice.h"
#include "mcdu-aircraft-profile.h"
#include <map>
#include <set>
#include <chrono>

class ProductMCDU: public USBDevice {
private:
    McduAircraftProfile *profile;
    std::vector<std::vector<char>> page;
    int lastUpdateCycle;
    std::map<std::string, std::string> cachedDatarefValues;
    std::set<int> pressedButtonIndices;
    
    // Display update throttling
    std::chrono::steady_clock::time_point lastDisplayUpdate;
    static constexpr int DISPLAY_UPDATE_INTERVAL_MS = 50; // 20 FPS max for display
    
    void updatePage();
    void draw(const std::vector<std::vector<char>> *pagePtr = nullptr);
    std::pair<uint8_t, uint8_t> dataFromColFont(char color, bool fontSmall = false);
    
    void setProfileForCurrentAircraft();

public:
    ProductMCDU(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductMCDU();
    
    static constexpr unsigned char IdentifierByte = 0x32;
    static constexpr unsigned int PageLines = 14; // Header + 6 * label + 6 * cont + textbox
    static constexpr unsigned int PageCharsPerLine = 24;
    static constexpr unsigned int PageBytesPerChar = 3;
    static constexpr unsigned int PageBytesPerLine = PageCharsPerLine * PageBytesPerChar;

    const char* classIdentifier() override;
    bool connect() override;
    void disconnect() override;
    void update() override;
    void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
    void writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color, bool fontSmall = false);
    
    void setLedBrightness(MCDULed led, uint8_t brightness);
    
    void clearDisplay();
    void showBackground(unsigned char variant);
};

#endif
