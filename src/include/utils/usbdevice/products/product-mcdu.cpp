#include "product-mcdu.h"
#include "dataref.h"
#include "appstate.h"

constexpr unsigned int PAGE_LINES = 14; // Header + 6 * label + 6 * cont + textbox
constexpr unsigned int PAGE_CHARS_PER_LINE = 24;
constexpr unsigned int PAGE_BYTES_PER_CHAR = 3;
constexpr unsigned int PAGE_BYTES_PER_LINE = PAGE_CHARS_PER_LINE * PAGE_BYTES_PER_CHAR;

struct ButtonDef {
    int id;
    const char* name;
    const char* dataref;
};

static constexpr ButtonDef buttonDefs[] = {
    {0, "LSK1L", "AirbusFBW/MCDU1LSK1L"},
    {1, "LSK2L", "AirbusFBW/MCDU1LSK2L"},
    {2, "LSK3L", "AirbusFBW/MCDU1LSK3L"},
    {3, "LSK4L", "AirbusFBW/MCDU1LSK4L"},
    {4, "LSK5L", "AirbusFBW/MCDU1LSK5L"},
    {5, "LSK6L", "AirbusFBW/MCDU1LSK6L"},
    {6, "LSK1R", "AirbusFBW/MCDU1LSK1R"},
    {7, "LSK2R", "AirbusFBW/MCDU1LSK2R"},
    {8, "LSK3R", "AirbusFBW/MCDU1LSK3R"},
    {9, "LSK4R", "AirbusFBW/MCDU1LSK4R"},
    {10, "LSK5R", "AirbusFBW/MCDU1LSK5R"},
    {11, "LSK6R", "AirbusFBW/MCDU1LSK6R"},
    {12, "DIRTO", "AirbusFBW/MCDU1DirTo"},
    {13, "PROG", "AirbusFBW/MCDU1Prog"},
    {14, "PERF", "AirbusFBW/MCDU1Perf"},
    {15, "INIT", "AirbusFBW/MCDU1Init"},
    {16, "DATA", "AirbusFBW/MCDU1Data"},
    {17, "EMPTY_TOP_RIGHT", "toliss_airbus/iscs_open"},
    {18, "BRT", "AirbusFBW/MCDU1KeyBright"},
    {19, "FPLN", "AirbusFBW/MCDU1Fpln"},
    {20, "RADNAV", "AirbusFBW/MCDU1RadNav"},
    {21, "FUEL", "AirbusFBW/MCDU1FuelPred"},
    {22, "SEC-FPLN", "AirbusFBW/MCDU1SecFpln"},
    {23, "ATC", "AirbusFBW/MCDU1ATC"},
    {24, "MENU", "AirbusFBW/MCDU1Menu"},
    {25, "DIM", "AirbusFBW/MCDU1KeyDim"},
    {26, "AIRPORT", "AirbusFBW/MCDU1Airport"},
    {27, "EMPTY_BOTTOM_LEFT", "AirbusFBW/purser/fwd"},
    {28, "SLEW_LEFT", "AirbusFBW/MCDU1SlewLeft"},
    {29, "SLEW_UP", "AirbusFBW/MCDU1SlewUp"},
    {30, "SLEW_RIGHT", "AirbusFBW/MCDU1SlewRight"},
    {31, "SLEW_DOWN", "AirbusFBW/MCDU1SlewDown"},
    {32, "KEY1", "AirbusFBW/MCDU1Key1"},
    {33, "KEY2", "AirbusFBW/MCDU1Key2"},
    {34, "KEY3", "AirbusFBW/MCDU1Key3"},
    {35, "KEY4", "AirbusFBW/MCDU1Key4"},
    {36, "KEY5", "AirbusFBW/MCDU1Key5"},
    {37, "KEY6", "AirbusFBW/MCDU1Key6"},
    {38, "KEY7", "AirbusFBW/MCDU1Key7"},
    {39, "KEY8", "AirbusFBW/MCDU1Key8"},
    {40, "KEY9", "AirbusFBW/MCDU1Key9"},
    {41, "DOT", "AirbusFBW/MCDU1KeyDecimal"},
    {42, "KEY0", "AirbusFBW/MCDU1Key0"},
    {43, "PLUSMINUS", "AirbusFBW/MCDU1KeyPM"},
    {44, "KEYA", "AirbusFBW/MCDU1KeyA"},
    {45, "KEYB", "AirbusFBW/MCDU1KeyB"},
    {46, "KEYC", "AirbusFBW/MCDU1KeyC"},
    {47, "KEYD", "AirbusFBW/MCDU1KeyD"},
    {48, "KEYE", "AirbusFBW/MCDU1KeyE"},
    {49, "KEYF", "AirbusFBW/MCDU1KeyF"},
    {50, "KEYG", "AirbusFBW/MCDU1KeyG"},
    {51, "KEYH", "AirbusFBW/MCDU1KeyH"},
    {52, "KEYI", "AirbusFBW/MCDU1KeyI"},
    {53, "KEYJ", "AirbusFBW/MCDU1KeyJ"},
    {54, "KEYK", "AirbusFBW/MCDU1KeyK"},
    {55, "KEYL", "AirbusFBW/MCDU1KeyL"},
    {56, "KEYM", "AirbusFBW/MCDU1KeyM"},
    {57, "KEYN", "AirbusFBW/MCDU1KeyN"},
    {58, "KEYO", "AirbusFBW/MCDU1KeyO"},
    {59, "KEYP", "AirbusFBW/MCDU1KeyP"},
    {60, "KEYQ", "AirbusFBW/MCDU1KeyQ"},
    {61, "KEYR", "AirbusFBW/MCDU1KeyR"},
    {62, "KEYS", "AirbusFBW/MCDU1KeyS"},
    {63, "KEYT", "AirbusFBW/MCDU1KeyT"},
    {64, "KEYU", "AirbusFBW/MCDU1KeyU"},
    {65, "KEYV", "AirbusFBW/MCDU1KeyV"},
    {66, "KEYW", "AirbusFBW/MCDU1KeyW"},
    {67, "KEYX", "AirbusFBW/MCDU1KeyX"},
    {68, "KEYY", "AirbusFBW/MCDU1KeyY"},
    {69, "KEYZ", "AirbusFBW/MCDU1KeyZ"},
    {70, "SLASH", "AirbusFBW/MCDU1KeySlash"},
    {71, "SPACE", "AirbusFBW/MCDU1KeySpace"},
    {72, "OVERFLY", "AirbusFBW/MCDU1KeyOverfly"},
    {73, "Clear", "AirbusFBW/MCDU1KeyClear"},
};

