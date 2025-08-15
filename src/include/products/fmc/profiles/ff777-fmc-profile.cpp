#include "ff777-fmc-profile.h"
#include "product-fmc.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>

FlightFactor777FMCProfile::FlightFactor777FMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);
    
    Dataref::getInstance()->monitorExistingDataref<float>("sim/cockpit/electrical/instrument_brightness", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/electrical/instrument_brightness");
    });
}

FlightFactor777FMCProfile::~FlightFactor777FMCProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");    
}

bool FlightFactor777FMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("1-sim/cduL/display/symbols");
}

const std::vector<std::string>& FlightFactor777FMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "1-sim/cduL/display/symbols", // 336 letters
        "1-sim/cduL/display/symbolsColor", // 336 numbers
        "1-sim/cduL/display/symbolsEffects", // 336 numbers
        "1-sim/cduL/display/symbolsSize" // 336 numbers
    };
    
    return datarefs;
}

const std::vector<FMCButtonDef>& FlightFactor777FMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> tripleSevenButtonLayout = {
        {0, "LSK1L", "1-sim/ckpt/cduLLK1/anim"},
        {1, "LSK2L", "1-sim/ckpt/cduLLK2/anim"},
        {2, "LSK3L", "1-sim/ckpt/cduLLK3/anim"},
        {3, "LSK4L", "1-sim/ckpt/cduLLK4/anim"},
        {4, "LSK5L", "1-sim/ckpt/cduLLK5/anim"},
        {5, "LSK6L", "1-sim/ckpt/cduLLK6/anim"},
        {6, "LSK1R", "1-sim/ckpt/cduLRK1/anim"},
        {7, "LSK2R", "1-sim/ckpt/cduLRK2/anim"},
        {8, "LSK3R", "1-sim/ckpt/cduLRK3/anim"},
        {9, "LSK4R", "1-sim/ckpt/cduLRK4/anim"},
        {10, "LSK5R", "1-sim/ckpt/cduLRK5/anim"},
        {11, "LSK6R", "1-sim/ckpt/cduLRK6/anim"},
        {12, "INITREF", ""},
        {13, "RTE", ""},
        {14, "DEP/ARR", ""},
        {15, "ALTN", ""},
        {16, "VNAV", ""},
        {17, "BRT-", ""},
        {18, "BRT+", ""},
        {19, "FIX", ""},
        {20, "LEGS", ""},
        {21, "HOLD", ""},
        {22, "FMC/COMM", ""},
        {23, "PROG", ""},
        {24, "EXEC", ""},
        {25, "MENU", ""},
        {26, "NAV/RAD", ""},
        {27, "PREV_PAGE", ""},
        {28, "NEXT_PAGE", ""},
        {29, "KEY1", ""},
        {30, "KEY2", ""},
        {31, "KEY3", ""},
        {32, "KEY4", ""},
        {33, "KEY5", ""},
        {34, "KEY6", ""},
        {35, "KEY7", ""},
        {36, "KEY8", ""},
        {37, "KEY9", ""},
        {38, "PERIOD", ""},
        {39, "KEY0", ""},
        {40, "PLUSMINUS", ""},
        {41, "KEYA", ""},
        {42, "KEYB", ""},
        {43, "KEYC", ""},
        {44, "KEYD", ""},
        {45, "KEYE", ""},
        {46, "KEYF", ""},
        {47, "KEYG", ""},
        {48, "KEYH", ""},
        {49, "KEYI", ""},
        {50, "KEYJ", ""},
        {51, "KEYK", ""},
        {52, "KEYL", ""},
        {53, "KEYM", ""},
        {54, "KEYN", ""},
        {55, "KEYO", ""},
        {56, "KEYP", ""},
        {57, "KEYQ", ""},
        {58, "KEYR", ""},
        {59, "KEYS", ""},
        {60, "KEYT", ""},
        {61, "KEYU", ""},
        {62, "KEYV", ""},
        {63, "KEYW", ""},
        {64, "KEYX", ""},
        {65, "KEYY", ""},
        {66, "KEYZ", ""},
        {67, "SPACE", ""},
        {68, "DEL", ""},
        {69, "SLASH", ""},
        {70, "CLR", ""}
    };
    
    return tripleSevenButtonLayout;
}

const std::map<char, FMCTextColor>& FlightFactor777FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {0, FMCTextColor::COLOR_WHITE},
        {1, FMCTextColor::COLOR_WHITE},
        {2, FMCTextColor::COLOR_MAGENTA},
        {3, FMCTextColor::COLOR_RED}, // Unknown color, change to the proper color
        {4, FMCTextColor::COLOR_CYAN},
        {5, FMCTextColor::COLOR_GREY},
    };
    
    return colMap;
}

void FlightFactor777FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;
            
        case '*':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;
        
        default:
            buffer->push_back(character);
            break;
    }
}

void FlightFactor777FMCProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    std::vector<unsigned char> symbols = datarefManager->getCached<std::vector<unsigned char>>("1-sim/cduL/display/symbols");
    std::vector<int> colors = datarefManager->getCached<std::vector<int>>("1-sim/cduL/display/symbolsColor");
    std::vector<int> sizes = datarefManager->getCached<std::vector<int>>("1-sim/cduL/display/symbolsSize");
    std::vector<int> effects = datarefManager->getCached<std::vector<int>>("1-sim/cduL/display/symbolsEffects");
    
    if (symbols.size() < FlightFactor777FMCProfile::DataLength || colors.size() < FlightFactor777FMCProfile::DataLength || sizes.size() < FlightFactor777FMCProfile::DataLength || effects.size() < FlightFactor777FMCProfile::DataLength) {
        return;
    }
    
    for (int line = 0; line < ProductFMC::PageLines && line * ProductFMC::PageCharsPerLine < FlightFactor777FMCProfile::DataLength; ++line) {
        for (int pos = 0; pos < ProductFMC::PageCharsPerLine; ++pos) {
            int index = line * ProductFMC::PageCharsPerLine + pos;
            
            if (index >= FlightFactor777FMCProfile::DataLength) {
                break;
            }
            
            char symbol = symbols[index];
            if (symbol == 0x00 || symbol == 0x20) {
                continue;
            }
            
            unsigned char color = static_cast<unsigned char>(colors[index]);
            unsigned char fontSize = static_cast<unsigned char>(sizes[index]);
            unsigned char effect = static_cast<unsigned char>(effects[index]);
            bool fontSmall = fontSize == 2;
            
            if (effect == 1) {
                // Inverted text
                color = 2;
            }
            
            product->writeLineToPage(page, line, pos, std::string(1, symbol), color, fontSmall);
        }
    }
}

void FlightFactor777FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }

    Dataref::getInstance()->set<float>(button->dataref.c_str(), phase == xplm_CommandBegin ? 1 : 0);
}
