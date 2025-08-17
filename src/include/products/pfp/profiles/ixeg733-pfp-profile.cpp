#include "ixeg733-pfp-profile.h"
#include "product-pfp.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <XPLMUtilities.h>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>

IXEG733PfpProfile::IXEG733PfpProfile(ProductPFP *product) : PfpAircraftProfile(product) {
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
        
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("ixeg/733/rheostats/light_fmc_pt_act", [product](std::vector<float> brightness) {
        if (brightness.size() < 27) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[10] * 255.0f : 0;
        product->setLedBrightness(PFPLed::BACKLIGHT, target);
        product->setLedBrightness(PFPLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("ixeg/733/rheostats/light_fmc_pt_act");
    });
}

IXEG733PfpProfile::~IXEG733PfpProfile() {
    Dataref::getInstance()->unbind("ixeg/733/rheostats/light_fmc_pt_act");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool IXEG733PfpProfile::IsEligible() {
    return Dataref::getInstance()->exists("ixeg/733/FMC/cdu1_menu");
}

const std::vector<std::string>& IXEG733PfpProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "ixeg/733/FMC/cdu1D_pg_number",
        "ixeg/733/FMC/cdu1D_title",
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
        {0, "LSK1L", "ixeg/733/FMC/cdu1_lsk_1L"},
        {1, "LSK2L", "ixeg/733/FMC/cdu1_lsk_2L"},
        {2, "LSK3L", "ixeg/733/FMC/cdu1_lsk_3L"},
        {3, "LSK4L", "ixeg/733/FMC/cdu1_lsk_4L"},
        {4, "LSK5L", "ixeg/733/FMC/cdu1_lsk_5L"},
        {5, "LSK6L", "ixeg/733/FMC/cdu1_lsk_6L"},
        {6, "LSK1R", "ixeg/733/FMC/cdu1_lsk_1R"},
        {7, "LSK2R", "ixeg/733/FMC/cdu1_lsk_2R"},
        {8, "LSK3R", "ixeg/733/FMC/cdu1_lsk_3R"},
        {9, "LSK4R", "ixeg/733/FMC/cdu1_lsk_4R"},
        {10, "LSK5R", "ixeg/733/FMC/cdu1_lsk_5R"},
        {11, "LSK6R", "ixeg/733/FMC/cdu1_lsk_6R"},
        {12, "INITREF", "ixeg/733/FMC/cdu1_initref"},
        {13, "RTE", "ixeg/733/FMC/cdu1_rte"},
        {14, "CLB", "ixeg/733/FMC/cdu1_clb"},
        {15, "CRZ", "ixeg/733/FMC/cdu1_crz"},
        {16, "DES", "ixeg/733/FMC/cdu1_des"},
        {17, "BRT-", "ixeg/733/rheostats/light_fmc_pt_act", -0.1},
        {18, "BRT+", "ixeg/733/rheostats/light_fmc_pt_act", 0.1},
        {19, "MENU", "ixeg/733/FMC/cdu1_menu"},
        {20, "LEGS", "ixeg/733/FMC/cdu1_legs"},
        {21, "DEPARR", "ixeg/733/FMC/cdu1_deparr"},
        {22, "HOLD", "ixeg/733/FMC/cdu1_hold"},
        {23, "PROG", "ixeg/733/FMC/cdu1_prog"},
        {24, "EXEC", "ixeg/733/FMC/cdu1_exec"},
        {25, "N1LIMIT", "ixeg/733/FMC/cdu1_n1limit"},
        {26, "FIX", "ixeg/733/FMC/cdu1_fix"},
        {27, "PREV_PAGE", "ixeg/733/FMC/cdu1_prev"},
        {28, "NEXT_PAGE", "ixeg/733/FMC/cdu1_next"},
        {29, "KEY1", "ixeg/733/FMC/cdu1_1"},
        {30, "KEY2", "ixeg/733/FMC/cdu1_2"},
        {31, "KEY3", "ixeg/733/FMC/cdu1_3"},
        {32, "KEY4", "ixeg/733/FMC/cdu1_4"},
        {33, "KEY5", "ixeg/733/FMC/cdu1_5"},
        {34, "KEY6", "ixeg/733/FMC/cdu1_6"},
        {35, "KEY7", "ixeg/733/FMC/cdu1_7"},
        {36, "KEY8", "ixeg/733/FMC/cdu1_8"},
        {37, "KEY9", "ixeg/733/FMC/cdu1_9"},
        {38, "PERIOD", "ixeg/733/FMC/cdu1_dot"},
        {39, "KEY0", "ixeg/733/FMC/cdu1_0"},
        {40, "PLUSMINUS", "ixeg/733/FMC/cdu1_plus"},
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
        {67, "SPACE", "ixeg/733/FMC/cdu1_sp"},
        {68, "DEL", "ixeg/733/FMC/cdu1_del"},
        {69, "SLASH", "ixeg/733/FMC/cdu1_slash"},
        {70, "CLR", "ixeg/733/FMC/cdu2_clr"}
    };
    
    return sevenThreeSevenButtonLayout;
}

const std::map<char, int>& IXEG733PfpProfile::colorMap() const {
    static const std::map<char, int> colMap = {
        {'W', 0x0042}, // White text
        {'G', 0x0084}, // Green text
        {'S', 0x0084}, // Green text (Small)
        {'I', 0x0108}, // Gray text, should be inverted (black text on green). Not sure yet how.
    };
    return colMap;
}