// List of datarefs without led connection to request.
// Text Dataref format:  <MCDU[1,2]><Line[title/label/cont/etc]><Linenumber[1...6]><Color[a,b,m,s,w,y]>.
// We must read all 25 Bytes per dataref!
static std::vector<std::string> datarefs = {
    //"AirbusFBW/MCDU1titleb",
    "AirbusFBW/MCDU1titleg",
    "AirbusFBW/MCDU1titles",
    "AirbusFBW/MCDU1titlew",
    //"AirbusFBW/MCDU1titley",
    "AirbusFBW/MCDU1stitley",
    "AirbusFBW/MCDU1stitlew",
    "AirbusFBW/MCDU1label1w",
    "AirbusFBW/MCDU1label2w",
    "AirbusFBW/MCDU1label3w",
    "AirbusFBW/MCDU1label4w",
    "AirbusFBW/MCDU1label5w",
    "AirbusFBW/MCDU1label6w",
    //"AirbusFBW/MCDU1label1a",
    "AirbusFBW/MCDU1label2a",
    "AirbusFBW/MCDU1label3a",
    //"AirbusFBW/MCDU1label4a",
    //"AirbusFBW/MCDU1label5a",
    "AirbusFBW/MCDU1label6a",
    "AirbusFBW/MCDU1label1g",
    "AirbusFBW/MCDU1label2g",
    "AirbusFBW/MCDU1label3g",
    "AirbusFBW/MCDU1label4g",
    "AirbusFBW/MCDU1label5g",
    //"AirbusFBW/MCDU1label6g",
    "AirbusFBW/MCDU1label1b",
    //"AirbusFBW/MCDU1label2b",
    "AirbusFBW/MCDU1label3b",
    "AirbusFBW/MCDU1label4b",
    "AirbusFBW/MCDU1label5b",
    "AirbusFBW/MCDU1label6b",
    //"AirbusFBW/MCDU1label1y",
    //"AirbusFBW/MCDU1label2y",
    //"AirbusFBW/MCDU1label3y",
    //"AirbusFBW/MCDU1label4y",
    //"AirbusFBW/MCDU1label5y",
    "AirbusFBW/MCDU1label6y",
    "AirbusFBW/MCDU1label1Lg",
    "AirbusFBW/MCDU1label2Lg",
    "AirbusFBW/MCDU1label3Lg",
    "AirbusFBW/MCDU1label4Lg",
    "AirbusFBW/MCDU1label5Lg",
    "AirbusFBW/MCDU1label6Lg",
    "AirbusFBW/MCDU1cont1b",
    "AirbusFBW/MCDU1cont2b",
    "AirbusFBW/MCDU1cont3b",
    "AirbusFBW/MCDU1cont4b",
    "AirbusFBW/MCDU1cont5b",
    "AirbusFBW/MCDU1cont6b",
    "AirbusFBW/MCDU1cont1m",
    "AirbusFBW/MCDU1cont2m",
    "AirbusFBW/MCDU1cont3m",
    //"AirbusFBW/MCDU1cont4m",
    //"AirbusFBW/MCDU1cont5m",
    "AirbusFBW/MCDU1cont6m",
    "AirbusFBW/MCDU1scont1m",
    "AirbusFBW/MCDU1scont2m",
    "AirbusFBW/MCDU1scont3m",
    "AirbusFBW/MCDU1scont4m",
    "AirbusFBW/MCDU1scont5m",
    "AirbusFBW/MCDU1scont6m",
    "AirbusFBW/MCDU1cont1a",
    "AirbusFBW/MCDU1cont2a",
    "AirbusFBW/MCDU1cont3a",
    "AirbusFBW/MCDU1cont4a",
    "AirbusFBW/MCDU1cont5a",
    "AirbusFBW/MCDU1cont6a",
    "AirbusFBW/MCDU1scont1a",
    "AirbusFBW/MCDU1scont2a",
    "AirbusFBW/MCDU1scont3a",
    "AirbusFBW/MCDU1scont4a",
    "AirbusFBW/MCDU1scont5a",
    "AirbusFBW/MCDU1scont6a",
    "AirbusFBW/MCDU1cont1w",
    "AirbusFBW/MCDU1cont2w",
    "AirbusFBW/MCDU1cont3w",
    "AirbusFBW/MCDU1cont4w",
    "AirbusFBW/MCDU1cont5w",
    "AirbusFBW/MCDU1cont6w",
    "AirbusFBW/MCDU1cont1g",
    "AirbusFBW/MCDU1cont2g",
    "AirbusFBW/MCDU1cont3g",
    "AirbusFBW/MCDU1cont4g",
    "AirbusFBW/MCDU1cont5g",
    "AirbusFBW/MCDU1cont6g",
//        "AirbusFBW/MCDU1cont1c",
//        "AirbusFBW/MCDU1cont2c",
//        "AirbusFBW/MCDU1cont3c",
//        "AirbusFBW/MCDU1cont4c",
    //"AirbusFBW/MCDU1cont5c",
    //"AirbusFBW/MCDU1cont6c",
    "AirbusFBW/MCDU1scont1g",
    "AirbusFBW/MCDU1scont2g",
    "AirbusFBW/MCDU1scont3g",
    "AirbusFBW/MCDU1scont4g",
    "AirbusFBW/MCDU1scont5g",
    "AirbusFBW/MCDU1scont6g",
    "AirbusFBW/MCDU1cont1s",
    "AirbusFBW/MCDU1cont2s",
    "AirbusFBW/MCDU1cont3s",
    "AirbusFBW/MCDU1cont4s",
    "AirbusFBW/MCDU1cont5s",
    "AirbusFBW/MCDU1cont6s",
    "AirbusFBW/MCDU1scont1b",
    "AirbusFBW/MCDU1scont2b",
    "AirbusFBW/MCDU1scont3b",
    "AirbusFBW/MCDU1scont4b",
    "AirbusFBW/MCDU1scont5b",
    "AirbusFBW/MCDU1scont6b",
    "AirbusFBW/MCDU1cont1y",
    "AirbusFBW/MCDU1cont2y",
    "AirbusFBW/MCDU1cont3y",
    "AirbusFBW/MCDU1cont4y",
    "AirbusFBW/MCDU1cont5y",
    "AirbusFBW/MCDU1cont6y",
    "AirbusFBW/MCDU1scont1w",
    "AirbusFBW/MCDU1scont2w",
    "AirbusFBW/MCDU1scont3w",
    "AirbusFBW/MCDU1scont4w",
    "AirbusFBW/MCDU1scont5w",
    "AirbusFBW/MCDU1scont6w",
    "AirbusFBW/MCDU1scont1y",
    "AirbusFBW/MCDU1scont2y",
    "AirbusFBW/MCDU1scont3y",
    "AirbusFBW/MCDU1scont4y",
    //"AirbusFBW/MCDU1scont5y",
    //"AirbusFBW/MCDU1scont6y",
    
    "AirbusFBW/MCDU1spw", // scratchpad
    "AirbusFBW/MCDU1spa" // scratchpad
};

