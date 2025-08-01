#include "ixeg733-pfp-profile.h"
#include "product-pfp.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>
#include <XPLMUtilities.h>
#include <XPLMDataAccess.h>

IXEG733PfpProfile::IXEG733PfpProfile(ProductPFP *product) : PfpAircraftProfile(product) {
    datarefRegex = std::regex("laminar/B738/fmc1/Line([0-9]{2})_([A-Z]+)");
    
    const PFPLed ledsToSet[] = {
        PFPLed::CALL,
        PFPLed::FAIL,
        PFPLed::MSG,
        PFPLed::OFST,
        PFPLed::EXEC
    };

    for (auto led : ledsToSet) {
        product->setLedBrightness(led, 0);
    }
        
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/instrument_brightness", [product](std::vector<float> brightness) {
        if (brightness.size() < 27) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[10] * 255.0f : 0;
        product->setLedBrightness(PFPLed::BACKLIGHT, target);
        product->setLedBrightness(PFPLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/instrument_brightness");
    });
}

IXEG733PfpProfile::~IXEG733PfpProfile() {
    Dataref::getInstance()->unbind("laminar/B738/electric/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool IXEG733PfpProfile::IsEligible() {
    return Dataref::getInstance()->exists("ixeg/733/FMC/cdu1_menu");
}

const std::vector<std::string>& IXEG733PfpProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "ixeg/733/FMC/cdu1D_pg_number", // Basically top-right
        "ixeg/733/FMC/cdu1D_title", // Always on the first line. Line zero, basically.
        "ixeg/733/FMC/cdu1D_line1L_d",
        "ixeg/733/FMC/cdu1D_line1L_t",
        "ixeg/733/FMC/cdu1D_line1R_d",
        "ixeg/733/FMC/cdu1D_line1R_t",
        "ixeg/733/FMC/cdu1D_line2L_d",
        "ixeg/733/FMC/cdu1D_line2L_t",
        "ixeg/733/FMC/cdu1D_line2R_d",
        "ixeg/733/FMC/cdu1D_line2R_t",
        "ixeg/733/FMC/cdu1D_line3L_d",
        "ixeg/733/FMC/cdu1D_line3L_t",
        "ixeg/733/FMC/cdu1D_line3R_d",
        "ixeg/733/FMC/cdu1D_line3R_t",
        "ixeg/733/FMC/cdu1D_line4L_d",
        "ixeg/733/FMC/cdu1D_line4L_t",
        "ixeg/733/FMC/cdu1D_line4R_d",
        "ixeg/733/FMC/cdu1D_line4R_t",
        "ixeg/733/FMC/cdu1D_line5L_d",
        "ixeg/733/FMC/cdu1D_line5L_t",
        "ixeg/733/FMC/cdu1D_line5R_d",
        "ixeg/733/FMC/cdu1D_line5R_t",
        "ixeg/733/FMC/cdu1D_line6L_d",
        "ixeg/733/FMC/cdu1D_line6L_t",
        "ixeg/733/FMC/cdu1D_line6R_d",
        "ixeg/733/FMC/cdu1D_line6R_t",

        "ixeg/733/FMC/cdu1D_scrpad",
    };
    
    return datarefs;
}

