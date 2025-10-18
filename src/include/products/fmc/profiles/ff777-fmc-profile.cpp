#include "ff777-fmc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>
#include <cstring>

FlightFactor777FMCProfile::FlightFactor777FMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::Font737, product->identifierByte));

    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/cduL/brt", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("1-sim/cduL/ok") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<float>("1-sim/ckpt/lights/aisle", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("1-sim/cduL/ok") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/cduL/ok", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/cduL/brt");
        Dataref::getInstance()->executeChangedCallbacksForDataref("1-sim/ckpt/lights/aisle");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lamps/cduCptAct", [product](bool enabled) {
        product->setLedBrightness(FMCLed::PFP_EXEC, enabled ? 1 : 0);
        product->setLedBrightness(FMCLed::MCDU_MCDU, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lamps/cduCptMSG", [product](bool enabled) {
        product->setLedBrightness(FMCLed::PFP_MSG, enabled ? 1 : 0);
        product->setLedBrightness(FMCLed::MCDU_RDY, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("1-sim/ckpt/lamps/cduCptOFST", [product](bool enabled) {
        product->setLedBrightness(FMCLed::PFP_OFST, enabled ? 1 : 0);
    });
}

FlightFactor777FMCProfile::~FlightFactor777FMCProfile() {
    Dataref::getInstance()->unbind("1-sim/cduL/brt");
    Dataref::getInstance()->unbind("1-sim/ckpt/lights/aisle");
    Dataref::getInstance()->unbind("1-sim/cduL/ok");
    Dataref::getInstance()->unbind("1-sim/ckpt/lamps/cduCptAct");
    Dataref::getInstance()->unbind("1-sim/ckpt/lamps/cduCptMSG");
    Dataref::getInstance()->unbind("1-sim/ckpt/lamps/cduCptOFST");
}

bool FlightFactor777FMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("1-sim/cduL/display/symbols");
}

const std::vector<std::string> &FlightFactor777FMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "1-sim/cduL/display/symbols",        // 336 letters
        "1-sim/cduL/display/symbolsColor",   // 336 numbers
        "1-sim/cduL/display/symbolsEffects", // 336 numbers
        "1-sim/cduL/display/symbolsSize"     // 336 numbers
    };

    return datarefs;
}