std::pair<std::string, std::vector<char>> IXEG733PfpProfile::processIxegText(const std::vector<unsigned char> &characters) {
    std::string text;
    std::vector<char> colors;
    
    bool inInvertedMode = false;
    bool inSmallMode = false;
    
    for (size_t i = 0; i < characters.size(); ++i) {
        unsigned char c = characters[i];
        
        if (c == 0x00) {
            break;
        }
        
        if (c == 0xA3) {
            // 0xA3 = start marker for white text until next space
            inSmallMode = true;
            continue;
        }
        
        if (c >= 0x20 && c <= 0x7E) {
            char ch = static_cast<char>(c);
            
            // Check for $$ at current position
            if (!inInvertedMode && ch == '$' && i + 1 < characters.size() && 
                characters[i + 1] >= 0x20 && characters[i + 1] <= 0x7E && 
                static_cast<char>(characters[i + 1]) == '$') {
                inInvertedMode = true;
                i++; // Skip the second $
                continue; // Don't add $$ to the output text
            }
            
            // Check if we hit a space while in inverted or white mode
            if (ch == ' ') {
                if (inInvertedMode) {
                    inInvertedMode = false;
                }
                if (inSmallMode) {
                    inSmallMode = false;
                }
            }
            
            text += ch;
            colors.push_back(inInvertedMode ? 'I' : (inSmallMode ? 'S' : 'G'));
        }
    }
    
    return std::make_pair(text, colors);
}

void IXEG733PfpProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductPFP::PageLines, std::vector<char>(ProductPFP::PageCharsPerLine * ProductPFP::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    for (const auto& ref : displayDatarefs()) {
        std::vector<unsigned char> characters = datarefManager->getCached<std::vector<unsigned char>>(ref.c_str());
        if (characters.empty()) {
            continue;
        }
        
        debug_force("[PFP] Dataref %s: {", ref.c_str());
        for (unsigned char ch : characters) {
            debug_force("0x%02X, ", ch);
        }
        debug_force("}\n");
        
        auto [text, colors] = processIxegText(characters);
        if (ref == "ixeg/733/FMC/cdu1D_title") {
            for (int i = 0; i < text.size() && i < ProductPFP::PageCharsPerLine; ++i) {
                char c = text[i];
                char color = i < colors.size() ? colors[i] : 'G';
                product->writeLineToPage(page, 0, i, std::string(1, c), color, color == 'S');
            }
            continue;
        }
        
        if (ref == "ixeg/733/FMC/cdu1D_pg_number") {
            int startPos = ProductPFP::PageCharsPerLine - (int)text.length();
            if (startPos > 0) {
                for (int i = 0; i < text.size() && (startPos + i) < ProductPFP::PageCharsPerLine; ++i) {
                    char c = text[i];
                    char color = i < colors.size() ? colors[i] : 'G';
                    product->writeLineToPage(page, 0, startPos + i, std::string(1, c), color, color == 'S');
                }
            }
            continue;
        }
        
        if (ref == "ixeg/733/FMC/cdu1D_scrpad") {
            for (int i = 0; i < text.size() && i < ProductPFP::PageCharsPerLine; ++i) {
                char c = text[i];
                char color = i < colors.size() ? colors[i] : 'G';
                product->writeLineToPage(page, 13, i, std::string(1, c), color, color == 'S');
            }
            continue;
        }
        
        if (ref.find("ixeg/733/FMC/cdu1D_line") == 0) {
            size_t linePos = ref.find("line") + 4;
            if (linePos + 2 < ref.length()) {
                char lineChar = ref[linePos];
                char sideChar = ref[linePos + 1];
                char typeChar = ref[ref.length() - 1];
                
                if (lineChar >= '1' && lineChar <= '6' && 
                    (sideChar == 'L' || sideChar == 'R') &&
                    (typeChar == 'd' || typeChar == 't')) {
                    
                    int lineNum = lineChar - '0';
                    bool isLeftSide = (sideChar == 'L');
                    bool isTitle = (typeChar == 't');
                    
                    int displayLine = (lineNum * 2) - (isTitle ? 1 : 0);
                    
                    int startPos = 0;
                    if (!isLeftSide) {
                        startPos = ProductPFP::PageCharsPerLine - (int)text.length();
                        if (startPos < ProductPFP::PageCharsPerLine / 2) {
                            startPos = ProductPFP::PageCharsPerLine / 2;
                        }
                    }
                    
                    for (int i = 0; i < text.size() && (startPos + i) < ProductPFP::PageCharsPerLine; ++i) {
                        char c = text[i];
                        char color = i < colors.size() ? colors[i] : 'G';
                        product->writeLineToPage(page, displayLine, startPos + i, std::string(1, c), color, isTitle || color == 'S');
                    }
                }
            }
            continue;
        }
    }
}

void IXEG733PfpProfile::buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }
    
    if (std::fabs(button->value) > DBL_EPSILON) {
        if (phase != xplm_CommandBegin) {
            return;
        }
        
        float currentValue = Dataref::getInstance()->get<float>(button->dataref.c_str());
        Dataref::getInstance()->set<float>(button->dataref.c_str(), std::clamp(currentValue + button->value, 0.0, 1.0));
    }
    else {
        Dataref::getInstance()->set<int>(button->dataref.c_str(), phase == xplm_CommandBegin ? 1 : 0);
    }
}