const std::vector<PFPButtonDef>& IXEG733PfpProfile::buttonDefs() const {
    static const std::vector<PFPButtonDef> sevenThreeSevenButtonLayout = {
        {0, "LSK1L", ""},
        {1, "LSK2L", ""},
        {2, "LSK3L", ""},
        {3, "LSK4L", ""},
        {4, "LSK5L", ""},
        {5, "LSK6L", ""},
        {6, "LSK1R", ""},
        {7, "LSK2R", ""},
        {8, "LSK3R", ""},
        {9, "LSK4R", ""},
        {10, "LSK5R", ""},
        {11, "LSK6R", ""},
        {12, "INITREF", "ixeg/733/FMC/cdu1_initref"},
        {13, "RTE", ""},
        {14, "CLB", ""},
        {15, "CRZ", ""},
        {16, "DES", ""},
        {17, "BRT-", ""},
        {18, "BRT+", ""},
        {19, "MENU", "ixeg/733/FMC/cdu1_menu"},
        {20, "LEGS", ""},
        {21, "DEPARR", "ixeg/733/FMC/cdu1_deparr"},
        {22, "HOLD", ""},
        {23, "PROG", ""},
        {24, "EXEC", ""},
        {25, "N1LIMIT", ""},
        {26, "FIX", ""},
        {27, "PREV_PAGE", ""},
        {28, "NEXT_PAGE", ""},
        {29, "KEY1", "ixeg/733/FMC/cdu1_1"},
        {30, "KEY2", "ixeg/733/FMC/cdu1_2"},
        {31, "KEY3", "ixeg/733/FMC/cdu1_3"},
        {32, "KEY4", "ixeg/733/FMC/cdu1_4"},
        {33, "KEY5", "ixeg/733/FMC/cdu1_5"},
        {34, "KEY6", "ixeg/733/FMC/cdu1_6"},
        {35, "KEY7", "ixeg/733/FMC/cdu1_7"},
        {36, "KEY8", "ixeg/733/FMC/cdu1_8"},
        {37, "KEY9", "ixeg/733/FMC/cdu1_9"},
        {38, "PERIOD", ""},
        {39, "KEY0", "ixeg/733/FMC/cdu1_0"},
        {40, "PLUSMINUS", ""},
        {41, "KEYA", "ixeg/733/FMC/cdu1_A"},
        {42, "KEYB", "ixeg/733/FMC/cdu1_B"},
        {43, "KEYC", "ixeg/733/FMC/cdu1_C"},
        {44, "KEYD", "ixeg/733/FMC/cdu1_D"},
        {45, "KEYE", "ixeg/733/FMC/cdu1_E"},
        {46, "KEYF", "ixeg/733/FMC/cdu1_F"},
        {47, "KEYG", "ixeg/733/FMC/cdu1_G"},
        {48, "KEYH", "ixeg/733/FMC/cdu1_H"},
        {49, "KEYI", "ixeg/733/FMC/cdu1_I"},
        {50, "KEYJ", "ixeg/733/FMC/cdu1_J"},
        {51, "KEYK", "ixeg/733/FMC/cdu1_K"},
        {52, "KEYL", "ixeg/733/FMC/cdu1_L"},
        {53, "KEYM", "ixeg/733/FMC/cdu1_M"},
        {54, "KEYN", "ixeg/733/FMC/cdu1_N"},
        {55, "KEYO", "ixeg/733/FMC/cdu1_O"},
        {56, "KEYP", "ixeg/733/FMC/cdu1_P"},
        {57, "KEYQ", "ixeg/733/FMC/cdu1_Q"},
        {58, "KEYR", "ixeg/733/FMC/cdu1_R"},
        {59, "KEYS", "ixeg/733/FMC/cdu1_S"},
        {60, "KEYT", "ixeg/733/FMC/cdu1_T"},
        {61, "KEYU", "ixeg/733/FMC/cdu1_U"},
        {62, "KEYV", "ixeg/733/FMC/cdu1_V"},
        {63, "KEYW", "ixeg/733/FMC/cdu1_W"},
        {64, "KEYX", "ixeg/733/FMC/cdu1_X"},
        {65, "KEYY", "ixeg/733/FMC/cdu1_Y"},
        {66, "KEYZ", "ixeg/733/FMC/cdu1_Z"},
        {67, "SPACE", ""},
        {68, "DEL", ""},
        {69, "SLASH", ""},
        {70, "CLR", "ixeg/733/FMC/cdu2_clr"}
    };
    
    return sevenThreeSevenButtonLayout;
}

const std::map<char, int>& IXEG733PfpProfile::colorMap() const {
    static const std::map<char, int> colMap = {
        {'L', 0x0042}, // L = Large/normal text (white)
        {'S', 0x0042}, // S = Small text (white)
        {'M', 0x00A5}, // M = Magenta
        {'G', 0x0084}, // G = Green
        {'C', 0x0063}, // C = Cyan (blue)
        {'I', 0x0042}, // I = Inverted (white for now)
        {'X', 0x0042}, // X = Special/labels (white)
        {' ', 0x0042}, // Space = white
        {'W', 0x0042}, // White fallback
    };

    return colMap;
}

