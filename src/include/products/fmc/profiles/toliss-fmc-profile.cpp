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
    const std::string mcdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "MCDU1" : "MCDU2";
    static const std::vector<std::string> datarefs = {
        "AirbusFBW/" + mcdu + "titleb",
        "AirbusFBW/" + mcdu + "titleg",
        "AirbusFBW/" + mcdu + "titles",
        "AirbusFBW/" + mcdu + "titlew",
        "AirbusFBW/" + mcdu + "titley",
        "AirbusFBW/" + mcdu + "stitley",
        "AirbusFBW/" + mcdu + "stitlew",
        "AirbusFBW/" + mcdu + "label1w",
        "AirbusFBW/" + mcdu + "label2w",
        "AirbusFBW/" + mcdu + "label3w",
        "AirbusFBW/" + mcdu + "label4w",
        "AirbusFBW/" + mcdu + "label5w",
        "AirbusFBW/" + mcdu + "label6w",
        "AirbusFBW/" + mcdu + "label1a",
        "AirbusFBW/" + mcdu + "label2a",
        "AirbusFBW/" + mcdu + "label3a",
        "AirbusFBW/" + mcdu + "label4a",
        "AirbusFBW/" + mcdu + "label5a",
        "AirbusFBW/" + mcdu + "label6a",
        "AirbusFBW/" + mcdu + "label1g",
        "AirbusFBW/" + mcdu + "label2g",
        "AirbusFBW/" + mcdu + "label3g",
        "AirbusFBW/" + mcdu + "label4g",
        "AirbusFBW/" + mcdu + "label5g",
        "AirbusFBW/" + mcdu + "label6g",
        "AirbusFBW/" + mcdu + "label1b",
        "AirbusFBW/" + mcdu + "label2b",
        "AirbusFBW/" + mcdu + "label3b",
        "AirbusFBW/" + mcdu + "label4b",
        "AirbusFBW/" + mcdu + "label5b",
        "AirbusFBW/" + mcdu + "label6b",
        "AirbusFBW/" + mcdu + "label1y",
        "AirbusFBW/" + mcdu + "label2y",
        "AirbusFBW/" + mcdu + "label3y",
        "AirbusFBW/" + mcdu + "label4y",
        "AirbusFBW/" + mcdu + "label5y",
        "AirbusFBW/" + mcdu + "label6y",
        "AirbusFBW/" + mcdu + "label1Lg",
        "AirbusFBW/" + mcdu + "label2Lg",
        "AirbusFBW/" + mcdu + "label3Lg",
        "AirbusFBW/" + mcdu + "label4Lg",
        "AirbusFBW/" + mcdu + "label5Lg",
        "AirbusFBW/" + mcdu + "label6Lg",
        "AirbusFBW/" + mcdu + "cont1b",
        "AirbusFBW/" + mcdu + "cont2b",
        "AirbusFBW/" + mcdu + "cont3b",
        "AirbusFBW/" + mcdu + "cont4b",
        "AirbusFBW/" + mcdu + "cont5b",
        "AirbusFBW/" + mcdu + "cont6b",
        "AirbusFBW/" + mcdu + "cont1m",
        "AirbusFBW/" + mcdu + "cont2m",
        "AirbusFBW/" + mcdu + "cont3m",
        "AirbusFBW/" + mcdu + "cont4m",
        "AirbusFBW/" + mcdu + "cont5m",
        "AirbusFBW/" + mcdu + "cont6m",
        "AirbusFBW/" + mcdu + "scont1m",
        "AirbusFBW/" + mcdu + "scont2m",
        "AirbusFBW/" + mcdu + "scont3m",
        "AirbusFBW/" + mcdu + "scont4m",
        "AirbusFBW/" + mcdu + "scont5m",
        "AirbusFBW/" + mcdu + "scont6m",
        "AirbusFBW/" + mcdu + "cont1a",
        "AirbusFBW/" + mcdu + "cont2a",
        "AirbusFBW/" + mcdu + "cont3a",
        "AirbusFBW/" + mcdu + "cont4a",
        "AirbusFBW/" + mcdu + "cont5a",
        "AirbusFBW/" + mcdu + "cont6a",
        "AirbusFBW/" + mcdu + "scont1a",
        "AirbusFBW/" + mcdu + "scont2a",
        "AirbusFBW/" + mcdu + "scont3a",
        "AirbusFBW/" + mcdu + "scont4a",
        "AirbusFBW/" + mcdu + "scont5a",
        "AirbusFBW/" + mcdu + "scont6a",
        "AirbusFBW/" + mcdu + "cont1w",
        "AirbusFBW/" + mcdu + "cont2w",
        "AirbusFBW/" + mcdu + "cont3w",
        "AirbusFBW/" + mcdu + "cont4w",
        "AirbusFBW/" + mcdu + "cont5w",
        "AirbusFBW/" + mcdu + "cont6w",
        "AirbusFBW/" + mcdu + "cont1g",
        "AirbusFBW/" + mcdu + "cont2g",
        "AirbusFBW/" + mcdu + "cont3g",
        "AirbusFBW/" + mcdu + "cont4g",
        "AirbusFBW/" + mcdu + "cont5g",
        "AirbusFBW/" + mcdu + "cont6g",
        "AirbusFBW/" + mcdu + "cont1c",
        "AirbusFBW/" + mcdu + "cont2c",
        "AirbusFBW/" + mcdu + "cont3c",
        "AirbusFBW/" + mcdu + "cont4c",
        "AirbusFBW/" + mcdu + "cont5c",
        "AirbusFBW/" + mcdu + "cont6c",
        "AirbusFBW/" + mcdu + "scont1g",
        "AirbusFBW/" + mcdu + "scont2g",
        "AirbusFBW/" + mcdu + "scont3g",
        "AirbusFBW/" + mcdu + "scont4g",
        "AirbusFBW/" + mcdu + "scont5g",
        "AirbusFBW/" + mcdu + "scont6g",
        "AirbusFBW/" + mcdu + "cont1s",
        "AirbusFBW/" + mcdu + "cont2s",
        "AirbusFBW/" + mcdu + "cont3s",
        "AirbusFBW/" + mcdu + "cont4s",
        "AirbusFBW/" + mcdu + "cont5s",
        "AirbusFBW/" + mcdu + "cont6s",
        "AirbusFBW/" + mcdu + "scont1b",
        "AirbusFBW/" + mcdu + "scont2b",
        "AirbusFBW/" + mcdu + "scont3b",
        "AirbusFBW/" + mcdu + "scont4b",
        "AirbusFBW/" + mcdu + "scont5b",
        "AirbusFBW/" + mcdu + "scont6b",
        "AirbusFBW/" + mcdu + "cont1y",
        "AirbusFBW/" + mcdu + "cont2y",
        "AirbusFBW/" + mcdu + "cont3y",
        "AirbusFBW/" + mcdu + "cont4y",
        "AirbusFBW/" + mcdu + "cont5y",
        "AirbusFBW/" + mcdu + "cont6y",
        "AirbusFBW/" + mcdu + "scont1w",
        "AirbusFBW/" + mcdu + "scont2w",
        "AirbusFBW/" + mcdu + "scont3w",
        "AirbusFBW/" + mcdu + "scont4w",
        "AirbusFBW/" + mcdu + "scont5w",
        "AirbusFBW/" + mcdu + "scont6w",
        "AirbusFBW/" + mcdu + "scont1y",
        "AirbusFBW/" + mcdu + "scont2y",
        "AirbusFBW/" + mcdu + "scont3y",
        "AirbusFBW/" + mcdu + "scont4y",
        "AirbusFBW/" + mcdu + "scont5y",
        "AirbusFBW/" + mcdu + "scont6y",

        "AirbusFBW/" + mcdu + "spw", // scratchpad
        "AirbusFBW/" + mcdu + "spa"  // scratchpad
    };

    return datarefs;
}