const std::vector<FMCButtonDef> &FlightFactor777FMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "1-sim/command/cduLLK1_button"},
        {FMCKey::LSK2L, "1-sim/command/cduLLK2_button"},
        {FMCKey::LSK3L, "1-sim/command/cduLLK3_button"},
        {FMCKey::LSK4L, "1-sim/command/cduLLK4_button"},
        {FMCKey::LSK5L, "1-sim/command/cduLLK5_button"},
        {FMCKey::LSK6L, "1-sim/command/cduLLK6_button"},
        {FMCKey::LSK1R, "1-sim/command/cduLRK1_button"},
        {FMCKey::LSK2R, "1-sim/command/cduLRK2_button"},
        {FMCKey::LSK3R, "1-sim/command/cduLRK3_button"},
        {FMCKey::LSK4R, "1-sim/command/cduLRK4_button"},
        {FMCKey::LSK5R, "1-sim/command/cduLRK5_button"},
        {FMCKey::LSK6R, "1-sim/command/cduLRK6_button"},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, "1-sim/command/cduLinitButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, "1-sim/command/cduLrteButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, "1-sim/command/cduLdepButton_button"},
        {FMCKey::PFP7_ALTN, "1-sim/command/cduLaltnButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP7_VNAV, FMCKey::MCDU_DATA, FMCKey::PFP4_VNAV, FMCKey::PFP3_CRZ}, "1-sim/command/cduLvnavButton_button"},
        {FMCKey::BRIGHTNESS_DOWN, "1-sim/command/cduLBrtRotary_rotary-"},
        {FMCKey::BRIGHTNESS_UP, "1-sim/command/cduLBrtRotary_rotary+"},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, "1-sim/command/cduLfixButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, "1-sim/command/cduLlegsButton_button"},
        {FMCKey::PFP_HOLD, "1-sim/command/cduLholdButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP7_FMC_COMM, FMCKey::PFP4_FMC_COMM}, "1-sim/command/cduLfmcCommButton_button"},
        {FMCKey::PROG, "1-sim/command/cduLprogButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, "1-sim/command/cduLexecButton_button"},
        {FMCKey::MENU, "1-sim/command/cduLmenuButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP7_NAV_RAD, FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD}, "1-sim/command/cduLnavButton_button"},
        {FMCKey::PAGE_PREV, "1-sim/command/cduLprevButton_button"},
        {FMCKey::PAGE_NEXT, "1-sim/command/cduLnextButton_button"},
        {FMCKey::KEY1, "1-sim/command/cduL1Button_button"},
        {FMCKey::KEY2, "1-sim/command/cduL2Button_button"},
        {FMCKey::KEY3, "1-sim/command/cduL3Button_button"},
        {FMCKey::KEY4, "1-sim/command/cduL4Button_button"},
        {FMCKey::KEY5, "1-sim/command/cduL5Button_button"},
        {FMCKey::KEY6, "1-sim/command/cduL6Button_button"},
        {FMCKey::KEY7, "1-sim/command/cduL7Button_button"},
        {FMCKey::KEY8, "1-sim/command/cduL8Button_button"},
        {FMCKey::KEY9, "1-sim/command/cduL9Button_button"},
        {FMCKey::PERIOD, "1-sim/command/cduLdotButton_button"},
        {FMCKey::KEY0, "1-sim/command/cduL0Button_button"},
        {FMCKey::PLUSMINUS, "1-sim/command/cduLpmButton_button"},
        {FMCKey::KEYA, "1-sim/command/cduLAButton_button"},
        {FMCKey::KEYB, "1-sim/command/cduLBButton_button"},
        {FMCKey::KEYC, "1-sim/command/cduLCButton_button"},
        {FMCKey::KEYD, "1-sim/command/cduLDButton_button"},
        {FMCKey::KEYE, "1-sim/command/cduLEButton_button"},
        {FMCKey::KEYF, "1-sim/command/cduLFButton_button"},
        {FMCKey::KEYG, "1-sim/command/cduLGButton_button"},
        {FMCKey::KEYH, "1-sim/command/cduLHButton_button"},
        {FMCKey::KEYI, "1-sim/command/cduLIButton_button"},
        {FMCKey::KEYJ, "1-sim/command/cduLJButton_button"},
        {FMCKey::KEYK, "1-sim/command/cduLKButton_button"},
        {FMCKey::KEYL, "1-sim/command/cduLLButton_button"},
        {FMCKey::KEYM, "1-sim/command/cduLMButton_button"},
        {FMCKey::KEYN, "1-sim/command/cduLNButton_button"},
        {FMCKey::KEYO, "1-sim/command/cduLOButton_button"},
        {FMCKey::KEYP, "1-sim/command/cduLPButton_button"},
        {FMCKey::KEYQ, "1-sim/command/cduLQButton_button"},
        {FMCKey::KEYR, "1-sim/command/cduLRButton_button"},
        {FMCKey::KEYS, "1-sim/command/cduLSButton_button"},
        {FMCKey::KEYT, "1-sim/command/cduLTButton_button"},
        {FMCKey::KEYU, "1-sim/command/cduLUButton_button"},
        {FMCKey::KEYV, "1-sim/command/cduLVButton_button"},
        {FMCKey::KEYW, "1-sim/command/cduLWButton_button"},
        {FMCKey::KEYX, "1-sim/command/cduLXButton_button"},
        {FMCKey::KEYY, "1-sim/command/cduLYButton_button"},
        {FMCKey::KEYZ, "1-sim/command/cduLZButton_button"},
        {FMCKey::SPACE, "1-sim/command/cduLspButton_button"},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, "1-sim/command/cduLdelButton_button"},
        {FMCKey::SLASH, "1-sim/command/cduLslashButton_button"},
        {FMCKey::CLR, "1-sim/command/cduLclrButton_button"}};

    return buttons;
}

const std::map<char, FMCTextColor> &FlightFactor777FMCProfile::colorMap() const {
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

void FlightFactor777FMCProfile::updatePage(std::vector<std::vector<char>> &page) {
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

    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
}
