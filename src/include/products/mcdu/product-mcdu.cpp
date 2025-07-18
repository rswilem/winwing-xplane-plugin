#include "product-mcdu.h"
#include "dataref.h"
#include "appstate.h"
#include "toliss-mcdu-profile.h"
#include "laminar-mcdu-profile.h"

constexpr unsigned int PAGE_LINES = 14; // Header + 6 * label + 6 * cont + textbox
constexpr unsigned int PAGE_CHARS_PER_LINE = 24;
constexpr unsigned int PAGE_BYTES_PER_CHAR = 3;
constexpr unsigned int PAGE_BYTES_PER_LINE = PAGE_CHARS_PER_LINE * PAGE_BYTES_PER_CHAR;

ProductMCDU::ProductMCDU(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    profile = nullptr;
    page = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_BYTES_PER_LINE, ' '));
    previousPage = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_BYTES_PER_LINE, ' '));
    
    connect();
}

ProductMCDU::~ProductMCDU() {
    disconnect();
}

void ProductMCDU::setProfileForCurrentAircraft() {
    if (TolissMcduProfile::IsEligible()) {
        clear();
        profile = new TolissMcduProfile();
        monitorDatarefs();
    }
    
    else if (LaminarMcduProfile::IsEligible()) {
        clear();
        profile = new LaminarMcduProfile();
        monitorDatarefs();
    }
}

bool ProductMCDU::connect() {
    if (USBDevice::connect()) {
        uint8_t col_bg[] = {0x00, 0x00, 0x00};
        
        writeData({0xf0, 0x0, 0x1, 0x38, 0x32, 0xbb, 0x0, 0x0, 0x1e, 0x1, 0x0, 0x0, 0xc4, 0x24, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x18, 0x1, 0x0, 0x0, 0xc4, 0x24, 0xa, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x34, 0x0, 0x18, 0x0, 0xe, 0x0, 0x18, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0xc4, 0x24, 0xa, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x2, 0x38, 0x0, 0x0, 0x0, 0x1, 0x0, 0x5, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0xc4, 0x24, 0xa, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x1, 0x0, 0x6, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x3, 0x38, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0xff, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0xa5, 0xff, 0xff, 0x5, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x4, 0x38, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0xff, 0xff, 0xff, 0xff, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0xff, 0xff, 0x0, 0xff, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x5, 0x38, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x3d, 0xff, 0x0, 0xff, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0xff, 0x63, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x6, 0x38, 0xff, 0xff, 0x9, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0xff, 0xff, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x7, 0x38, 0x0, 0x0, 0x2, 0x0, 0x0, 0xff, 0xff, 0xff, 0xb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x42, 0x5c, 0x61, 0xff, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x8, 0x38, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x77, 0x77, 0x77, 0xff, 0xd, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x2, 0x0, 0x5e, 0x73, 0x79, 0xff, 0xe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x9, 0x38, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, col_bg[0], col_bg[1], col_bg[2], 0xff, 0xf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0xa5, 0xff, 0xff, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0xa, 0x38, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0xff, 0xff, 0xff, 0xff, 0x11, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0xb, 0x38, 0xff, 0x12, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x3d, 0xff, 0x0, 0xff, 0x13, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0xc, 0x38, 0x0, 0x3, 0x0, 0xff, 0x63, 0xff, 0xff, 0x14, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0xff, 0xff, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0xd, 0x38, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0xff, 0xff, 0xff, 0x16, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x42, 0x5c, 0x61, 0xff, 0x17, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0xe, 0x38, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x77, 0x77, 0x77, 0xff, 0x18, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x3, 0x0, 0x5e, 0x73, 0x79, 0xff, 0x19, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0xf, 0x38, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x4, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x10, 0x38, 0x1b, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x19, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x4, 0x0, 0x2, 0x0, 0x0, 0x0, 0x1c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x32, 0xbb, 0x0, 0x0, 0x1a, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});
        writeData({0xf0, 0x0, 0x11, 0x12, 0x2, 0x32, 0xbb, 0x0, 0x0, 0x1c, 0x1, 0x0, 0x0, 0x76, 0x72, 0x19, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0});

        
//        clear2(1);
        
        setLedBrightness(MCDULed::BACKLIGHT, 128);
        setLedBrightness(MCDULed::SCREEN_BACKLIGHT, 128);
        const MCDULed ledsToSet[] = {
            MCDULed::FM,
            MCDULed::MCDU,
            MCDULed::FM1,
            MCDULed::IND,
            MCDULed::RDY,
            MCDULed::STATUS,
            MCDULed::FM2
        };

        for (auto led : ledsToSet) {
            setLedBrightness(led, 0);
        }
        
        setLedBrightness(MCDULed::FAIL, 1);
        setLedBrightness(MCDULed::MENU, 1);
        
        clear2(8);
        
        if (!profile) {
            setProfileForCurrentAircraft();
        }
        
        return true;
    }
    
    return false;
}

