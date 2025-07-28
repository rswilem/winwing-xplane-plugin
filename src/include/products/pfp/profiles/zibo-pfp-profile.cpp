#include "zibo-pfp-profile.h"
#include "product-mcdu.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>

constexpr unsigned int PAGE_LINES = 14;
constexpr unsigned int PAGE_CHARS_PER_LINE = 24;
constexpr unsigned int PAGE_BYTES_PER_CHAR = 3;

ZiboPfpProfile::ZiboPfpProfile() {
    const PFPLed ledsToSet[] = {
        PFPLed::CALL,
        PFPLed::FAIL,
        PFPLed::MSG,
        PFPLed::OFST,
    };

    for (auto led : ledsToSet) {
        if (ledBrightnessCallback) {
            ledBrightnessCallback(led, 0);
        }
    }
        
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/instrument_brightness", [this](std::vector<float> brightness) {
        if (brightness.size() < 27 || !ledBrightnessCallback) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[10] * 255.0f : 0;
        ledBrightnessCallback(PFPLed::BACKLIGHT, target);
        ledBrightnessCallback(PFPLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/instrument_brightness");
    });
}

ZiboPfpProfile::~ZiboPfpProfile() {
    Dataref::getInstance()->unbind("laminar/B738/electric/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool ZiboPfpProfile::IsEligible() {
    return Dataref::getInstance()->exists("laminar/B738/electric/instrument_brightness");
}

const std::vector<std::string>& ZiboPfpProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
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
    
    return datarefs;
}

const std::vector<PFPButtonDef>& ZiboPfpProfile::buttonDefs() const {
    static const std::vector<PFPButtonDef> buttons = {
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
        {17, "BRT-", ""},
        {18, "BRT+", ""},
        {19, "MENU", "laminar/B738/button/fmc1_menu"},
        {20, "LEGS", "laminar/B738/button/fmc1_legs"},
        {21, "DEPARR", "laminar/B738/button/fmc1_dep_app"},
        {22, "HOLD", "laminar/B738/button/fmc1_hold"},
        {23, "PROG", "laminar/B738/button/fmc1_prog"},
        {24, "EXEC", "laminar/B738/button/fmc1_exec"},
        {25, "N1LIMIT", "laminar/B738/button/fmc1_n1_lim"},
        {26, "FIX", "laminar/B738/button/fmc1_fix"},
        {27, "PREV_PAGE", "laminar/B738/button/fmc1_prev_page"},
        {28, "NEXT_PAGE", "laminar/B738/button/fmc1_next_page"},
        {29, "KEY1", "laminar/B738/button/fmc1_1"},
        {30, "KEY2", "laminar/B738/button/fmc1_2"},
        {31, "KEY3", "laminar/B738/button/fmc1_3"},
        {32, "KEY4", "laminar/B738/button/fmc1_4"},
        {33, "KEY5", "laminar/B738/button/fmc1_5"},
        {34, "KEY6", "laminar/B738/button/fmc1_6"},
        {35, "KEY7", "laminar/B738/button/fmc1_7"},
        {36, "KEY8", "laminar/B738/button/fmc1_8"},
        {37, "KEY9", "laminar/B738/button/fmc1_9"},
        {38, "PERIOD", "laminar/B738/button/fmc1_period"},
        {39, "KEY0", "laminar/B738/button/fmc1_0"},
        {40, "PLUSMINUS", "laminar/B738/button/fmc1_minus"},
        {41, "KEYA", "laminar/B738/button/fmc1_A"},
        {42, "KEYB", "laminar/B738/button/fmc1_B"},
        {43, "KEYC", "laminar/B738/button/fmc1_C"},
        {44, "KEYD", "laminar/B738/button/fmc1_D"},
        {45, "KEYE", "laminar/B738/button/fmc1_E"},
        {46, "KEYF", "laminar/B738/button/fmc1_F"},
        {47, "KEYG", "laminar/B738/button/fmc1_G"},
        {48, "KEYH", "laminar/B738/button/fmc1_H"},
        {49, "KEYI", "laminar/B738/button/fmc1_I"},
        {50, "KEYJ", "laminar/B738/button/fmc1_J"},
        {51, "KEYK", "laminar/B738/button/fmc1_K"},
        {52, "KEYL", "laminar/B738/button/fmc1_L"},
        {53, "KEYM", "laminar/B738/button/fmc1_M"},
        {54, "KEYN", "laminar/B738/button/fmc1_N"},
        {55, "KEYO", "laminar/B738/button/fmc1_O"},
        {56, "KEYP", "laminar/B738/button/fmc1_P"},
        {57, "KEYQ", "laminar/B738/button/fmc1_Q"},
        {58, "KEYR", "laminar/B738/button/fmc1_R"},
        {59, "KEYS", "laminar/B738/button/fmc1_S"},
        {60, "KEYT", "laminar/B738/button/fmc1_T"},
        {61, "KEYU", "laminar/B738/button/fmc1_U"},
        {62, "KEYV", "laminar/B738/button/fmc1_V"},
        {63, "KEYW", "laminar/B738/button/fmc1_W"},
        {64, "KEYX", "laminar/B738/button/fmc1_X"},
        {65, "KEYY", "laminar/B738/button/fmc1_Y"},
        {66, "KEYZ", "laminar/B738/button/fmc1_Z"},
        {67, "SPACE", "laminar/B738/button/fmc1_SP"},
        {68, "DEL", "laminar/B738/button/fmc1_del"},
        {69, "SLASH", "laminar/B738/button/fmc1_slash"},
        {70, "CLR", "laminar/B738/button/fmc1_clr"}
    };
    
    return buttons;
}

const std::map<char, int>& ZiboPfpProfile::colorMap() const {
    static const std::map<char, int> colMap = {
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
    
    return colMap;
}

void ZiboPfpProfile::updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) {
    page = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_CHARS_PER_LINE * PAGE_BYTES_PER_CHAR, ' '));
    
    for (const auto& [dataref, text] : cachedDatarefValues) {
        // Handle scratchpad datarefs specially
        if (dataref == "laminar/B738/fmc1/Line_entry" || dataref == "laminar/B738/fmc1/Line_entry_I") {
            if (!text.empty()) {
                char color = (dataref == "laminar/B738/fmc1/Line_entry_I") ? 'I' : 'W';
                
                // Store scratchpad text for later display on line 13
                for (int i = 0; i < text.size() && i < PAGE_CHARS_PER_LINE; ++i) {
                    char c = text[i];
                    if (c == 0x00) {
                        break; // End of string
                    }
                    if (c != 0x20) { // Skip spaces
                        writeLineToPage(page, 13, i, std::string(1, c), color, false);
                    }
                }
            }
            continue;
        }
        
        std::regex rgx("laminar/B738/fmc1/Line([0-9]{2})_([A-Z]+)");
        std::smatch match;
        if (!std::regex_match(dataref, match, rgx)) {
            continue;
        }
        
        unsigned char lineNum = std::stoi(match[1]);
        std::string colorStr = match[2];
        
        // For double-letter codes like "GX", "LX", use first letter for color
        char color = colorStr[0];
        
        unsigned char displayLine = lineNum * 2;
        bool fontSmall = color == 'X' || color == 'S';
        if (colorStr.back() == 'X') {
            displayLine -= 1; // X datarefs go to odd lines (labels)
        }

        if (text.empty()) {
            continue;
        }

        #if DEBUG
        printf("[PFP] %s: {", dataref.c_str());
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
                writeLineToPage(page, displayLine, i, std::string(1, c), color, fontSmall);
            }
        }
    }
}

void ZiboPfpProfile::writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color, bool fontSmall) {
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
