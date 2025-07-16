#include "laminar-mcdu-profile.h"
#include "product-mcdu.h"
#include "dataref.h"
#include <cstring>
#include <algorithm>

constexpr unsigned int PAGE_LINES = 14;
constexpr unsigned int PAGE_CHARS_PER_LINE = 24;
constexpr unsigned int PAGE_BYTES_PER_CHAR = 3;

LaminarMcduProfile::LaminarMcduProfile() {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("sim/cockpit2/electrical/instrument_brightness_ratio", [this](std::vector<float> brightness) {
        if (!ledBrightnessCallback || brightness.size() <= 6) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/avionics_on") ? brightness[6] * 255.0f : 0;
        ledBrightnessCallback(MCDULed::BACKLIGHT, target);
        ledBrightnessCallback(MCDULed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/instrument_brightness_ratio");
    });
}

LaminarMcduProfile::~LaminarMcduProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("sim/cockpit2/electrical/instrument_brightness_ratio");
}

bool LaminarMcduProfile::IsEligible() {
    std::string icao = Dataref::getInstance()->get<std::string>("sim/aircraft/view/acf_ICAO");
    icao.erase(std::find(icao.begin(), icao.end(), '\0'), icao.end());
    return Dataref::getInstance()->exists("sim/cockpit2/radios/indicators/fms_cdu1_text_line0") && icao == "A333";
}

const std::vector<std::string>& LaminarMcduProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // Text content for lines 0-15
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line0",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line1",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line2",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line3",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line4",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line5",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line6",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line7",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line8",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line9",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line10",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line11",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line12",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line13",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line14",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line15"
    };
    return datarefs;
}

const std::vector<MCDUButtonDef>& LaminarMcduProfile::buttonDefs() const {
    static const std::vector<MCDUButtonDef> buttons = {
        {0, "LSK1L", "sim/FMS/ls_1l"},
        {1, "LSK2L", "sim/FMS/ls_2l"},
        {2, "LSK3L", "sim/FMS/ls_3l"},
        {3, "LSK4L", "sim/FMS/ls_4l"},
        {4, "LSK5L", "sim/FMS/ls_5l"},
        {5, "LSK6L", "sim/FMS/ls_6l"},
        {6, "LSK1R", "sim/FMS/ls_1r"},
        {7, "LSK2R", "sim/FMS/ls_2r"},
        {8, "LSK3R", "sim/FMS/ls_3r"},
        {9, "LSK4R", "sim/FMS/ls_4r"},
        {10, "LSK5R", "sim/FMS/ls_5r"},
        {11, "LSK6R", "sim/FMS/ls_6r"},
        {12, "DIRTO", "sim/FMS/dir_intc"},
        {13, "PROG", "sim/FMS/prog"},
        {14, "PERF", "sim/FMS/perf"},
        {15, "INIT", "sim/FMS/index"},
        {16, "DATA", "sim/FMS/data"},
        {17, "EMPTY_TOP_RIGHT", ""},
        {18, "BRT", "laminar/A333/buttons/fms1_brightness_up"},
        {19, "FPLN", "sim/FMS/fpln"},
        {20, "RADNAV", "sim/FMS/navrad"},
        {21, "FUEL", "sim/FMS/fuel_pred"},
        {22, "SEC-FPLN", ""},
        {23, "ATC", ""},
        {24, "MENU", "sim/FMS/menu"},
        {25, "DIM", "laminar/A333/buttons/fms1_brightness_dn"},
        {26, "AIRPORT", "sim/FMS/airport"},
        {27, "EMPTY_BOTTOM_LEFT", ""},
        {28, "SLEW_LEFT", "sim/FMS/prev"},
        {29, "SLEW_UP", "sim/FMS/up"},
        {30, "SLEW_RIGHT", "sim/FMS/next"},
        {31, "SLEW_DOWN", "sim/FMS/down"},
        {32, "KEY1", "sim/FMS/key_1"},
        {33, "KEY2", "sim/FMS/key_2"},
        {34, "KEY3", "sim/FMS/key_1"},
        {35, "KEY4", "sim/FMS/key_1"},
        {36, "KEY5", "sim/FMS/key_1"},
        {37, "KEY6", "sim/FMS/key_1"},
        {38, "KEY7", "sim/FMS/key_1"},
        {39, "KEY8", "sim/FMS/key_1"},
        {40, "KEY9", "sim/FMS/key_1"},
        {41, "DOT", "sim/FMS/key_period"},
        {42, "KEY0", "sim/FMS/key_0"},
        {43, "PLUSMINUS", "sim/FMS/key_minus"},
        {44, "KEYA", "sim/FMS/key_A"},
        {45, "KEYB", "sim/FMS/key_B"},
        {46, "KEYC", "sim/FMS/key_C"},
        {47, "KEYD", "sim/FMS/key_D"},
        {48, "KEYE", "sim/FMS/key_E"},
        {49, "KEYF", "sim/FMS/key_F"},
        {50, "KEYG", "sim/FMS/key_G"},
        {51, "KEYH", "sim/FMS/key_H"},
        {52, "KEYI", "sim/FMS/key_I"},
        {53, "KEYJ", "sim/FMS/key_J"},
        {54, "KEYK", "sim/FMS/key_K"},
        {55, "KEYL", "sim/FMS/key_L"},
        {56, "KEYM", "sim/FMS/key_M"},
        {57, "KEYN", "sim/FMS/key_N"},
        {58, "KEYO", "sim/FMS/key_O"},
        {59, "KEYP", "sim/FMS/key_P"},
        {60, "KEYQ", "sim/FMS/key_Q"},
        {61, "KEYR", "sim/FMS/key_R"},
        {62, "KEYS", "sim/FMS/key_S"},
        {63, "KEYT", "sim/FMS/key_T"},
        {64, "KEYU", "sim/FMS/key_U"},
        {65, "KEYV", "sim/FMS/key_V"},
        {66, "KEYW", "sim/FMS/key_W"},
        {67, "KEYX", "sim/FMS/key_X"},
        {68, "KEYY", "sim/FMS/key_Y"},
        {69, "KEYZ", "sim/FMS/key_Z"},
        {70, "SLASH", "sim/FMS/key_slash"},
        {71, "SPACE", "sim/FMS/key_space"},
        {72, "OVERFLY", "sim/FMS/key_overfly"},
        {73, "Clear", "sim/FMS/key_clear"},
    };
    return buttons;
}

