#ifndef PRODUCT_PFP_H
#define PRODUCT_PFP_H

#include "usbdevice.h"
#include "pfp-aircraft-profile.h"
#include <map>
#include <set>

class ProductPFP : public USBDevice {
private:
    PfpAircraftProfile *profile;
    std::vector<std::vector<char>> page;
    std::vector<std::vector<char>> previousPage;
    std::map<std::string, std::string> cachedDatarefValues;
    std::set<int> pressedButtonIndices;
    
    void updatePage();
    void draw(const std::vector<std::vector<char>> *pagePtr = nullptr);
    
    std::pair<uint8_t, uint8_t> dataFromColFont(char color, bool fontSmall = false);
    
    void setProfileForCurrentAircraft();

public:
    ProductPFP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
    ~ProductPFP();
    
    static constexpr unsigned char IdentifierByte = 0x31;
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
    
    void setLedBrightness(PFPLed led, uint8_t brightness);
    
    void clear();
    void clear2(unsigned char variant);
};

#endif
