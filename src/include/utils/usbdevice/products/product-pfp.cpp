#include "product-pfp.h"
#include "dataref.h"
#include "appstate.h"
#include <map>
#include <thread>
#include <regex>
#include <cctype>
#include <cstdio>

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
    // Zibo 737 FMC LSK/RSK buttons
    {0, "LSK1L", "laminar/B738/button/fmc1_1L"},
    {1, "LSK2L", "laminar/B738/button/fmc1_2L"},
    {2, "LSK3L", "laminar/B738/button/fmc1_3L"},
    {3, "LSK4L", "laminar/B738/button/fmc1_4L"},
    {4, "LSK5L", "laminar/B738/button/fmc1_5L"},
    {5, "LSK6L", "laminar/B738/button/fmc1_6L"},
    {6, "LSK1R", "laminar/B738/button/fmc1_1R"},
    {7, "LSK2R", "laminar/B738/button/fmc1_2R"},
    {8, "LSK3R", "laminar/B738/button/fmc1_3R"},
    {9, "LSK4R", "laminar/B738/button/fmc1_4R"},
    {10, "LSK5R", "laminar/B738/button/fmc1_5R"},
    {11, "LSK6R", "laminar/B738/button/fmc1_6R"},
    {12, "INITREF", "laminar/B738/button/fmc1_init_ref"},
    {13, "RTE", "laminar/B738/button/fmc1_rte"},
    {14, "CLB", "laminar/B738/button/fmc1_clb"},
    {15, "CRZ", "laminar/B738/button/fmc1_crz"},
    {16, "DES", "laminar/B738/button/fmc1_des"},
    {17, "UNUSED1", ""},
    {18, "BRT", ""},
    {19, "MENU", "laminar/B738/button/fmc1_menu"},
    {20, "LEGS", "laminar/B738/button/fmc1_legs"},
    {21, "DEPARR", "laminar/B738/button/fmc1_dep_app"},
    {22, "HOLD", "laminar/B738/button/fmc1_hold"},
    {23, "PROG", "laminar/B738/button/fmc1_prog"},
    {24, "EXEC", "laminar/B738/button/fmc1_exec"},
    {25, "DIM", ""},
    {26, "N1LIMIT", "laminar/B738/button/fmc1_n1_lim"},
    {27, "FIX", "laminar/B738/button/fmc1_fix"},
    {28, "PREV_PAGE", "laminar/B738/button/fmc1_prev_page"},
    {29, "NEXT_PAGE", "laminar/B738/button/fmc1_next_page"},
    {30, "UNUSED2", ""},
    {31, "UNUSED3", ""},
    {32, "KEY1", "laminar/B738/button/fmc1_1"},
    {33, "KEY2", "laminar/B738/button/fmc1_2"},
    {34, "KEY3", "laminar/B738/button/fmc1_3"},
    {35, "KEY4", "laminar/B738/button/fmc1_4"},
    {36, "KEY5", "laminar/B738/button/fmc1_5"},
    {37, "KEY6", "laminar/B738/button/fmc1_6"},
    {38, "KEY7", "laminar/B738/button/fmc1_7"},
    {39, "KEY8", "laminar/B738/button/fmc1_8"},
    {40, "KEY9", "laminar/B738/button/fmc1_9"},
    {41, "PERIOD", "laminar/B738/button/fmc1_period"},
    {42, "KEY0", "laminar/B738/button/fmc1_0"},
    {43, "PLUSMINUS", "laminar/B738/button/fmc1_minus"},
    {44, "KEYA", "laminar/B738/button/fmc1_A"},
    {45, "KEYB", "laminar/B738/button/fmc1_B"},
    {46, "KEYC", "laminar/B738/button/fmc1_C"},
    {47, "KEYD", "laminar/B738/button/fmc1_D"},
    {48, "KEYE", "laminar/B738/button/fmc1_E"},
    {49, "KEYF", "laminar/B738/button/fmc1_F"},
    {50, "KEYG", "laminar/B738/button/fmc1_G"},
    {51, "KEYH", "laminar/B738/button/fmc1_H"},
    {52, "KEYI", "laminar/B738/button/fmc1_I"},
    {53, "KEYJ", "laminar/B738/button/fmc1_J"},
    {54, "KEYK", "laminar/B738/button/fmc1_K"},
    {55, "KEYL", "laminar/B738/button/fmc1_L"},
    {56, "KEYM", "laminar/B738/button/fmc1_M"},
    {57, "KEYN", "laminar/B738/button/fmc1_N"},
    {58, "KEYO", "laminar/B738/button/fmc1_O"},
    {59, "KEYP", "laminar/B738/button/fmc1_P"},
    {60, "KEYQ", "laminar/B738/button/fmc1_Q"},
    {61, "KEYR", "laminar/B738/button/fmc1_R"},
    {62, "KEYS", "laminar/B738/button/fmc1_S"},
    {63, "KEYT", "laminar/B738/button/fmc1_T"},
    {64, "KEYU", "laminar/B738/button/fmc1_U"},
    {65, "KEYV", "laminar/B738/button/fmc1_V"},
    {66, "KEYW", "laminar/B738/button/fmc1_W"},
    {67, "KEYX", "laminar/B738/button/fmc1_X"},
    {68, "KEYY", "laminar/B738/button/fmc1_Y"},
    {69, "KEYZ", "laminar/B738/button/fmc1_Z"},
    {70, "SPACE", "laminar/B738/button/fmc1_SP"},
    {71, "DEL", "laminar/B738/button/fmc1_del"},
    {72, "SLASH", "laminar/B738/button/fmc1_slash"},
    {73, "CLR", "laminar/B738/button/fmc1_clr"},
};