const std::vector<FMCButtonDef> &TolissFMCProfile::buttonDefs() const {
    const std::string mcdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "MCDU1" : "MCDU2";
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "AirbusFBW/" + mcdu + "LSK1L"},
        {FMCKey::LSK2L, "AirbusFBW/" + mcdu + "LSK2L"},
        {FMCKey::LSK3L, "AirbusFBW/" + mcdu + "LSK3L"},
        {FMCKey::LSK4L, "AirbusFBW/" + mcdu + "LSK4L"},
        {FMCKey::LSK5L, "AirbusFBW/" + mcdu + "LSK5L"},
        {FMCKey::LSK6L, "AirbusFBW/" + mcdu + "LSK6L"},
        {FMCKey::LSK1R, "AirbusFBW/" + mcdu + "LSK1R"},
        {FMCKey::LSK2R, "AirbusFBW/" + mcdu + "LSK2R"},
        {FMCKey::LSK3R, "AirbusFBW/" + mcdu + "LSK3R"},
        {FMCKey::LSK4R, "AirbusFBW/" + mcdu + "LSK4R"},
        {FMCKey::LSK5R, "AirbusFBW/" + mcdu + "LSK5R"},
        {FMCKey::LSK6R, "AirbusFBW/" + mcdu + "LSK6R"},
        {FMCKey::MCDU_DIR, "AirbusFBW/" + mcdu + "DirTo"},
        {FMCKey::PROG, "AirbusFBW/" + mcdu + "Prog"},
        {std::vector<FMCKey>{FMCKey::MCDU_PERF, FMCKey::PFP3_N1_LIMIT}, "AirbusFBW/" + mcdu + "Perf"},
        {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP_INIT_REF}, "AirbusFBW/" + mcdu + "Init"},
        {FMCKey::MCDU_DATA, "AirbusFBW/" + mcdu + "Data"},
        {FMCKey::MCDU_EMPTY_TOP_RIGHT, "AirbusFBW/CaptChronoButton"},
        {FMCKey::BRIGHTNESS_UP, "AirbusFBW/" + mcdu + "KeyBright"},
        {std::vector<FMCKey>{FMCKey::MCDU_FPLN, FMCKey::PFP_LEGS}, "AirbusFBW/" + mcdu + "Fpln"},
        {std::vector<FMCKey>{FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD, FMCKey::PFP7_NAV_RAD}, "AirbusFBW/" + mcdu + "RadNav"},
        {FMCKey::MCDU_FUEL_PRED, "AirbusFBW/" + mcdu + "FuelPred"},
        {FMCKey::MCDU_SEC_FPLN, "AirbusFBW/" + mcdu + "SecFpln"},
        {std::vector<FMCKey>{FMCKey::MCDU_ATC_COMM, FMCKey::PFP4_ATC}, "AirbusFBW/" + mcdu + "ATC"},
        {FMCKey::MENU, "AirbusFBW/" + mcdu + "Menu"},
        {FMCKey::BRIGHTNESS_DOWN, "AirbusFBW/" + mcdu + "KeyDim"},
        {std::vector<FMCKey>{FMCKey::MCDU_AIRPORT, FMCKey::PFP_DEP_ARR}, "AirbusFBW/" + mcdu + "Airport"},
        {FMCKey::MCDU_EMPTY_BOTTOM_LEFT, "AirbusFBW/purser/fwd"},
        {FMCKey::PAGE_PREV, "AirbusFBW/" + mcdu + "SlewLeft"},
        {std::vector<FMCKey>{FMCKey::MCDU_PAGE_UP, FMCKey::PAGE_PREV}, "AirbusFBW/" + mcdu + "SlewUp"},
        {FMCKey::PAGE_NEXT, "AirbusFBW/" + mcdu + "SlewRight"},
        {std::vector<FMCKey>{FMCKey::MCDU_PAGE_DOWN, FMCKey::PAGE_NEXT}, "AirbusFBW/" + mcdu + "SlewDown"},
        {FMCKey::KEY1, "AirbusFBW/" + mcdu + "Key1"},
        {FMCKey::KEY2, "AirbusFBW/" + mcdu + "Key2"},
        {FMCKey::KEY3, "AirbusFBW/" + mcdu + "Key3"},
        {FMCKey::KEY4, "AirbusFBW/" + mcdu + "Key4"},
        {FMCKey::KEY5, "AirbusFBW/" + mcdu + "Key5"},
        {FMCKey::KEY6, "AirbusFBW/" + mcdu + "Key6"},
        {FMCKey::KEY7, "AirbusFBW/" + mcdu + "Key7"},
        {FMCKey::KEY8, "AirbusFBW/" + mcdu + "Key8"},
        {FMCKey::KEY9, "AirbusFBW/" + mcdu + "Key9"},
        {FMCKey::PERIOD, "AirbusFBW/" + mcdu + "KeyDecimal"},
        {FMCKey::KEY0, "AirbusFBW/" + mcdu + "Key0"},
        {FMCKey::PLUSMINUS, "AirbusFBW/" + mcdu + "KeyPM"},
        {FMCKey::KEYA, "AirbusFBW/" + mcdu + "KeyA"},
        {FMCKey::KEYB, "AirbusFBW/" + mcdu + "KeyB"},
        {FMCKey::KEYC, "AirbusFBW/" + mcdu + "KeyC"},
        {FMCKey::KEYD, "AirbusFBW/" + mcdu + "KeyD"},
        {FMCKey::KEYE, "AirbusFBW/" + mcdu + "KeyE"},
        {FMCKey::KEYF, "AirbusFBW/" + mcdu + "KeyF"},
        {FMCKey::KEYG, "AirbusFBW/" + mcdu + "KeyG"},
        {FMCKey::KEYH, "AirbusFBW/" + mcdu + "KeyH"},
        {FMCKey::KEYI, "AirbusFBW/" + mcdu + "KeyI"},
        {FMCKey::KEYJ, "AirbusFBW/" + mcdu + "KeyJ"},
        {FMCKey::KEYK, "AirbusFBW/" + mcdu + "KeyK"},
        {FMCKey::KEYL, "AirbusFBW/" + mcdu + "KeyL"},
        {FMCKey::KEYM, "AirbusFBW/" + mcdu + "KeyM"},
        {FMCKey::KEYN, "AirbusFBW/" + mcdu + "KeyN"},
        {FMCKey::KEYO, "AirbusFBW/" + mcdu + "KeyO"},
        {FMCKey::KEYP, "AirbusFBW/" + mcdu + "KeyP"},
        {FMCKey::KEYQ, "AirbusFBW/" + mcdu + "KeyQ"},
        {FMCKey::KEYR, "AirbusFBW/" + mcdu + "KeyR"},
        {FMCKey::KEYS, "AirbusFBW/" + mcdu + "KeyS"},
        {FMCKey::KEYT, "AirbusFBW/" + mcdu + "KeyT"},
        {FMCKey::KEYU, "AirbusFBW/" + mcdu + "KeyU"},
        {FMCKey::KEYV, "AirbusFBW/" + mcdu + "KeyV"},
        {FMCKey::KEYW, "AirbusFBW/" + mcdu + "KeyW"},
        {FMCKey::KEYX, "AirbusFBW/" + mcdu + "KeyX"},
        {FMCKey::KEYY, "AirbusFBW/" + mcdu + "KeyY"},
        {FMCKey::KEYZ, "AirbusFBW/" + mcdu + "KeyZ"},
        {FMCKey::SLASH, "AirbusFBW/" + mcdu + "KeySlash"},
        {FMCKey::SPACE, "AirbusFBW/" + mcdu + "KeySpace"},
        {std::vector<FMCKey>{FMCKey::MCDU_OVERFLY, FMCKey::PFP_DEL}, "AirbusFBW/" + mcdu + "KeyOverfly"},
        {FMCKey::CLR, "AirbusFBW/" + mcdu + "KeyClear"},
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
    int vertSlewType = Dataref::getInstance()->getCached<int>(product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "AirbusFBW/MCDU1VertSlewKeys" : "AirbusFBW/MCDU2VertSlewKeys");
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
