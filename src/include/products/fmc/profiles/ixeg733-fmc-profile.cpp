#include "ixeg733-fmc-profile.h"
#include "product-fmc.h"
#include "font.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <XPLMUtilities.h>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>

IXEG733FMCProfile::IXEG733FMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::FontVGA1, product->identifierByte));
        
    Dataref::getInstance()->monitorExistingDataref<float>("ixeg/733/rheostats/light_fmc_pt_act", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("ixeg/733/rheostats/light_fmc_pt_act");
    });
}

IXEG733FMCProfile::~IXEG733FMCProfile() {
    Dataref::getInstance()->unbind("ixeg/733/rheostats/light_fmc_pt_act");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool IXEG733FMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("ixeg/733/FMC/cdu1_menu");
}

const std::vector<std::string>& IXEG733FMCProfile::displayDatarefs() const {
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

const std::vector<FMCButtonDef>& IXEG733FMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "ixeg/733/FMC/cdu1_lsk_1L"},
        {FMCKey::LSK2L, "ixeg/733/FMC/cdu1_lsk_2L"},
        {FMCKey::LSK3L, "ixeg/733/FMC/cdu1_lsk_3L"},
        {FMCKey::LSK4L, "ixeg/733/FMC/cdu1_lsk_4L"},
        {FMCKey::LSK5L, "ixeg/733/FMC/cdu1_lsk_5L"},
        {FMCKey::LSK6L, "ixeg/733/FMC/cdu1_lsk_6L"},
        {FMCKey::LSK1R, "ixeg/733/FMC/cdu1_lsk_1R"},
        {FMCKey::LSK2R, "ixeg/733/FMC/cdu1_lsk_2R"},
        {FMCKey::LSK3R, "ixeg/733/FMC/cdu1_lsk_3R"},
        {FMCKey::LSK4R, "ixeg/733/FMC/cdu1_lsk_4R"},
        {FMCKey::LSK5R, "ixeg/733/FMC/cdu1_lsk_5R"},
        {FMCKey::LSK6R, "ixeg/733/FMC/cdu1_lsk_6R"},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, "ixeg/733/FMC/cdu1_initref"},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, "ixeg/733/FMC/cdu1_rte"},
        {FMCKey::PFP3_CLB, "ixeg/733/FMC/cdu1_clb"},
        {FMCKey::PFP3_CRZ, "ixeg/733/FMC/cdu1_crz"},
        {FMCKey::PFP3_DES, "ixeg/733/FMC/cdu1_des"},
        {FMCKey::BRIGHTNESS_DOWN, "ixeg/733/rheostats/light_fmc_pt_act", -0.1},
        {FMCKey::BRIGHTNESS_UP, "ixeg/733/rheostats/light_fmc_pt_act", 0.1},
        {FMCKey::MENU, "ixeg/733/FMC/cdu1_menu"},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, "ixeg/733/FMC/cdu1_legs"},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, "ixeg/733/FMC/cdu1_deparr"},
        {FMCKey::PFP_HOLD, "ixeg/733/FMC/cdu1_hold"},
        {FMCKey::PROG, "ixeg/733/FMC/cdu1_prog"},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, "ixeg/733/FMC/cdu1_exec"},
        {std::vector<FMCKey>{FMCKey::PFP3_N1_LIMIT, FMCKey::MCDU_PERF}, "ixeg/733/FMC/cdu1_n1limit"},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, "ixeg/733/FMC/cdu1_fix"},
        {FMCKey::PAGE_PREV, "ixeg/733/FMC/cdu1_prev"},
        {FMCKey::PAGE_NEXT, "ixeg/733/FMC/cdu1_next"},
        {FMCKey::KEY1, "ixeg/733/FMC/cdu1_1"},
        {FMCKey::KEY2, "ixeg/733/FMC/cdu1_2"},
        {FMCKey::KEY3, "ixeg/733/FMC/cdu1_3"},
        {FMCKey::KEY4, "ixeg/733/FMC/cdu1_4"},
        {FMCKey::KEY5, "ixeg/733/FMC/cdu1_5"},
        {FMCKey::KEY6, "ixeg/733/FMC/cdu1_6"},
        {FMCKey::KEY7, "ixeg/733/FMC/cdu1_7"},
        {FMCKey::KEY8, "ixeg/733/FMC/cdu1_8"},
        {FMCKey::KEY9, "ixeg/733/FMC/cdu1_9"},
        {FMCKey::PERIOD, "ixeg/733/FMC/cdu1_dot"},
        {FMCKey::KEY0, "ixeg/733/FMC/cdu1_0"},
        {FMCKey::PLUSMINUS, "ixeg/733/FMC/cdu1_plus"},
        {FMCKey::KEYA, "ixeg/733/FMC/cdu1_A"},
        {FMCKey::KEYB, "ixeg/733/FMC/cdu1_B"},
        {FMCKey::KEYC, "ixeg/733/FMC/cdu1_C"},
        {FMCKey::KEYD, "ixeg/733/FMC/cdu1_D"},
        {FMCKey::KEYE, "ixeg/733/FMC/cdu1_E"},
        {FMCKey::KEYF, "ixeg/733/FMC/cdu1_F"},
        {FMCKey::KEYG, "ixeg/733/FMC/cdu1_G"},
        {FMCKey::KEYH, "ixeg/733/FMC/cdu1_H"},
        {FMCKey::KEYI, "ixeg/733/FMC/cdu1_I"},
        {FMCKey::KEYJ, "ixeg/733/FMC/cdu1_J"},
        {FMCKey::KEYK, "ixeg/733/FMC/cdu1_K"},
        {FMCKey::KEYL, "ixeg/733/FMC/cdu1_L"},
        {FMCKey::KEYM, "ixeg/733/FMC/cdu1_M"},
        {FMCKey::KEYN, "ixeg/733/FMC/cdu1_N"},
        {FMCKey::KEYO, "ixeg/733/FMC/cdu1_O"},
        {FMCKey::KEYP, "ixeg/733/FMC/cdu1_P"},
        {FMCKey::KEYQ, "ixeg/733/FMC/cdu1_Q"},
        {FMCKey::KEYR, "ixeg/733/FMC/cdu1_R"},
        {FMCKey::KEYS, "ixeg/733/FMC/cdu1_S"},
        {FMCKey::KEYT, "ixeg/733/FMC/cdu1_T"},
        {FMCKey::KEYU, "ixeg/733/FMC/cdu1_U"},
        {FMCKey::KEYV, "ixeg/733/FMC/cdu1_V"},
        {FMCKey::KEYW, "ixeg/733/FMC/cdu1_W"},
        {FMCKey::KEYX, "ixeg/733/FMC/cdu1_X"},
        {FMCKey::KEYY, "ixeg/733/FMC/cdu1_Y"},
        {FMCKey::KEYZ, "ixeg/733/FMC/cdu1_Z"},
        {FMCKey::SPACE, "ixeg/733/FMC/cdu1_sp"},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, "ixeg/733/FMC/cdu1_del"},
        {FMCKey::SLASH, "ixeg/733/FMC/cdu1_slash"},
        {FMCKey::CLR, "ixeg/733/FMC/cdu1_clr"}
    };
    
    return buttons;
}