ProductMCDU::ProductMCDU(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    page = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_BYTES_PER_LINE, ' '));
    previousPage = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_BYTES_PER_LINE, ' '));
    // Pre-compile the regex pattern
    datarefRegex = std::regex("AirbusFBW/MCDU(1|2)([s]{0,1})([a-zA-Z]+)([0-6]{0,1})([L]{0,1})([a-z]{1})");
    connect();
}

ProductMCDU::~ProductMCDU() {
    disconnect();
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
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("AirbusFBW/DUBrightness");
    Dataref::getInstance()->unbind("AirbusFBW/MCDUIntegBrightness");
    didInitializeDatarefs = false;
    
    // Clear caches
    cachedDatarefValues.clear();
}

void ProductMCDU::update() {
    if (!connected) {
        return;
    }
    
    if (!didInitializeDatarefs) {
        initializeDatarefs();
    }
    USBDevice::update();
    updatePage();
}

void ProductMCDU::didReceiveData(int reportId, uint8_t *report, int reportLength) {
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
    
    constexpr int numButtons = sizeof(buttonDefs) / sizeof(ButtonDef);
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
            Dataref::getInstance()->executeCommand(buttonDefs[i].dataref, xplm_CommandBegin);
        }
        else if (pressed && pressedButtonIndexExists) {
            Dataref::getInstance()->executeCommand(buttonDefs[i].dataref, xplm_CommandContinue);
        }
        else if (!pressed && pressedButtonIndexExists) {
            pressedButtonIndices.erase(std::remove(pressedButtonIndices.begin(), pressedButtonIndices.end(), i), pressedButtonIndices.end());
            Dataref::getInstance()->executeCommand(buttonDefs[i].dataref, xplm_CommandEnd);
        }
    }
}