// List of datarefs for Zibo Boeing 737 FMC display data.
// Format: laminar/B738/fmc1/Line<XX>_<S> where:
// - XX is the line number (00-06)
// - S is the display attribute (L=left, S=small, M=magenta, G=green)
// Each dataref provides 25 bytes of display data for the corresponding line and color.
// Lines 00-05 are the main display area (6 lines total), line 06 is scratchpad
static std::vector<std::string> datarefs = {
    "laminar/B738/fmc1/Line00_C",
    "laminar/B738/fmc1/Line00_G",
    "laminar/B738/fmc1/Line00_I",
    "laminar/B738/fmc1/Line00_L",
    "laminar/B738/fmc1/Line00_M",
    "laminar/B738/fmc1/Line00_S",
    
    "laminar/B738/fmc1/Line01_G",
    "laminar/B738/fmc1/Line01_GX",
    "laminar/B738/fmc1/Line01_I",
    "laminar/B738/fmc1/Line01_L",
    "laminar/B738/fmc1/Line01_LX",
    "laminar/B738/fmc1/Line01_M",
    "laminar/B738/fmc1/Line01_S", 
    "laminar/B738/fmc1/Line01_X",
    
    "laminar/B738/fmc1/Line02_G",
    "laminar/B738/fmc1/Line02_GX",
    "laminar/B738/fmc1/Line02_I",
    "laminar/B738/fmc1/Line02_L",
    "laminar/B738/fmc1/Line02_LX",
    "laminar/B738/fmc1/Line02_M",
    "laminar/B738/fmc1/Line02_S", 
    "laminar/B738/fmc1/Line02_X",
    
    "laminar/B738/fmc1/Line03_G",
    "laminar/B738/fmc1/Line03_GX",
    "laminar/B738/fmc1/Line03_I",
    "laminar/B738/fmc1/Line03_L",
    "laminar/B738/fmc1/Line03_LX",
    "laminar/B738/fmc1/Line03_M",
    "laminar/B738/fmc1/Line03_S", 
    "laminar/B738/fmc1/Line03_X",
    
    "laminar/B738/fmc1/Line04_G",
    "laminar/B738/fmc1/Line04_GX",
    "laminar/B738/fmc1/Line04_I",
    "laminar/B738/fmc1/Line04_L",
    "laminar/B738/fmc1/Line04_LX",
    "laminar/B738/fmc1/Line04_M",
    "laminar/B738/fmc1/Line04_S", 
    "laminar/B738/fmc1/Line04_SI", 
    "laminar/B738/fmc1/Line04_X",
    
    "laminar/B738/fmc1/Line05_G",
    "laminar/B738/fmc1/Line05_GX",
    "laminar/B738/fmc1/Line05_I",
    "laminar/B738/fmc1/Line05_L",
    "laminar/B738/fmc1/Line05_LX",
    "laminar/B738/fmc1/Line05_M",
    "laminar/B738/fmc1/Line05_S", 
    "laminar/B738/fmc1/Line05_X",
    
    "laminar/B738/fmc1/Line06_G",
    "laminar/B738/fmc1/Line06_GX",
    "laminar/B738/fmc1/Line06_I",
    "laminar/B738/fmc1/Line06_L",
    "laminar/B738/fmc1/Line06_LX",
    "laminar/B738/fmc1/Line06_M",
    "laminar/B738/fmc1/Line06_S", 
    "laminar/B738/fmc1/Line06_X",
    
    "laminar/B738/fmc1/Line_entry",
    "laminar/B738/fmc1/Line_entry_I"
};

