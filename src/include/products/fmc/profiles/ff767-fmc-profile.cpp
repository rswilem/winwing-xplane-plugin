#include "ff767-fmc-profile.h"
#include "product-fmc.h"
#include "font.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <regex>
#include <XPLMUtilities.h>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>

FlightFactor767FMCProfile::FlightFactor767FMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
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

FlightFactor767FMCProfile::~FlightFactor767FMCProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool FlightFactor767FMCProfile::IsEligible() {
    static const std::string author = Dataref::getInstance()->get<std::string>("sim/aircraft/view/acf_author");
    static const std::string icao = Dataref::getInstance()->get<std::string>("sim/aircraft/view/acf_ICAO");
    
    if (!author.starts_with("FlightFactor")) {
        return false;
    }

    static const std::regex icaoPattern("^(B75[23]|B76[234]|76[XY])$");
    
    return std::regex_match(icao, icaoPattern);
}

const std::vector<std::string>& FlightFactor767FMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "1-sim/cduL/display/symbols", // 336 letters
        "1-sim/cduL/display/symbolsColor", // 336 numbers
        "1-sim/cduL/display/symbolsEffects", // 336 numbers
        "1-sim/cduL/display/symbolsSize" // 336 numbers
    };
    
    return datarefs;
}

const std::vector<FMCButtonDef>& FlightFactor767FMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "757Avionics/CDU/LLSK1"},
        {FMCKey::LSK2L, "757Avionics/CDU/LLSK2"},
        {FMCKey::LSK3L, "757Avionics/CDU/LLSK3"},
        {FMCKey::LSK4L, "757Avionics/CDU/LLSK4"},
        {FMCKey::LSK5L, "757Avionics/CDU/LLSK5"},
        {FMCKey::LSK6L, "757Avionics/CDU/LLSK6"},
        {FMCKey::LSK1R, "757Avionics/CDU/RLSK1"},
        {FMCKey::LSK2R, "757Avionics/CDU/RLSK2"},
        {FMCKey::LSK3R, "757Avionics/CDU/RLSK3"},
        {FMCKey::LSK4R, "757Avionics/CDU/RLSK4"},
        {FMCKey::LSK5R, "757Avionics/CDU/RLSK5"},
        {FMCKey::LSK6R, "757Avionics/CDU/RLSK6"},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, "757Avionics/CDU/init_ref"},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, "757Avionics/CDU/rte"},
        {FMCKey::PFP3_CLB, "757Avionics/CDU/clb"},
        {FMCKey::PFP3_CRZ, "757Avionics/CDU/crz"},
        {FMCKey::PFP3_DES, "757Avionics/CDU/des"},
        {FMCKey::BRIGHTNESS_DOWN, "ixeg/733/rheostats/light_fmc_pt_act", -0.1},
        {FMCKey::BRIGHTNESS_UP, "ixeg/733/rheostats/light_fmc_pt_act", 0.1},
        {FMCKey::MENU, "757Avionics/CDU/mcdu_menu"},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, "757Avionics/CDU/legs"},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, "757Avionics/CDU/dep_arr"},
        {FMCKey::PFP_HOLD, "757Avionics/CDU/hold"},
        {FMCKey::PROG, "757Avionics/CDU/prog"},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, "757Avionics/CDU/exec"},
        {std::vector<FMCKey>{FMCKey::PFP3_N1_LIMIT, FMCKey::MCDU_PERF}, "757Avionics/CDU/dir"},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, "757Avionics/CDU/fix"},
        {FMCKey::PAGE_PREV, "757Avionics/CDU/prev_page"},
        {FMCKey::PAGE_NEXT, "757Avionics/CDU/next_page"},
        {FMCKey::KEY1, "757Avionics/CDU/1"},
        {FMCKey::KEY2, "757Avionics/CDU/2"},
        {FMCKey::KEY3, "757Avionics/CDU/3"},
        {FMCKey::KEY4, "757Avionics/CDU/4"},
        {FMCKey::KEY5, "757Avionics/CDU/5"},
        {FMCKey::KEY6, "757Avionics/CDU/6"},
        {FMCKey::KEY7, "757Avionics/CDU/7"},
        {FMCKey::KEY8, "757Avionics/CDU/8"},
        {FMCKey::KEY9, "757Avionics/CDU/9"},
        {FMCKey::PERIOD, "757Avionics/CDU/point"},
        {FMCKey::KEY0, "757Avionics/CDU/0"},
        {FMCKey::PLUSMINUS, "757Avionics/CDU/plusminus"},
        {FMCKey::KEYA, "757Avionics/CDU/A"},
        {FMCKey::KEYB, "757Avionics/CDU/B"},
        {FMCKey::KEYC, "757Avionics/CDU/C"},
        {FMCKey::KEYD, "757Avionics/CDU/D"},
        {FMCKey::KEYE, "757Avionics/CDU/E"},
        {FMCKey::KEYF, "757Avionics/CDU/F"},
        {FMCKey::KEYG, "757Avionics/CDU/G"},
        {FMCKey::KEYH, "757Avionics/CDU/H"},
        {FMCKey::KEYI, "757Avionics/CDU/I"},
        {FMCKey::KEYJ, "757Avionics/CDU/J"},
        {FMCKey::KEYK, "757Avionics/CDU/K"},
        {FMCKey::KEYL, "757Avionics/CDU/L"},
        {FMCKey::KEYM, "757Avionics/CDU/M"},
        {FMCKey::KEYN, "757Avionics/CDU/N"},
        {FMCKey::KEYO, "757Avionics/CDU/O"},
        {FMCKey::KEYP, "757Avionics/CDU/P"},
        {FMCKey::KEYQ, "757Avionics/CDU/Q"},
        {FMCKey::KEYR, "757Avionics/CDU/R"},
        {FMCKey::KEYS, "757Avionics/CDU/S"},
        {FMCKey::KEYT, "757Avionics/CDU/T"},
        {FMCKey::KEYU, "757Avionics/CDU/U"},
        {FMCKey::KEYV, "757Avionics/CDU/V"},
        {FMCKey::KEYW, "757Avionics/CDU/W"},
        {FMCKey::KEYX, "757Avionics/CDU/X"},
        {FMCKey::KEYY, "757Avionics/CDU/Y"},
        {FMCKey::KEYZ, "757Avionics/CDU/Z"},
        {FMCKey::SPACE, "757Avionics/CDU/space"},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, "757Avionics/CDU/delete"},
        {FMCKey::SLASH, "757Avionics/CDU/slash"},
        {FMCKey::CLR, "757Avionics/CDU/clear"}
    };
    
    return buttons;
}