void ProductMCDU::disconnect() {
    const MCDULed ledsToSet[] = {
        MCDULed::BACKLIGHT,
        MCDULed::SCREEN_BACKLIGHT,
        MCDULed::FAIL,
        MCDULed::FM,
        MCDULed::MCDU,
        MCDULed::MENU,
        MCDULed::FM1,
        MCDULed::IND,
        MCDULed::RDY,
        MCDULed::STATUS,
        MCDULed::FM2
    };

    for (auto led : ledsToSet) {
        setLedBrightness(led, 0);
    }
    
    USBDevice::disconnect();
    if (profile) {
        delete profile;
        profile = nullptr;
    }
    
    // Clear caches
    cachedDatarefValues.clear();
}

void ProductMCDU::update() {
    if (!connected) {
        return;
    }
    
    if (!profile) {
        setProfileForCurrentAircraft();
        return;
    }
    
    USBDevice::update();
    updatePage();
}

void ProductMCDU::didReceiveData(int reportId, uint8_t *report, int reportLength) {
    if (!profile) {
        return;
    }
    
    if (reportId != 1 || reportLength != 25) {
#if DEBUG
        printf("[MCDU] Ignoring reportId %d, length %d\n", reportId, reportLength);
        printf("[MCDU] Data (hex): ");
        for (int i = 0; i < reportLength; ++i) {
            printf("%02X ", report[i]);
        }
        printf("\n");
#endif
        return;
    }

    uint64_t buttons_lo = 0;
    uint32_t buttons_hi = 0;
    for (int i = 0; i < 8; ++i) {
        buttons_lo |= ((uint64_t)report[i+1]) << (8 * i);
    }
    for (int i = 0; i < 4; ++i) {
        buttons_hi |= ((uint32_t)report[i+9]) << (8 * i);
    }
    
    const std::vector<MCDUButtonDef>& currentButtonDefs = profile->buttonDefs();
    size_t numButtons = currentButtonDefs.size();
    
    for (int i = 0; i < numButtons; ++i) {
        bool pressed;
        if (i < 64) {
            pressed = (buttons_lo >> i) & 1;
        } else {
            pressed = (buttons_hi >> (i - 64)) & 1;
        }
        
        bool pressedButtonIndexExists = std::find(pressedButtonIndices.begin(), pressedButtonIndices.end(), i) != pressedButtonIndices.end();
        if (pressed && !pressedButtonIndexExists) {
            pressedButtonIndices.push_back(i);
            Dataref::getInstance()->executeCommand(currentButtonDefs[i].dataref.c_str(), xplm_CommandBegin);
        }
        else if (pressed && pressedButtonIndexExists) {
            Dataref::getInstance()->executeCommand(currentButtonDefs[i].dataref.c_str(), xplm_CommandContinue);
        }
        else if (!pressed && pressedButtonIndexExists) {
            pressedButtonIndices.erase(std::remove(pressedButtonIndices.begin(), pressedButtonIndices.end(), i), pressedButtonIndices.end());
            Dataref::getInstance()->executeCommand(currentButtonDefs[i].dataref.c_str(), xplm_CommandEnd);
        }
    }
}

void ProductMCDU::updatePage() {
    if (!profile) {
        return;
    }
    
    bool anyDatarefChanged = false;
    const std::vector<std::string>& currentDatarefs = profile->displayDatarefs();
    for (const std::string &ref : currentDatarefs) {
        std::string newValue = Dataref::getInstance()->getCached<std::string>(ref.c_str());
        auto it = cachedDatarefValues.find(ref);
        if (it == cachedDatarefValues.end() || it->second != newValue) {
            cachedDatarefValues[ref] = newValue;
            anyDatarefChanged = true;
        }
    }

    if (!anyDatarefChanged) {
        return;
    }

    profile->updatePage(page, cachedDatarefValues);
    
    if (page != previousPage) {
        previousPage = page;
        draw();
    }
}

