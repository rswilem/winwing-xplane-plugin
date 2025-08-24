#include "ff777-fmc-profile.h"
#include "product-fmc.h"
#include "font.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>

FlightFactor777FMCProfile::FlightFactor777FMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::Font737, product->identifierByte));
    
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
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "1-sim/ckpt/cduLLK1/anim"},
        {FMCKey::LSK2L, "1-sim/ckpt/cduLLK2/anim"},
        {FMCKey::LSK3L, "1-sim/ckpt/cduLLK3/anim"},
        {FMCKey::LSK4L, "1-sim/ckpt/cduLLK4/anim"},
        {FMCKey::LSK5L, "1-sim/ckpt/cduLLK5/anim"},
        {FMCKey::LSK6L, "1-sim/ckpt/cduLLK6/anim"},
        {FMCKey::LSK1R, "1-sim/ckpt/cduLRK1/anim"},
        {FMCKey::LSK2R, "1-sim/ckpt/cduLRK2/anim"},
        {FMCKey::LSK3R, "1-sim/ckpt/cduLRK3/anim"},
        {FMCKey::LSK4R, "1-sim/ckpt/cduLRK4/anim"},
        {FMCKey::LSK5R, "1-sim/ckpt/cduLRK5/anim"},
        {FMCKey::LSK6R, "1-sim/ckpt/cduLRK6/anim"},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, ""},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, ""},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, ""},
        {FMCKey::PFP7_ALTN, ""},
        {std::vector<FMCKey>{FMCKey::PFP7_VNAV, FMCKey::MCDU_DATA, FMCKey::PFP4_VNAV}, ""},
        {FMCKey::BRIGHTNESS_DOWN, ""},
        {FMCKey::BRIGHTNESS_UP, ""},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, ""},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, ""},
        {FMCKey::PFP_HOLD, ""},
        {std::vector<FMCKey>{FMCKey::PFP7_FMC_COMM, FMCKey::PFP4_FMC_COMM}, ""},
        {FMCKey::PROG, ""},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, ""},
        {FMCKey::MENU, ""},
        {std::vector<FMCKey>{FMCKey::PFP7_NAV_RAD, FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD}, ""},
        {FMCKey::PAGE_PREV, ""},
        {FMCKey::PAGE_NEXT, ""},
        {FMCKey::KEY1, ""},
        {FMCKey::KEY2, ""},
        {FMCKey::KEY3, ""},
        {FMCKey::KEY4, ""},
        {FMCKey::KEY5, ""},
        {FMCKey::KEY6, ""},
        {FMCKey::KEY7, ""},
        {FMCKey::KEY8, ""},
        {FMCKey::KEY9, ""},
        {FMCKey::PERIOD, ""},
        {FMCKey::KEY0, ""},
        {FMCKey::PLUSMINUS, ""},
        {FMCKey::KEYA, ""},
        {FMCKey::KEYB, ""},
        {FMCKey::KEYC, ""},
        {FMCKey::KEYD, ""},
        {FMCKey::KEYE, ""},
        {FMCKey::KEYF, ""},
        {FMCKey::KEYG, ""},
        {FMCKey::KEYH, ""},
        {FMCKey::KEYI, ""},
        {FMCKey::KEYJ, ""},
        {FMCKey::KEYK, ""},
        {FMCKey::KEYL, ""},
        {FMCKey::KEYM, ""},
        {FMCKey::KEYN, ""},
        {FMCKey::KEYO, ""},
        {FMCKey::KEYP, ""},
        {FMCKey::KEYQ, ""},
        {FMCKey::KEYR, ""},
        {FMCKey::KEYS, ""},
        {FMCKey::KEYT, ""},
        {FMCKey::KEYU, ""},
        {FMCKey::KEYV, ""},
        {FMCKey::KEYW, ""},
        {FMCKey::KEYX, ""},
        {FMCKey::KEYY, ""},
        {FMCKey::KEYZ, ""},
        {FMCKey::SPACE, ""},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, ""},
        {FMCKey::SLASH, ""},
        {FMCKey::CLR, ""}
    };
    
    return buttons;
}

const std::map<char, FMCTextColor>& FlightFactor777FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {0, FMCTextColor::COLOR_WHITE},
        {1, FMCTextColor::COLOR_WHITE},
        {2, FMCTextColor::COLOR_MAGENTA},
        {3, FMCTextColor::COLOR_GREEN},
        {4, FMCTextColor::COLOR_CYAN},
        {5, FMCTextColor::COLOR_GREY},
        {6, FMCTextColor::COLOR_WHITE_BG},
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
                color = 6;
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