const std::map<char, FMCTextColor>& IXEG733FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {'W', FMCTextColor::COLOR_WHITE},
        {'G', FMCTextColor::COLOR_GREEN}, // Green text
        {'S', FMCTextColor::COLOR_GREEN}, // Green text (Small)
        {'I', FMCTextColor::COLOR_GREEN_BG},
    };
    return colMap;
}

void IXEG733FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;
            
        case '`':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;
        
        default:
            buffer->push_back(character);
            break;
    }
}

std::pair<std::string, std::vector<char>> IXEG733FMCProfile::processIxegText(const std::vector<unsigned char> &characters) {
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

void IXEG733FMCProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    for (const auto& ref : displayDatarefs()) {
        std::vector<unsigned char> characters = datarefManager->getCached<std::vector<unsigned char>>(ref.c_str());
        if (characters.empty()) {
            continue;
        }
        
        auto [text, colors] = processIxegText(characters);
        if (ref == "ixeg/733/FMC/cdu1D_title") {
            for (int i = 0; i < text.size() && i < ProductFMC::PageCharsPerLine; ++i) {
                char c = text[i];
                char color = i < colors.size() ? colors[i] : 'G';
                product->writeLineToPage(page, 0, i, std::string(1, c), color, color == 'S');
            }
            continue;
        }
        
        if (ref == "ixeg/733/FMC/cdu1D_pg_number") {
            int startPos = ProductFMC::PageCharsPerLine - (int)text.length();
            if (startPos > 0) {
                for (int i = 0; i < text.size() && (startPos + i) < ProductFMC::PageCharsPerLine; ++i) {
                    char c = text[i];
                    char color = i < colors.size() ? colors[i] : 'G';
                    product->writeLineToPage(page, 0, startPos + i, std::string(1, c), color, color == 'S');
                }
            }
            continue;
        }
        
        if (ref == "ixeg/733/FMC/cdu1D_scrpad") {
            for (int i = 0; i < text.size() && i < ProductFMC::PageCharsPerLine; ++i) {
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
                        startPos = ProductFMC::PageCharsPerLine - (int)text.length();
                        if (startPos < ProductFMC::PageCharsPerLine / 2) {
                            startPos = ProductFMC::PageCharsPerLine / 2;
                        }
                    }
                    
                    for (int i = 0; i < text.size() && (startPos + i) < ProductFMC::PageCharsPerLine; ++i) {
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

void IXEG733FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
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