const std::map<char, int>& LaminarMcduProfile::colorMap() const {
    static const std::map<char, int> colMap = {
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
    return colMap;
}

void LaminarMcduProfile::updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) {
    // Clear the page
    for (int i = 0; i < PAGE_LINES; ++i) {
        std::fill(page[i].begin(), page[i].end(), ' ');
    }

    // Process lines 0-13 (we have 14 lines available, lines 0-15 from Laminar map to 0-13)
    for (int lineNum = 0; lineNum < std::min(PAGE_LINES, (unsigned int)16); ++lineNum) {
        // Get text content for this line
        std::string textDataref = "sim/cockpit2/radios/indicators/fms_cdu1_text_line" + std::to_string(lineNum);
        std::string styleDataref = "sim/cockpit2/radios/indicators/fms_cdu1_style_line" + std::to_string(lineNum);
        
        auto textIt = cachedDatarefValues.find(textDataref);
        if (textIt == cachedDatarefValues.end()) {
            continue;
        }
        
        std::string text = textIt->second;
        if (text.empty()) {
            continue;
        }
        
        // Get style bytes directly from dataref system, not from cached string values
        std::vector<unsigned char> styleBytes = Dataref::getInstance()->get<std::vector<unsigned char>>(styleDataref.c_str());

        // Replace all special characters with placeholders
        const std::vector<std::pair<std::string, unsigned char>> symbols = {
            { "\u2190", '<' }, // Left arrow
            { "\u2192", '>' }, // Right arrow
            { "\u2191", 30 }, // Up arrow
            { "\u2193", 31 }, // Down arrow
            { "\u2610", '#' }, // Ballot box
            { "\u00B0", 96 }
        };
        
        for (const auto& symbol : symbols) {
            size_t pos = 0;
            while ((pos = text.find(symbol.first, pos)) != std::string::npos) {
              text.replace(pos, symbol.first.length(), std::string(1, static_cast<char>(symbol.second)));
              pos += 1;
            }
        }
        // Replace any remaining non-ASCII characters with '?'
        for (size_t i = 0; i < text.size(); ++i) {
            if (static_cast<unsigned char>(text[i]) > 127) {
                text[i] = '?';
            }
        }

#if DEBUG
        printf("[LaminarMCDU] Line %d: text='%s', style=0x", lineNum, text.c_str());
        if (styleBytes.empty()) {
            printf("--------");
        } else {
            for (unsigned char ch : styleBytes) {
                printf("%02X", ch);
            }
        }
        printf("\n");
#endif

        // Process each character in the line
        for (int i = 0; i < text.size() && i < PAGE_CHARS_PER_LINE; ++i) {
            char c = text[i];
            if (c == 0x00) {
                continue;
            }

            // Default color and font settings
            char color = 'W'; // Default to white
            bool fontSmall = false;

            // Parse style information if available
            unsigned char styleByte = (i < styleBytes.size()) ? styleBytes[i] : 0x00;
            switch (styleByte & 0x0F) {
                case 0x01:
                    color = 'B';
                    break;
                    
                case 0x04:
                    color = 'G';
                    break;
                    /*        {'L', 0x0000},
                     {'A', 0x0021},
                     {'W', 0x0042},
                     {'B', 0x0063},
                     {'G', 0x0084},
                     {'M', 0x00A5},
                     {'R', 0x00C6},
                     {'Y', 0x00E7},
                     {'E', 0x0108},*/
                    
                case 0x06:
                    color = 'A';
                    break;
                    
                case 0x07:
                    fontSmall = (styleByte & 0xF0) == 0x00;
                    break;
                    
                default:
                    break;
            }
            
            int displayLine = lineNum;
            if (displayLine >= PAGE_LINES) {
                break;
            }

            writeLineToPage(page, displayLine, i, std::string(1, c), color, fontSmall);
        }
    }
}

void LaminarMcduProfile::writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color, bool fontSmall) {
    if (line < 0 || line >= PAGE_LINES) {
        return;
    }
    if (pos < 0 || pos + text.length() > PAGE_CHARS_PER_LINE) {
        return;
    }
    if (text.length() > PAGE_CHARS_PER_LINE) {
        return;
    }

    pos = pos * PAGE_BYTES_PER_CHAR;
    size_t textLen = text.length();
    for (int c = 0; c < textLen; ++c) {
        int pagePos = pos + c * PAGE_BYTES_PER_CHAR;
        page[line][pagePos] = color;
        page[line][pagePos + 1] = fontSmall;
        page[line][pagePos + PAGE_BYTES_PER_CHAR - 1] = text[c];
    }
}

