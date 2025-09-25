#include "toliss-fmc-profile.h"

#include "config.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>

TolissFMCProfile::TolissFMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    datarefRegex = std::regex("AirbusFBW/MCDU(1|2)([s]{0,1})([a-zA-Z]+)([0-6]{0,1})([L]{0,1})([a-z]{1})");

    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::FontAirbus, product->identifierByte));

    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/DUBrightness", [product](std::vector<float> brightness) {
        if (brightness.size() < 8) {
            return;
        }

        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[6] * 255.0f : 0;
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/DUBrightness");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    });
}

TolissFMCProfile::~TolissFMCProfile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("AirbusFBW/DUBrightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool TolissFMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::vector<std::string> &TolissFMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "AirbusFBW/MCDU1titleb",
        "AirbusFBW/MCDU1titleg",
        "AirbusFBW/MCDU1titles",
        "AirbusFBW/MCDU1titlew",
        "AirbusFBW/MCDU1titley",
        "AirbusFBW/MCDU1stitley",
        "AirbusFBW/MCDU1stitlew",
        "AirbusFBW/MCDU1label1w",
        "AirbusFBW/MCDU1label2w",
        "AirbusFBW/MCDU1label3w",
        "AirbusFBW/MCDU1label4w",
        "AirbusFBW/MCDU1label5w",
        "AirbusFBW/MCDU1label6w",
        "AirbusFBW/MCDU1label1a",
        "AirbusFBW/MCDU1label2a",
        "AirbusFBW/MCDU1label3a",
        "AirbusFBW/MCDU1label4a",
        "AirbusFBW/MCDU1label5a",
        "AirbusFBW/MCDU1label6a",
        "AirbusFBW/MCDU1label1g",
        "AirbusFBW/MCDU1label2g",
        "AirbusFBW/MCDU1label3g",
        "AirbusFBW/MCDU1label4g",
        "AirbusFBW/MCDU1label5g",
        "AirbusFBW/MCDU1label6g",
        "AirbusFBW/MCDU1label1b",
        "AirbusFBW/MCDU1label2b",
        "AirbusFBW/MCDU1label3b",
        "AirbusFBW/MCDU1label4b",
        "AirbusFBW/MCDU1label5b",
        "AirbusFBW/MCDU1label6b",
        "AirbusFBW/MCDU1label1y",
        "AirbusFBW/MCDU1label2y",
        "AirbusFBW/MCDU1label3y",
        "AirbusFBW/MCDU1label4y",
        "AirbusFBW/MCDU1label5y",
        "AirbusFBW/MCDU1label6y",
        "AirbusFBW/MCDU1label1Lg",
        "AirbusFBW/MCDU1label2Lg",
        "AirbusFBW/MCDU1label3Lg",
        "AirbusFBW/MCDU1label4Lg",
        "AirbusFBW/MCDU1label5Lg",
        "AirbusFBW/MCDU1label6Lg",
        "AirbusFBW/MCDU1cont1b",
        "AirbusFBW/MCDU1cont2b",
        "AirbusFBW/MCDU1cont3b",
        "AirbusFBW/MCDU1cont4b",
        "AirbusFBW/MCDU1cont5b",
        "AirbusFBW/MCDU1cont6b",
        "AirbusFBW/MCDU1cont1m",
        "AirbusFBW/MCDU1cont2m",
        "AirbusFBW/MCDU1cont3m",
        "AirbusFBW/MCDU1cont4m",
        "AirbusFBW/MCDU1cont5m",
        "AirbusFBW/MCDU1cont6m",
        "AirbusFBW/MCDU1scont1m",
        "AirbusFBW/MCDU1scont2m",
        "AirbusFBW/MCDU1scont3m",
        "AirbusFBW/MCDU1scont4m",
        "AirbusFBW/MCDU1scont5m",
        "AirbusFBW/MCDU1scont6m",
        "AirbusFBW/MCDU1cont1a",
        "AirbusFBW/MCDU1cont2a",
        "AirbusFBW/MCDU1cont3a",
        "AirbusFBW/MCDU1cont4a",
        "AirbusFBW/MCDU1cont5a",
        "AirbusFBW/MCDU1cont6a",
        "AirbusFBW/MCDU1scont1a",
        "AirbusFBW/MCDU1scont2a",
        "AirbusFBW/MCDU1scont3a",
        "AirbusFBW/MCDU1scont4a",
        "AirbusFBW/MCDU1scont5a",
        "AirbusFBW/MCDU1scont6a",
        "AirbusFBW/MCDU1cont1w",
        "AirbusFBW/MCDU1cont2w",
        "AirbusFBW/MCDU1cont3w",
        "AirbusFBW/MCDU1cont4w",
        "AirbusFBW/MCDU1cont5w",
        "AirbusFBW/MCDU1cont6w",
        "AirbusFBW/MCDU1cont1g",
        "AirbusFBW/MCDU1cont2g",
        "AirbusFBW/MCDU1cont3g",
        "AirbusFBW/MCDU1cont4g",
        "AirbusFBW/MCDU1cont5g",
        "AirbusFBW/MCDU1cont6g",
        "AirbusFBW/MCDU1cont1c",
        "AirbusFBW/MCDU1cont2c",
        "AirbusFBW/MCDU1cont3c",
        "AirbusFBW/MCDU1cont4c",
        "AirbusFBW/MCDU1cont5c",
        "AirbusFBW/MCDU1cont6c",
        "AirbusFBW/MCDU1scont1g",
        "AirbusFBW/MCDU1scont2g",
        "AirbusFBW/MCDU1scont3g",
        "AirbusFBW/MCDU1scont4g",
        "AirbusFBW/MCDU1scont5g",
        "AirbusFBW/MCDU1scont6g",
        "AirbusFBW/MCDU1cont1s",
        "AirbusFBW/MCDU1cont2s",
        "AirbusFBW/MCDU1cont3s",
        "AirbusFBW/MCDU1cont4s",
        "AirbusFBW/MCDU1cont5s",
        "AirbusFBW/MCDU1cont6s",
        "AirbusFBW/MCDU1scont1b",
        "AirbusFBW/MCDU1scont2b",
        "AirbusFBW/MCDU1scont3b",
        "AirbusFBW/MCDU1scont4b",
        "AirbusFBW/MCDU1scont5b",
        "AirbusFBW/MCDU1scont6b",
        "AirbusFBW/MCDU1cont1y",
        "AirbusFBW/MCDU1cont2y",
        "AirbusFBW/MCDU1cont3y",
        "AirbusFBW/MCDU1cont4y",
        "AirbusFBW/MCDU1cont5y",
        "AirbusFBW/MCDU1cont6y",
        "AirbusFBW/MCDU1scont1w",
        "AirbusFBW/MCDU1scont2w",
        "AirbusFBW/MCDU1scont3w",
        "AirbusFBW/MCDU1scont4w",
        "AirbusFBW/MCDU1scont5w",
        "AirbusFBW/MCDU1scont6w",
        "AirbusFBW/MCDU1scont1y",
        "AirbusFBW/MCDU1scont2y",
        "AirbusFBW/MCDU1scont3y",
        "AirbusFBW/MCDU1scont4y",
        "AirbusFBW/MCDU1scont5y",
        "AirbusFBW/MCDU1scont6y",

        "AirbusFBW/MCDU1spw", // scratchpad
        "AirbusFBW/MCDU1spa"  // scratchpad
    };

    return datarefs;
}