void ProductMCDU::updatePage() {
    if (!didInitializeDatarefs) {
        return;
    }
    
    std::array<int, PAGE_BYTES_PER_LINE> spw_line{};
    std::array<int, PAGE_BYTES_PER_LINE> spa_line{};
    int vertslew_key = 0;

    bool anyDatarefChanged = false;
    for (const std::string &ref : datarefs) {
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

    // Clear only changed areas instead of entire page
    for (int i = 0; i < PAGE_LINES; ++i) {
        std::fill(page[i].begin(), page[i].end(), ' ');
    }

    for (const std::string &ref : datarefs) {
        bool isScratchpad = (ref.size() >= 3 && 
                           (ref.substr(ref.size() - 3) == "spw" || ref.substr(ref.size() - 3) == "spa"));
        std::smatch match;
        
        // Use pre-compiled regex
        if (!std::regex_match(ref, match, datarefRegex) && !isScratchpad) {
            continue;
        }
        
        unsigned char mcduIndex = std::stoi(match[1]);
        if (mcduIndex != 1) {
            continue;
        }

        std::string type = match[3];
        unsigned char line = match[4].str().empty() ? 0 : std::stoi(match[4]) * 2;
        char color = match[6].str()[0];
        bool fontSmall = match[2] == "s" || (type == "label" && match[5] != "L") || color == 's';

        // Use cached value
        const std::string &text = cachedDatarefValues[ref];
        
        if (text.empty()) {
            continue;
        }

        // Process text characters
        for (size_t i = 0; i < text.size(); ++i) {
            char c = text[i];
            if (c == 0x00 || (c == 0x20 && !isScratchpad)) {
                continue;
            }

            unsigned char targetColor = color;
            if (color == 's') {
                switch (c) {
                    case 'A': c = 91; targetColor = 'b'; break;
                    case 'B': c = 93; targetColor = 'b'; break;
                    case '0': c = 60; targetColor = 'b'; break;
                    case '1': c = 62; targetColor = 'b'; break;
                    case '2': c = 60; targetColor = 'w'; break;
                    case '3': c = 62; targetColor = 'w'; break;
                    case '4': c = 60; targetColor = 'a'; break;
                    case '5': c = 62; targetColor = 'a'; break;
                    case 'E': c = 35; targetColor = 'a'; break;
                }
            }

            if (type.find("title") != std::string::npos || type.find("stitle") != std::string::npos) {
                writeLineToPage(0, i, std::string(1, c), targetColor, fontSmall);
            } else if (type.find("label") != std::string::npos) {
                unsigned char lbl_line = (match[4].str().empty() ? 1 : std::stoi(match[4])) * 2 - 1;
                writeLineToPage(lbl_line, i, std::string(1, c), targetColor, fontSmall);
            } else if (type.find("cont") != std::string::npos || type.find("scont") != std::string::npos) {
                writeLineToPage(line, i, std::string(1, c), targetColor, fontSmall);
            } else if (isScratchpad) {
                if (ref.size() >= 3 && ref.substr(ref.size() - 3) == "spw") {
                    spw_line[i] = c;
                } else {
                    if (i <= 21) {
                        spa_line[i] = c;
                    }
                }
            }

            if (ref.find("VertSlewKeys") != std::string::npos) {
                vertslew_key = c;
            }
        }
    }

    // Process scratchpad data
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
        if (spw_line[i] == 0) {
            std::fill(spw_line.begin() + i, spw_line.end(), 0);
            break;
        }
    }
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
        if (spa_line[i] == 0) {
            std::fill(spa_line.begin() + i, spa_line.end(), 0);
            break;
        }
    }

    // Merge spw and spa into line 13
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
        char disp_char = ' ';
        char disp_color = 'w';
        if (spw_line[i] != 0 && spa_line[i] == 0) {
            disp_char = spw_line[i];
            disp_color = 'w';
        } else if (spa_line[i] != 0) {
            disp_char = spa_line[i];
            disp_color = 'a';
        }
        writeLineToPage(13, i, std::string(1, disp_char), disp_color, false);
    }
    
    // Check if page actually changed before drawing
    if (page != previousPage) {
        previousPage = page;
        draw();
    }
}