const std::map<char, FMCTextColor>& FlightFactor767FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {0, FMCTextColor::COLOR_WHITE},
        {1, FMCTextColor::COLOR_WHITE},
        {2, FMCTextColor::COLOR_MAGENTA},
        {3, FMCTextColor::COLOR_GREEN},
        {4, FMCTextColor::COLOR_CYAN},
        {5, FMCTextColor::COLOR_GREY},
        {6, FMCTextColor::COLOR_WHITE_BG}
    };
    return colMap;
}

void FlightFactor767FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
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

void FlightFactor767FMCProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    std::vector<unsigned char> symbols = datarefManager->getCached<std::vector<unsigned char>>("1-sim/cduL/display/symbols");
    std::vector<int> colors = datarefManager->getCached<std::vector<int>>("1-sim/cduL/display/symbolsColor");
    std::vector<int> sizes = datarefManager->getCached<std::vector<int>>("1-sim/cduL/display/symbolsSize");
    std::vector<int> effects = datarefManager->getCached<std::vector<int>>("1-sim/cduL/display/symbolsEffects");
    
    if (symbols.size() < FlightFactor767FMCProfile::DataLength || colors.size() < FlightFactor767FMCProfile::DataLength || sizes.size() < FlightFactor767FMCProfile::DataLength || effects.size() < FlightFactor767FMCProfile::DataLength) {
        return;
    }
    
    for (int line = 0; line < ProductFMC::PageLines && line * ProductFMC::PageCharsPerLine < FlightFactor767FMCProfile::DataLength; ++line) {
        for (int pos = 0; pos < ProductFMC::PageCharsPerLine; ++pos) {
            int index = line * ProductFMC::PageCharsPerLine + pos;
            
            if (index >= FlightFactor767FMCProfile::DataLength) {
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

void FlightFactor767FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }

    Dataref::getInstance()->set<float>(button->dataref.c_str(), phase == xplm_CommandBegin ? 1 : 0);
}