void IXEG733PfpProfile::updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) {
    page = std::vector<std::vector<char>>(ProductPFP::PageLines, std::vector<char>(ProductPFP::PageCharsPerLine * ProductPFP::PageBytesPerChar, ' '));
    
    for (const auto& [ref, text] : cachedDatarefValues) {
        if (text.empty()) {
            continue;
        }

        // Handle title (line 0)
        if (ref == "ixeg/733/FMC/cdu1D_title") {
            for (int i = 0; i < text.size() && i < ProductPFP::PageCharsPerLine; ++i) {
                char c = text[i];
                if (c == 0x00) break;
                if (c != 0x20 || i == 0) {
                    product->writeLineToPage(page, 0, i, std::string(1, c), 'L', false);
                }
            }
            continue;
        }
        
        // Handle page number (top right of line 0)
        if (ref == "ixeg/733/FMC/cdu1D_pg_number") {
            // Position page number on the right side of line 0
            int startPos = ProductPFP::PageCharsPerLine - text.length();
            if (startPos > 0) {
                for (int i = 0; i < text.size() && (startPos + i) < ProductPFP::PageCharsPerLine; ++i) {
                    char c = text[i];
                    if (c == 0x00) break;
                    if (c != 0x20 || i == 0) {
                        product->writeLineToPage(page, 0, startPos + i, std::string(1, c), 'L', false);
                    }
                }
            }
            continue;
        }
        
        // Handle scratchpad (line 13)
        if (ref == "ixeg/733/FMC/cdu1D_scrpad") {
            for (int i = 0; i < text.size() && i < ProductPFP::PageCharsPerLine; ++i) {
                char c = text[i];
                if (c == 0x00) break;
                if (c != 0x20 || i == 0) {
                    product->writeLineToPage(page, 13, i, std::string(1, c), 'W', false);
                }
            }
            continue;
        }
        
        // Handle line datarefs
        if (ref.find("ixeg/733/FMC/cdu1D_line") == 0) {
            // Extract line number and side from dataref name
            // Format: ixeg/733/FMC/cdu1D_line{N}{L/R}_{d/t}
            size_t linePos = ref.find("line") + 4;
            if (linePos + 2 < ref.length()) {
                char lineChar = ref[linePos];
                char sideChar = ref[linePos + 1];
                char typeChar = ref[ref.length() - 1]; // 'd' for data, 't' for title
                
                if (lineChar >= '1' && lineChar <= '6' && 
                    (sideChar == 'L' || sideChar == 'R') &&
                    (typeChar == 'd' || typeChar == 't')) {
                    
                    int lineNum = lineChar - '0';
                    bool isLeftSide = (sideChar == 'L');
                    bool isTitle = (typeChar == 't');
                    
                    // Calculate display line
                    // Title lines (odd): 1, 3, 5, 7, 9, 11
                    // Data lines (even): 2, 4, 6, 8, 10, 12
                    int displayLine = (lineNum * 2) - (isTitle ? 1 : 0);
                    
                    // Calculate horizontal position
                    int startPos = 0;
                    if (!isLeftSide) {
                        // Right side - position text towards the right
                        startPos = ProductPFP::PageCharsPerLine - text.length();
                        if (startPos < ProductPFP::PageCharsPerLine / 2) {
                            startPos = ProductPFP::PageCharsPerLine / 2;
                        }
                    }
                    
                    // Determine color and font
                    char color = isTitle ? 'S' : 'L'; // Small font for titles, large for data
                    bool fontSmall = isTitle;
                    
                    // Write text to page
                    for (int i = 0; i < text.size() && (startPos + i) < ProductPFP::PageCharsPerLine; ++i) {
                        char c = text[i];
                        if (c == 0x00) break;
                        if (c != 0x20 || i == 0) {
                            product->writeLineToPage(page, displayLine, startPos + i, std::string(1, c), color, fontSmall);
                        }
                    }
                }
            }
            continue;
        }

        #if DEBUG
        printf("[PFP] Unhandled dataref %s: {", ref.c_str());
        for (unsigned char ch : text) {
            printf("0x%02X, ", static_cast<unsigned char>(ch));
        }
        printf("}\n");
        #endif
    }
}

void IXEG733PfpProfile::buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }

    Dataref::getInstance()->set<int>(button->dataref.c_str(), phase == xplm_CommandBegin ? 1 : 0);
}