void ProductMCDU::writeLineToPage(int line, int pos, const std::string &text, char color, bool fontSmall) {
    if (line < 0 || line >= PAGE_LINES) {
        //debug("Not writing line %i: Line number is out of range!\n", line);
        return;
    }
    if (pos < 0 || pos + text.length() > PAGE_CHARS_PER_LINE) {
        //debug("Not writing line %i: Position number (%i) is out of range!\n", line, pos);
        return;
    }
    if (text.length() > PAGE_CHARS_PER_LINE) {
        //debug("Not writing line %i: Text is too long (%lu) for line.\n", line, text.length());
        return;
    }

    pos = pos * PAGE_BYTES_PER_CHAR;
    size_t textLen = text.length();
    for (size_t c = 0; c < textLen; ++c) {
        int pagePos = pos + c * PAGE_BYTES_PER_CHAR;
        page[line][pagePos] = color;
        page[line][pagePos + 1] = fontSmall;
        page[line][pagePos + PAGE_BYTES_PER_CHAR - 1] = text[c];
    }
}

void ProductMCDU::draw(const std::vector<std::vector<char>> *pagePtr, int vertslew_key) {
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
                case 96: // Change to °
                    buf.insert(buf.end(), {0xc2, 0xb0});
                    break;
                default:
                default_case:
                    if (i == PAGE_LINES - 1 && j == PAGE_CHARS_PER_LINE - 2 && (vertslew_key == 1 || vertslew_key == 2))
                        buf.insert(buf.end(), {0xe2, 0x86, 0x91});
                    else if (i == PAGE_LINES - 1 && j == PAGE_CHARS_PER_LINE - 1 && (vertslew_key == 1 || vertslew_key == 3))
                        buf.insert(buf.end(), {0xe2, 0x86, 0x93});
                    else
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
    static const std::map<char, int> col_map = {
        {'L', 0x0000},
        {'A', 0x0021},
        {'W', 0x0042},
        {'B', 0x0063},
        {'G', 0x0084},
        {'M', 0x00A5},
        {'R', 0x00C6},
        {'Y', 0x00E7},
        {'E', 0x0108},
        {' ', 0x0042},
    };

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

void ProductMCDU::setLedBrightness(MCDULed led, uint8_t brightness) {
    writeData({0x02, 0x32, 0xbb, 0, 0, 3, 0x49, static_cast<uint8_t>(led), brightness, 0, 0, 0, 0, 0});
}

void ProductMCDU::initializeDatarefs() {
    if (!Dataref::getInstance()->exists("AirbusFBW/MCDUIntegBrightness")) {
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
    
    didInitializeDatarefs = true;
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/MCDUIntegBrightness", [this](std::vector<float> brightness) {
        if (brightness.size() < 4) {
            return;
        }
        
        uint8_t target = brightness[0] * 255.0f;
        setLedBrightness(MCDULed::BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/DUBrightness", [this](std::vector<float> brightness) {
        if (brightness.size() < 8) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[6] * 255.0f : 0;
        setLedBrightness(MCDULed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/DUBrightness");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/MCDUIntegBrightness");
    });
}