const std::vector<FMCButtonDef> &TolissFMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "AirbusFBW/MCDU1LSK1L"},
        {FMCKey::LSK2L, "AirbusFBW/MCDU1LSK2L"},
        {FMCKey::LSK3L, "AirbusFBW/MCDU1LSK3L"},
        {FMCKey::LSK4L, "AirbusFBW/MCDU1LSK4L"},
        {FMCKey::LSK5L, "AirbusFBW/MCDU1LSK5L"},
        {FMCKey::LSK6L, "AirbusFBW/MCDU1LSK6L"},
        {FMCKey::LSK1R, "AirbusFBW/MCDU1LSK1R"},
        {FMCKey::LSK2R, "AirbusFBW/MCDU1LSK2R"},
        {FMCKey::LSK3R, "AirbusFBW/MCDU1LSK3R"},
        {FMCKey::LSK4R, "AirbusFBW/MCDU1LSK4R"},
        {FMCKey::LSK5R, "AirbusFBW/MCDU1LSK5R"},
        {FMCKey::LSK6R, "AirbusFBW/MCDU1LSK6R"},
        {FMCKey::MCDU_DIR, "AirbusFBW/MCDU1DirTo"},
        {FMCKey::PROG, "AirbusFBW/MCDU1Prog"},
        {std::vector<FMCKey>{FMCKey::MCDU_PERF, FMCKey::PFP3_N1_LIMIT}, "AirbusFBW/MCDU1Perf"},
        {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP_INIT_REF}, "AirbusFBW/MCDU1Init"},
        {FMCKey::MCDU_DATA, "AirbusFBW/MCDU1Data"},
        {FMCKey::MCDU_EMPTY_TOP_RIGHT, "AirbusFBW/CaptChronoButton"},
        {FMCKey::BRIGHTNESS_UP, "AirbusFBW/MCDU1KeyBright"},
        {std::vector<FMCKey>{FMCKey::MCDU_FPLN, FMCKey::PFP_LEGS}, "AirbusFBW/MCDU1Fpln"},
        {std::vector<FMCKey>{FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD, FMCKey::PFP7_NAV_RAD}, "AirbusFBW/MCDU1RadNav"},
        {FMCKey::MCDU_FUEL_PRED, "AirbusFBW/MCDU1FuelPred"},
        {FMCKey::MCDU_SEC_FPLN, "AirbusFBW/MCDU1SecFpln"},
        {std::vector<FMCKey>{FMCKey::MCDU_ATC_COMM, FMCKey::PFP4_ATC}, "AirbusFBW/MCDU1ATC"},
        {FMCKey::MENU, "AirbusFBW/MCDU1Menu"},
        {FMCKey::BRIGHTNESS_DOWN, "AirbusFBW/MCDU1KeyDim"},
        {std::vector<FMCKey>{FMCKey::MCDU_AIRPORT, FMCKey::PFP_DEP_ARR}, "AirbusFBW/MCDU1Airport"},
        {FMCKey::MCDU_EMPTY_BOTTOM_LEFT, "AirbusFBW/purser/fwd"},
        {FMCKey::PAGE_PREV, "AirbusFBW/MCDU1SlewLeft"},
        {std::vector<FMCKey>{FMCKey::MCDU_PAGE_UP, FMCKey::PAGE_PREV}, "AirbusFBW/MCDU1SlewUp"},
        {FMCKey::PAGE_NEXT, "AirbusFBW/MCDU1SlewRight"},
        {std::vector<FMCKey>{FMCKey::MCDU_PAGE_DOWN, FMCKey::PAGE_NEXT}, "AirbusFBW/MCDU1SlewDown"},
        {FMCKey::KEY1, "AirbusFBW/MCDU1Key1"},
        {FMCKey::KEY2, "AirbusFBW/MCDU1Key2"},
        {FMCKey::KEY3, "AirbusFBW/MCDU1Key3"},
        {FMCKey::KEY4, "AirbusFBW/MCDU1Key4"},
        {FMCKey::KEY5, "AirbusFBW/MCDU1Key5"},
        {FMCKey::KEY6, "AirbusFBW/MCDU1Key6"},
        {FMCKey::KEY7, "AirbusFBW/MCDU1Key7"},
        {FMCKey::KEY8, "AirbusFBW/MCDU1Key8"},
        {FMCKey::KEY9, "AirbusFBW/MCDU1Key9"},
        {FMCKey::PERIOD, "AirbusFBW/MCDU1KeyDecimal"},
        {FMCKey::KEY0, "AirbusFBW/MCDU1Key0"},
        {FMCKey::PLUSMINUS, "AirbusFBW/MCDU1KeyPM"},
        {FMCKey::KEYA, "AirbusFBW/MCDU1KeyA"},
        {FMCKey::KEYB, "AirbusFBW/MCDU1KeyB"},
        {FMCKey::KEYC, "AirbusFBW/MCDU1KeyC"},
        {FMCKey::KEYD, "AirbusFBW/MCDU1KeyD"},
        {FMCKey::KEYE, "AirbusFBW/MCDU1KeyE"},
        {FMCKey::KEYF, "AirbusFBW/MCDU1KeyF"},
        {FMCKey::KEYG, "AirbusFBW/MCDU1KeyG"},
        {FMCKey::KEYH, "AirbusFBW/MCDU1KeyH"},
        {FMCKey::KEYI, "AirbusFBW/MCDU1KeyI"},
        {FMCKey::KEYJ, "AirbusFBW/MCDU1KeyJ"},
        {FMCKey::KEYK, "AirbusFBW/MCDU1KeyK"},
        {FMCKey::KEYL, "AirbusFBW/MCDU1KeyL"},
        {FMCKey::KEYM, "AirbusFBW/MCDU1KeyM"},
        {FMCKey::KEYN, "AirbusFBW/MCDU1KeyN"},
        {FMCKey::KEYO, "AirbusFBW/MCDU1KeyO"},
        {FMCKey::KEYP, "AirbusFBW/MCDU1KeyP"},
        {FMCKey::KEYQ, "AirbusFBW/MCDU1KeyQ"},
        {FMCKey::KEYR, "AirbusFBW/MCDU1KeyR"},
        {FMCKey::KEYS, "AirbusFBW/MCDU1KeyS"},
        {FMCKey::KEYT, "AirbusFBW/MCDU1KeyT"},
        {FMCKey::KEYU, "AirbusFBW/MCDU1KeyU"},
        {FMCKey::KEYV, "AirbusFBW/MCDU1KeyV"},
        {FMCKey::KEYW, "AirbusFBW/MCDU1KeyW"},
        {FMCKey::KEYX, "AirbusFBW/MCDU1KeyX"},
        {FMCKey::KEYY, "AirbusFBW/MCDU1KeyY"},
        {FMCKey::KEYZ, "AirbusFBW/MCDU1KeyZ"},
        {FMCKey::SLASH, "AirbusFBW/MCDU1KeySlash"},
        {FMCKey::SPACE, "AirbusFBW/MCDU1KeySpace"},
        {std::vector<FMCKey>{FMCKey::MCDU_OVERFLY, FMCKey::PFP_DEL}, "AirbusFBW/MCDU1KeyOverfly"},
        {FMCKey::CLR, "AirbusFBW/MCDU1KeyClear"},
    };

    return buttons;
}