ProductPFP::ProductPFP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) : USBDevice(hidDevice, vendorId, productId, vendorName, productName) {
    page = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_BYTES_PER_LINE, ' '));
    connect();
}

ProductPFP::~ProductPFP() {
    disconnect();
}

bool ProductPFP::connect() {
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
        
        setLedBrightness(PFPLed::BACKLIGHT, 128);
        setLedBrightness(PFPLed::SCREEN_BACKLIGHT, 128);
        const PFPLed ledsToSet[] = {
            PFPLed::CALL,
            PFPLed::FAIL,
            PFPLed::MSG,
            PFPLed::OFST,
        };

        for (auto led : ledsToSet) {
            setLedBrightness(led, 0);
        }
        
        clear2(8);
        
        return true;
    }
    
    return false;
}

void ProductPFP::disconnect() {
    const PFPLed ledsToSet[] = {
        PFPLed::BACKLIGHT,
        PFPLed::SCREEN_BACKLIGHT,
        PFPLed::CALL,
        PFPLed::FAIL,
        PFPLed::MSG,
        PFPLed::OFST,
    };

    for (auto led : ledsToSet) {
        setLedBrightness(led, 0);
    }
    
    USBDevice::disconnect();
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("laminar/B738/electric/instrument_brightness");
    didInitializeDatarefs = false;
}

void ProductPFP::update() {
    if (!connected) {
        return;
    }
    
    if (!didInitializeDatarefs) {
        initializeDatarefs();
    }
    USBDevice::update();
    updatePage();
}

void ProductPFP::didReceiveData(int reportId, uint8_t *report, int reportLength) {
    if (reportId != 1 || reportLength != 25) {
#if DEBUG
        printf("[PFP] Ignoring reportId %d, length %d\n", reportId, reportLength);
        printf("[PFP] Data (hex): ");
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

void ProductPFP::updatePage() {
    page = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_BYTES_PER_LINE, ' '));

    printf("[PFP] DATAREFS-------------------------------------------\n");
    for (const std::string &ref : datarefs) {
        // Handle scratchpad datarefs specially
        if (ref == "laminar/B738/fmc1/Line_entry" || ref == "laminar/B738/fmc1/Line_entry_I") {
            std::string text = Dataref::getInstance()->getCached<std::string>(ref.c_str());
            if (!text.empty()) {
                char color = (ref == "laminar/B738/fmc1/Line_entry_I") ? 'I' : 'W';
                
                // Store scratchpad text for later display on line 13
                for (int i = 0; i < text.size() && i < PAGE_CHARS_PER_LINE; ++i) {
                    char c = text[i];
                    if (c == 0x00) {
                        break; // End of string
                    }
                    if (c != 0x20) { // Skip spaces
                        writeLineToPage(13, i, std::string(1, c), color, false);
                    }
                }
            }
            continue;
        }
        
        // Parse Zibo Boeing 737 FMC dataref format: laminar/B738/fmc1/Line<XX>_<S>
        // Where S can be L/LX (large/normal), S/SI (small), M (magenta), G/GX (green), X (special), I (inverted), C (cyan)
        // Single-letter codes (L, S, M, G, C, I, X) and double-letter codes ending in X (GX, LX, etc.)
        std::regex rgx("laminar/B738/fmc1/Line([0-9]{2})_([A-Z]+)");
        std::smatch match;
        if (!std::regex_match(ref, match, rgx)) {
            continue;
        }
        
        // Parse line number and color attribute
        unsigned char lineNum = std::stoi(match[1]);
        std::string colorStr = match[2];
        
        // For double-letter codes like "GX", "LX", use first letter for color
        // For single-letter codes like "X", use as-is
        char color = colorStr[0];
        
        unsigned char displayLine = lineNum * 2;
        bool fontSmall = color == 'X' || color == 'S';
        
        // Check if color string ends with 'X' (e.g., "X", "GX", "LX")
        if (colorStr.back() == 'X') {
            displayLine -= 1; // X datarefs go to odd lines (labels)
        }

        std::string text = Dataref::getInstance()->getCached<std::string>(ref.c_str());
        
        if (text.empty()) {
            continue;
        }

        #if DEBUG
        printf("[PFP] %s: {", ref.c_str());
        for (unsigned char ch : text) {
            printf("0x%02X, ", static_cast<unsigned char>(ch));
        }
        printf("}\n");
        #endif

        // Process each character in the text
        for (int i = 0; i < text.size() && i < PAGE_CHARS_PER_LINE; ++i) {
            char c = text[i];
            if (c == 0x00) {
                break; // End of string
            }
            
            if (c != 0x20) {
                writeLineToPage(displayLine, i, std::string(1, c), color, fontSmall);
            }
        }
    }
    
    draw();
}

