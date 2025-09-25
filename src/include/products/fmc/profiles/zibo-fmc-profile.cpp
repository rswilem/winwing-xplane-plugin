#include "zibo-fmc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>

ZiboFMCProfile::ZiboFMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    datarefRegex = std::regex("laminar/B738/fmc1/Line([0-9]{2})_([A-Z]+)");

    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::Font737, product->identifierByte));

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/instrument_brightness", [product](std::vector<float> screenBrightness) {
        if (screenBrightness.size() < 11) {
            return;
        }

        // brightness[11] is fmc2 screen
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? screenBrightness[10] * 255.0f : 0;
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/panel_brightness", [product](std::vector<float> panelBrightness) {
        if (panelBrightness.size() < 4) {
            return;
        }

        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? panelBrightness[3] * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/panel_brightness");
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/instrument_brightness");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/B738/fmc/fmc_message", [product](bool enabled) {
        product->setLedBrightness(FMCLed::PFP_MSG, enabled ? 1 : 0);
        product->setLedBrightness(FMCLed::MCDU_MCDU, enabled ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("laminar/B738/indicators/fmc_exec_lights", [product](bool enabled) {
        product->setLedBrightness(FMCLed::PFP_EXEC, enabled ? 1 : 0);
        product->setLedBrightness(FMCLed::MCDU_RDY, enabled ? 1 : 0);
    });
}

ZiboFMCProfile::~ZiboFMCProfile() {
    Dataref::getInstance()->unbind("laminar/B738/electric/instrument_brightness");
    Dataref::getInstance()->unbind("laminar/B738/electric/panel_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("laminar/B738/fmc/fmc_message");
    Dataref::getInstance()->unbind("laminar/B738/indicators/fmc_exec_lights");
}

bool ZiboFMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("laminar/B738/electric/instrument_brightness");
}

const std::vector<std::string> &ZiboFMCProfile::displayDatarefs() const {
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
        "laminar/B738/fmc1/Line_entry_I"};

    return datarefs;
}

const std::vector<FMCButtonDef> &ZiboFMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "laminar/B738/button/fmc1_1L"},
        {FMCKey::LSK2L, "laminar/B738/button/fmc1_2L"},
        {FMCKey::LSK3L, "laminar/B738/button/fmc1_3L"},
        {FMCKey::LSK4L, "laminar/B738/button/fmc1_4L"},
        {FMCKey::LSK5L, "laminar/B738/button/fmc1_5L"},
        {FMCKey::LSK6L, "laminar/B738/button/fmc1_6L"},
        {FMCKey::LSK1R, "laminar/B738/button/fmc1_1R"},
        {FMCKey::LSK2R, "laminar/B738/button/fmc1_2R"},
        {FMCKey::LSK3R, "laminar/B738/button/fmc1_3R"},
        {FMCKey::LSK4R, "laminar/B738/button/fmc1_4R"},
        {FMCKey::LSK5R, "laminar/B738/button/fmc1_5R"},
        {FMCKey::LSK6R, "laminar/B738/button/fmc1_6R"},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, "laminar/B738/button/fmc1_init_ref"},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, "laminar/B738/button/fmc1_rte"},
        {FMCKey::PFP3_CLB, "laminar/B738/button/fmc1_clb"},
        {FMCKey::PFP3_CRZ, "laminar/B738/button/fmc1_crz"},
        {FMCKey::PFP3_DES, "laminar/B738/button/fmc1_des"},
        {FMCKey::BRIGHTNESS_DOWN, "laminar/B738/electric/instrument_brightness[10]", -0.1},
        {FMCKey::BRIGHTNESS_UP, "laminar/B738/electric/instrument_brightness[10]", 0.1},
        {FMCKey::MENU, "laminar/B738/button/fmc1_menu"},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, "laminar/B738/button/fmc1_legs"},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, "laminar/B738/button/fmc1_dep_app"},
        {FMCKey::PFP_HOLD, "laminar/B738/button/fmc1_hold"},
        {FMCKey::PROG, "laminar/B738/button/fmc1_prog"},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, "laminar/B738/button/fmc1_exec"},
        {std::vector<FMCKey>{FMCKey::PFP3_N1_LIMIT, FMCKey::MCDU_PERF}, "laminar/B738/button/fmc1_n1_lim"},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, "laminar/B738/button/fmc1_fix"},
        {FMCKey::PAGE_PREV, "laminar/B738/button/fmc1_prev_page"},
        {FMCKey::PAGE_NEXT, "laminar/B738/button/fmc1_next_page"},
        {FMCKey::KEY1, "laminar/B738/button/fmc1_1"},
        {FMCKey::KEY2, "laminar/B738/button/fmc1_2"},
        {FMCKey::KEY3, "laminar/B738/button/fmc1_3"},
        {FMCKey::KEY4, "laminar/B738/button/fmc1_4"},
        {FMCKey::KEY5, "laminar/B738/button/fmc1_5"},
        {FMCKey::KEY6, "laminar/B738/button/fmc1_6"},
        {FMCKey::KEY7, "laminar/B738/button/fmc1_7"},
        {FMCKey::KEY8, "laminar/B738/button/fmc1_8"},
        {FMCKey::KEY9, "laminar/B738/button/fmc1_9"},
        {FMCKey::PERIOD, "laminar/B738/button/fmc1_period"},
        {FMCKey::KEY0, "laminar/B738/button/fmc1_0"},
        {FMCKey::PLUSMINUS, "laminar/B738/button/fmc1_minus"},
        {FMCKey::KEYA, "laminar/B738/button/fmc1_A"},
        {FMCKey::KEYB, "laminar/B738/button/fmc1_B"},
        {FMCKey::KEYC, "laminar/B738/button/fmc1_C"},
        {FMCKey::KEYD, "laminar/B738/button/fmc1_D"},
        {FMCKey::KEYE, "laminar/B738/button/fmc1_E"},
        {FMCKey::KEYF, "laminar/B738/button/fmc1_F"},
        {FMCKey::KEYG, "laminar/B738/button/fmc1_G"},
        {FMCKey::KEYH, "laminar/B738/button/fmc1_H"},
        {FMCKey::KEYI, "laminar/B738/button/fmc1_I"},
        {FMCKey::KEYJ, "laminar/B738/button/fmc1_J"},
        {FMCKey::KEYK, "laminar/B738/button/fmc1_K"},
        {FMCKey::KEYL, "laminar/B738/button/fmc1_L"},
        {FMCKey::KEYM, "laminar/B738/button/fmc1_M"},
        {FMCKey::KEYN, "laminar/B738/button/fmc1_N"},
        {FMCKey::KEYO, "laminar/B738/button/fmc1_O"},
        {FMCKey::KEYP, "laminar/B738/button/fmc1_P"},
        {FMCKey::KEYQ, "laminar/B738/button/fmc1_Q"},
        {FMCKey::KEYR, "laminar/B738/button/fmc1_R"},
        {FMCKey::KEYS, "laminar/B738/button/fmc1_S"},
        {FMCKey::KEYT, "laminar/B738/button/fmc1_T"},
        {FMCKey::KEYU, "laminar/B738/button/fmc1_U"},
        {FMCKey::KEYV, "laminar/B738/button/fmc1_V"},
        {FMCKey::KEYW, "laminar/B738/button/fmc1_W"},
        {FMCKey::KEYX, "laminar/B738/button/fmc1_X"},
        {FMCKey::KEYY, "laminar/B738/button/fmc1_Y"},
        {FMCKey::KEYZ, "laminar/B738/button/fmc1_Z"},
        {FMCKey::SPACE, "laminar/B738/button/fmc1_SP"},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, "laminar/B738/button/fmc1_del"},
        {FMCKey::SLASH, "laminar/B738/button/fmc1_slash"},
        {FMCKey::CLR, "laminar/B738/button/fmc1_clr"}};

    return buttons;
}