void ProductMCDU::draw(const std::vector<std::vector<char>> *pagePtr) {
    const auto &p = pagePtr ? *pagePtr : page;
    std::vector<uint8_t> buf;

    for (int i = 0; i < PAGE_LINES; ++i) {
        for (int j = 0; j < PAGE_CHARS_PER_LINE; ++j) {
            char color = p[i][j * PAGE_BYTES_PER_CHAR];
            bool font_small = p[i][j * PAGE_BYTES_PER_CHAR + 1];
            auto [data_low, data_high] = dataFromColFont(color, font_small);
            buf.push_back(data_low);
            buf.push_back(data_high);

            char val = p[i][j * PAGE_BYTES_PER_CHAR + PAGE_BYTES_PER_CHAR - 1];
            switch (val) {
                case '#':
                    buf.insert(buf.end(), {0xe2, 0x98, 0x90});
                    break;
                case '<': // Change to arrow
                case '>':
                    if (font_small) {
                        buf.insert(buf.end(), {0xe2, 0x86, static_cast<unsigned char>((val == '<' ? 0x90 : 0x92))});
                    }
                    else {
                        goto default_case;
                    }
                    break;

                case 30: // Change to up arrow
                case 31: // Change to down arrow
                    if (font_small) {
                        buf.insert(buf.end(), {0xe2, 0x86, static_cast<unsigned char>((val == 30 ? 0x91 : 0x93))});
                    }
                    else {
                        goto default_case;
                    }
                    break;

                case '`': // Change to °
                    buf.insert(buf.end(), {0xc2, 0xb0});
                    break;
                
                default:
                default_case:
                    buf.push_back(val);
                    break;
            }
        }
    }

    while (!buf.empty()) {
        size_t maxLength = std::min<size_t>(63, buf.size());
        std::vector<uint8_t> usbBuf(buf.begin(), buf.begin() + maxLength);
        usbBuf.insert(usbBuf.begin(), 0xf2);
        if (maxLength < 63)
            usbBuf.insert(usbBuf.end(), 63 - maxLength, 0);
        writeData(usbBuf);
        buf.erase(buf.begin(), buf.begin() + maxLength);
    }
}

std::pair<uint8_t, uint8_t> ProductMCDU::dataFromColFont(char color, bool fontSmall) {
    if (!profile) {
        return {0x42, 0x00}; // Default white
    }
    
    const std::map<char, int>& col_map = profile->colorMap();

    char upperColor = std::toupper(color);
    auto it = col_map.find(upperColor);
    if (it == col_map.end()) {
        //debug("Unknown color '%c', defaulting to white\n", color);
        it = col_map.find(' ');
    }

    int value = it->second;
    if (fontSmall) {
        value += 0x016b;
    }

    return {static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF)};
}

void ProductMCDU::clear() {
    std::vector<uint8_t> blankLine = {};
    blankLine.push_back(0xf2);
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
        blankLine.push_back(0x42);
        blankLine.push_back(0x00);
        blankLine.push_back(' ');
    }
    
    for (int i = 0; i < 16; ++i) {
        writeData(blankLine);
    }
}

void ProductMCDU::clear2(unsigned char variant) {
    std::vector<uint8_t> data;
    
    switch (variant) {
        case 1: // Gray
            data = {0xf0, 0x00, 0x02, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x53, 0x20, 0x07, 0x00};
            break;
            
        case 2: // Black
            data = {0xf0, 0x00, 0x03, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xfd, 0x24, 0x07, 0x00};
            break;
            
        case 3: // Red
            data = {0xf0, 0x00, 0x04, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x55, 0x29, 0x07, 0x00};
            break;
            
        case 4: // Green
            data = {0xf0, 0x00, 0x06, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xad, 0x95, 0x09, 0x00};
            break;
            
        case 5: // Blue
            data = {0xf0, 0x00, 0x07, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xa7, 0x9b, 0x09, 0x00};
            break;
            
        case 6: // Yellow
            data = {0xf0, 0x00, 0x08, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x09, 0xa1, 0x09, 0x00};
            break;
            
        case 7: // Purple
            data = {0xf0, 0x00, 0x09, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x05, 0xa7, 0x09, 0x00};
            break;
            
        case 8: // WinWing Logo
            data = {0xf0, 0x00, 0x0a, 0x12, 0x32, 0xbb, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xd4, 0xac, 0x09, 0x00};
            break;
            
        default:
            return;
    }
    
    std::vector<uint8_t> extra = {
        0x00, 0x01, 0x00, 0x00, 0x00, (unsigned char)(0x0c + variant), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    data.insert(data.end(), extra.begin(), extra.end());
    
    writeData(data);
}

void ProductMCDU::monitorDatarefs() {
    if (!profile) {
        return;
    }
    
    const MCDULed ledsToSet[] = {
        MCDULed::FAIL,
        MCDULed::FM,
        MCDULed::MCDU,
        MCDULed::MENU,
        MCDULed::FM1,
        MCDULed::IND,
        MCDULed::RDY,
        MCDULed::STATUS,
        MCDULed::FM2
    };

    for (auto led : ledsToSet) {
        setLedBrightness(led, 0);
    }
    
    profile->ledBrightnessCallback = [this](MCDULed led, unsigned char brightness) {
        setLedBrightness(led, brightness);
    };
}

void ProductMCDU::setLedBrightness(MCDULed led, uint8_t brightness) {
    writeData({0x02, 0x32, 0xbb, 0, 0, 3, 0x49, static_cast<uint8_t>(led), brightness, 0, 0, 0, 0, 0});
}