void ProductPFP::writeLineToPage(int line, int pos, const std::string &text, char color, bool fontSmall) {
    if (line < 0 || line >= PAGE_LINES) {
        debug("Not writing line %i: Line number is out of range!\n", line);
        return;
    }
    if (pos < 0 || pos + text.length() > PAGE_CHARS_PER_LINE) {
        debug("Not writing line %i: Position number (%i) is out of range!\n", line, pos);
        return;
    }
    if (text.length() > PAGE_CHARS_PER_LINE) {
        debug("Not writing line %i: Text is too long (%lu) for line.\n", line, text.length());
        return;
    }

    pos = pos * PAGE_BYTES_PER_CHAR;
    for (size_t c = 0; c < text.length(); ++c) {
        page[line][pos + c * PAGE_BYTES_PER_CHAR] = color;
        page[line][pos + c * PAGE_BYTES_PER_CHAR + 1] = fontSmall;
        page[line][pos + c * PAGE_BYTES_PER_CHAR + PAGE_BYTES_PER_CHAR - 1] = text[c];
    }
}

void ProductPFP::draw(const std::vector<std::vector<char>> *pagePtr) {
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
                // These still come from the MCDU. I saw =PRINT on the ACARS page, so maybe = is *
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

std::pair<uint8_t, uint8_t> ProductPFP::dataFromColFont(char color, bool fontSmall) {
    static const std::map<char, int> col_map = {
        // Zibo Boeing 737 FMC colors mapped to PFP display values
        {'L', 0x0042},  // Zibo L = Large/normal text (white)
        {'S', 0x0042},  // Zibo S = Small text (white)
        {'M', 0x00A5},  // Zibo M = Magenta
        {'G', 0x0084},  // Zibo G = Green
        {'C', 0x0063},  // Zibo C = Cyan (blue)
        {'I', 0x0042},  // Zibo I = Inverted (white for now)
        {'X', 0x0042},  // Zibo X = Special/labels (white)
        
        // Fallback
        {' ', 0x0042},  // Space = white
        {'W', 0x0042},  // White fallback
    };

    auto it = col_map.find(color);
    if (it == col_map.end()) {
        // Default to white if color not found
        debug("Unknown color '%c', defaulting to white\n", color);
        it = col_map.find('W');
    }

    int value = it->second;
    if (fontSmall) {
        value += 0x016b;
    }

    uint8_t dataLow = value & 0xFF;
    uint8_t dataHigh = (value >> 8) & 0xFF;
    return {dataLow, dataHigh};
}

void ProductPFP::clear() {
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

void ProductPFP::clear2(unsigned char variant) {
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

void ProductPFP::setLedBrightness(PFPLed led, uint8_t brightness) {
    writeData({0x02, 0x32, 0xbb, 0, 0, 3, 0x49, static_cast<uint8_t>(led), brightness, 0, 0, 0, 0, 0});
}

void ProductPFP::initializeDatarefs() {
    if (!Dataref::getInstance()->exists("laminar/B738/electric/instrument_brightness")) {
        return;
    }
    
    const PFPLed ledsToSet[] = {
        PFPLed::CALL,
        PFPLed::FAIL,
        PFPLed::MSG,
        PFPLed::OFST,
    };

    for (auto led : ledsToSet) {
        setLedBrightness(led, 0);
    }
    
    didInitializeDatarefs = true;
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/instrument_brightness", [this](std::vector<float> brightness) {
        if (brightness.size() < 27) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[10] * 255.0f : 0;
        setLedBrightness(PFPLed::BACKLIGHT, target);
        setLedBrightness(PFPLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/instrument_brightness");
    });
}