const std::map<char, FMCTextColor> &ZiboFMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {'W', FMCTextColor::COLOR_WHITE},
        {'L', FMCTextColor::COLOR_WHITE},
        {'S', FMCTextColor::COLOR_WHITE}, // White, small
        {'M', FMCTextColor::COLOR_MAGENTA},
        {'G', FMCTextColor::COLOR_GREEN},
        {'C', FMCTextColor::COLOR_CYAN},
        {'I', FMCTextColor::COLOR_WHITE_BG}, // White (should be inverted gray/white)
        {'X', FMCTextColor::COLOR_WHITE},    // White (should be special labels)
    };

    return colMap;
}

void ZiboFMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '*':
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

void ZiboFMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    for (const auto &ref : displayDatarefs()) {
        std::string text = datarefManager->getCached<std::string>(ref.c_str());

        // Handle scratchpad datarefs specially
        if (ref == "laminar/B738/fmc1/Line_entry" || ref == "laminar/B738/fmc1/Line_entry_I") {
            if (!text.empty()) {
                char color = (ref == "laminar/B738/fmc1/Line_entry_I") ? 'I' : 'W';

                // Store scratchpad text for later display on line 13
                for (int i = 0; i < text.size() && i < ProductFMC::PageCharsPerLine; ++i) {
                    char c = text[i];
                    if (c == 0x00) {
                        break; // End of string
                    }
                    if (c != 0x20) { // Skip spaces
                        product->writeLineToPage(page, 13, i, std::string(1, c), color, false);
                    }
                }
            }
            continue;
        }

        std::smatch match;
        if (!std::regex_match(ref, match, datarefRegex)) {
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

        for (int i = 0; i < text.size() && i < ProductFMC::PageCharsPerLine; ++i) {
            char c = text[i];
            if (c == 0x00) {
                break;
            }

            if (c != 0x20) {
                product->writeLineToPage(page, displayLine, i, std::string(1, c), color, fontSmall);
            }
        }
    }
}

void ZiboFMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (std::fabs(button->value) > DBL_EPSILON) {
        if (phase != xplm_CommandBegin) {
            return;
        }

        std::string ref = button->dataref;
        size_t start = ref.find('[');
        if (start != std::string::npos) {
            size_t end = ref.find(']', start);
            int index = std::stoi(ref.substr(start + 1, end - start - 1));
            std::string baseRef = ref.substr(0, start);

            auto vec = Dataref::getInstance()->get<std::vector<float>>(baseRef.c_str());
            if (index >= 0 && index < (int) vec.size()) {
                vec[index] = std::clamp(vec[index] + button->value, 0.0, 1.0);
                Dataref::getInstance()->set<std::vector<float>>(baseRef.c_str(), vec);
            }
        } else {
            Dataref::getInstance()->set<float>(ref.c_str(), button->value);
        }
    } else {
        Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
    }
}