const std::map<char, FMCTextColor> &TolissFMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {'a', FMCTextColor::COLOR_AMBER},
        {'w', FMCTextColor::COLOR_WHITE},
        {'b', FMCTextColor::COLOR_CYAN},
        {'g', FMCTextColor::COLOR_GREEN},
        {'m', FMCTextColor::COLOR_MAGENTA},
        {'r', FMCTextColor::COLOR_RED},
        {'y', FMCTextColor::COLOR_YELLOW},
        {'e', FMCTextColor::COLOR_GREY},
    };

    return colMap;
}

void TolissFMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;

        case '<':
            if (isFontSmall) {
                buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_LEFT.begin(), FMCSpecialCharacter::ARROW_LEFT.end());
            } else {
                buffer->push_back(character);
            }
            break;

        case '>':
            if (isFontSmall) {
                buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_RIGHT.begin(), FMCSpecialCharacter::ARROW_RIGHT.end());
            } else {
                buffer->push_back(character);
            }
            break;

        case 30: // Up arrow
            if (isFontSmall) {
                buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_UP.begin(), FMCSpecialCharacter::ARROW_UP.end());
            }
            break;

        case 31: // Down arrow
            if (isFontSmall) {
                buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_DOWN.begin(), FMCSpecialCharacter::ARROW_DOWN.end());
            } else {
                buffer->push_back(character);
            }
            break;

        case '`':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;

        case '|':
            buffer->insert(buffer->end(), FMCSpecialCharacter::TRIANGLE.begin(), FMCSpecialCharacter::TRIANGLE.end());
            break;

        default:
            buffer->push_back(character);
            break;
    }
}

void TolissFMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    std::array<int, ProductFMC::PageBytesPerLine> spw_line{};
    std::array<int, ProductFMC::PageBytesPerLine> spa_line{};
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    for (const auto &ref : displayDatarefs()) {
        bool isScratchpad = (ref.size() >= 3 && (ref.substr(ref.size() - 3) == "spw" || ref.substr(ref.size() - 3) == "spa"));

        std::smatch match;
        if (!std::regex_match(ref, match, datarefRegex) && !isScratchpad) {
            continue;
        }

        unsigned char mcduIndex = std::stoi(match[1]);
        if (mcduIndex != 1) {
            continue;
        }

        std::string type = match[3];
        unsigned char line = match[4].str().empty() ? 0 : std::stoi(match[4]) * 2;
        char color = match[6].str()[0];
        bool fontSmall = match[2] == "s" || (type == "label" && match[5] != "L") || color == 's';

        std::string text = datarefManager->getCached<std::string>(ref.c_str());
        if (text.empty()) {
            continue;
        }

        // Process text characters
        for (int i = 0; i < text.size(); ++i) {
            char c = text[i];
            if (c == 0x00 || (c == 0x20 && !isScratchpad)) {
                continue;
            }

            unsigned char targetColor = color;
            if (color == 's') {
                switch (c) {
                    case 'A':
                        c = 91;
                        targetColor = 'b';
                        break;
                    case 'B':
                        c = 93;
                        targetColor = 'b';
                        break;
                    case '0':
                        c = 60;
                        targetColor = 'b';
                        break;
                    case '1':
                        c = 62;
                        targetColor = 'b';
                        break;
                    case '2':
                        c = 60;
                        targetColor = 'w';
                        break;
                    case '3':
                        c = 62;
                        targetColor = 'w';
                        break;
                    case '4':
                        c = 60;
                        targetColor = 'a';
                        break;
                    case '5':
                        c = 62;
                        targetColor = 'a';
                        break;
                    case 'E':
                        c = 35;
                        targetColor = 'a';
                        break;
                }
            }

            if (type.find("title") != std::string::npos || type.find("stitle") != std::string::npos) {
                product->writeLineToPage(page, 0, i, std::string(1, c), targetColor, fontSmall);
            } else if (type.find("label") != std::string::npos) {
                unsigned char lbl_line = (match[4].str().empty() ? 1 : std::stoi(match[4])) * 2 - 1;
                product->writeLineToPage(page, lbl_line, i, std::string(1, c), targetColor, fontSmall);
            } else if (type.find("cont") != std::string::npos || type.find("scont") != std::string::npos) {
                product->writeLineToPage(page, line, i, std::string(1, c), targetColor, fontSmall);
            } else if (isScratchpad) {
                if (ref.size() >= 3 && ref.substr(ref.size() - 3) == "spw") {
                    spw_line[i] = c;
                } else {
                    if (i <= 21) {
                        spa_line[i] = c;
                    }
                }
            }
        }
    }

    for (int i = 0; i < ProductFMC::PageCharsPerLine; ++i) {
        if (spw_line[i] == 0) {
            std::fill(spw_line.begin() + i, spw_line.end(), 0);
            break;
        }
    }
    for (int i = 0; i < ProductFMC::PageCharsPerLine; ++i) {
        if (spa_line[i] == 0) {
            std::fill(spa_line.begin() + i, spa_line.end(), 0);
            break;
        }
    }

    // Merge spw and spa into line 13
    int vertSlewType = Dataref::getInstance()->getCached<int>("AirbusFBW/MCDU1VertSlewKeys");
    for (int i = 0; i < ProductFMC::PageCharsPerLine; ++i) {
        bool smallFont = false;
        char dispChar = ' ';
        char dispColor = 'w';
        if (spw_line[i] != 0 && spa_line[i] == 0) {
            dispChar = spw_line[i];
            dispColor = 'w';
        } else if (spa_line[i] != 0) {
            dispChar = spa_line[i];
            dispColor = 'a';
        }

        if (vertSlewType > 0 && i >= ProductFMC::PageCharsPerLine - 2) {
            if (i == ProductFMC::PageCharsPerLine - 2 && (vertSlewType == 1 || vertSlewType == 2)) {
                dispChar = 30; // Up character
            } else if (i == ProductFMC::PageCharsPerLine - 1 && (vertSlewType == 1 || vertSlewType == 3)) {
                dispChar = 31; // Down character
            }

            dispColor = 'w';
            smallFont = true;
        }

        product->writeLineToPage(page, 13, i, std::string(1, dispChar), dispColor, smallFont);
    }
}

void TolissFMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
}
