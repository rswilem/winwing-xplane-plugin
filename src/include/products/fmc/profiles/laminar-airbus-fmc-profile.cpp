#include "laminar-airbus-fmc-profile.h"
#include "product-fmc.h"
#include "dataref.h"
#include <cstring>
#include <algorithm>

LaminarFMCProfile::LaminarFMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);
    
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("sim/cockpit2/electrical/instrument_brightness_ratio", [product](std::vector<float> brightness) {
        if (brightness.size() <= 6) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/avionics_on") ? brightness[6] * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/instrument_brightness_ratio");
    });
    
    product->setLedBrightness(FMCLed::BACKLIGHT, 128);
    product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, 128);
}

LaminarFMCProfile::~LaminarFMCProfile() {
    Dataref::getInstance()->unbind("sim/cockpit2/electrical/instrument_brightness_ratio");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool LaminarFMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("laminar/A333/ckpt_temp");
}

const std::vector<std::string>& LaminarFMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        // Text content for lines 0-15
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line0",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line1",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line2",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line3",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line4",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line5",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line6",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line7",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line8",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line9",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line10",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line11",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line12",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line13",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line14",
        "sim/cockpit2/radios/indicators/fms_cdu1_text_line15"
    };
    return datarefs;
}

const std::vector<FMCButtonDef>& LaminarFMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {0, "LSK1L", "sim/FMS/ls_1l"},
        {1, "LSK2L", "sim/FMS/ls_2l"},
        {2, "LSK3L", "sim/FMS/ls_3l"},
        {3, "LSK4L", "sim/FMS/ls_4l"},
        {4, "LSK5L", "sim/FMS/ls_5l"},
        {5, "LSK6L", "sim/FMS/ls_6l"},
        {6, "LSK1R", "sim/FMS/ls_1r"},
        {7, "LSK2R", "sim/FMS/ls_2r"},
        {8, "LSK3R", "sim/FMS/ls_3r"},
        {9, "LSK4R", "sim/FMS/ls_4r"},
        {10, "LSK5R", "sim/FMS/ls_5r"},
        {11, "LSK6R", "sim/FMS/ls_6r"},
        {12, "DIRTO", "sim/FMS/dir_intc"},
        {13, "PROG", "sim/FMS/prog"},
        {14, "PERF", "sim/FMS/perf"},
        {15, "INIT", "sim/FMS/index"},
        {16, "DATA", "sim/FMS/data"},
        {17, "EMPTY_TOP_RIGHT", ""},
        {18, "BRT", "laminar/A333/buttons/fms1_brightness_up"},
        {19, "FPLN", "sim/FMS/fpln"},
        {20, "RADNAV", "sim/FMS/navrad"},
        {21, "FUEL", "sim/FMS/fuel_pred"},
        {22, "SEC-FPLN", ""},
        {23, "ATC", ""},
        {24, "MENU", "sim/FMS/menu"},
        {25, "DIM", "laminar/A333/buttons/fms1_brightness_dn"},
        {26, "AIRPORT", "sim/FMS/airport"},
        {27, "EMPTY_BOTTOM_LEFT", ""},
        {28, "SLEW_LEFT", "sim/FMS/prev"},
        {29, "SLEW_UP", "sim/FMS/up"},
        {30, "SLEW_RIGHT", "sim/FMS/next"},
        {31, "SLEW_DOWN", "sim/FMS/down"},
        {32, "KEY1", "sim/FMS/key_1"},
        {33, "KEY2", "sim/FMS/key_2"},
        {34, "KEY3", "sim/FMS/key_1"},
        {35, "KEY4", "sim/FMS/key_1"},
        {36, "KEY5", "sim/FMS/key_1"},
        {37, "KEY6", "sim/FMS/key_1"},
        {38, "KEY7", "sim/FMS/key_1"},
        {39, "KEY8", "sim/FMS/key_1"},
        {40, "KEY9", "sim/FMS/key_1"},
        {41, "DOT", "sim/FMS/key_period"},
        {42, "KEY0", "sim/FMS/key_0"},
        {43, "PLUSMINUS", "sim/FMS/key_minus"},
        {44, "KEYA", "sim/FMS/key_A"},
        {45, "KEYB", "sim/FMS/key_B"},
        {46, "KEYC", "sim/FMS/key_C"},
        {47, "KEYD", "sim/FMS/key_D"},
        {48, "KEYE", "sim/FMS/key_E"},
        {49, "KEYF", "sim/FMS/key_F"},
        {50, "KEYG", "sim/FMS/key_G"},
        {51, "KEYH", "sim/FMS/key_H"},
        {52, "KEYI", "sim/FMS/key_I"},
        {53, "KEYJ", "sim/FMS/key_J"},
        {54, "KEYK", "sim/FMS/key_K"},
        {55, "KEYL", "sim/FMS/key_L"},
        {56, "KEYM", "sim/FMS/key_M"},
        {57, "KEYN", "sim/FMS/key_N"},
        {58, "KEYO", "sim/FMS/key_O"},
        {59, "KEYP", "sim/FMS/key_P"},
        {60, "KEYQ", "sim/FMS/key_Q"},
        {61, "KEYR", "sim/FMS/key_R"},
        {62, "KEYS", "sim/FMS/key_S"},
        {63, "KEYT", "sim/FMS/key_T"},
        {64, "KEYU", "sim/FMS/key_U"},
        {65, "KEYV", "sim/FMS/key_V"},
        {66, "KEYW", "sim/FMS/key_W"},
        {67, "KEYX", "sim/FMS/key_X"},
        {68, "KEYY", "sim/FMS/key_Y"},
        {69, "KEYZ", "sim/FMS/key_Z"},
        {70, "SLASH", "sim/FMS/key_slash"},
        {71, "SPACE", "sim/FMS/key_space"},
        {72, "OVERFLY", "sim/FMS/key_overfly"},
        {73, "Clear", "sim/FMS/key_clear"},
    };
    return buttons;
}

const std::map<char, FMCTextColor>& LaminarFMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {0x00, FMCTextColor::COLOR_WHITE},
        {0x01, FMCTextColor::COLOR_CYAN},
        {0x04, FMCTextColor::COLOR_GREEN},
        {0x06, FMCTextColor::COLOR_AMBER}
    };
    return colMap;
}

void LaminarFMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;

        case '<':
            if (isFontSmall) {
                buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_LEFT.begin(), FMCSpecialCharacter::ARROW_LEFT.end());
            }
            else {
                buffer->push_back(character);
            }
            break;

        case '>':
            if (isFontSmall) {
                buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_RIGHT.begin(), FMCSpecialCharacter::ARROW_RIGHT.end());
            }
            else {
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
            }
            else {
                buffer->push_back(character);
            }
            break;

        case '`':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;
        
        default:
            buffer->push_back(character);
            break;
    }
}

void LaminarFMCProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    for (int lineNum = 0; lineNum < std::min(ProductFMC::PageLines, (unsigned int)16); ++lineNum) {
        std::string textDataref = "sim/cockpit2/radios/indicators/fms_cdu1_text_line" + std::to_string(lineNum);
        std::string styleDataref = "sim/cockpit2/radios/indicators/fms_cdu1_style_line" + std::to_string(lineNum);
        
        std::string text = datarefManager->getCached<std::string>(textDataref.c_str());
        if (text.empty()) {
            continue;
        }
        
        std::vector<unsigned char> styleBytes = datarefManager->getCached<std::vector<unsigned char>>(styleDataref.c_str());

        // Replace all special characters with placeholders
        const std::vector<std::pair<std::string, unsigned char>> symbols = {
            { "\u2190", '<' },
            { "\u2192", '>' },
            { "\u2191", 30 },
            { "\u2193", 31 },
            { "\u2610", '#' },
            { "\u00B0", '`' }
        };
        
        for (const auto& symbol : symbols) {
            size_t pos = 0;
            while ((pos = text.find(symbol.first, pos)) != std::string::npos) {
              text.replace(pos, symbol.first.length(), std::string(1, static_cast<char>(symbol.second)));
              pos += 1;
            }
        }
        
        for (size_t i = 0; i < text.size(); ++i) {
            if (static_cast<unsigned char>(text[i]) > 127) {
                text[i] = '?';
            }
        }

        for (int i = 0; i < text.size() && i < ProductFMC::PageCharsPerLine; ++i) {
            char c = text[i];
            if (c == 0x00) {
                continue;
            }

            bool fontSmall = false;
            unsigned char styleByte = (i < styleBytes.size()) ? styleBytes[i] : 0x00;
            fontSmall = (styleByte & 0xF0) == 0x00;
            
            int displayLine = lineNum;
            if (displayLine >= ProductFMC::PageLines) {
                break;
            }

            product->writeLineToPage(page, displayLine, i, std::string(1, c), (styleByte & 0x0F), fontSmall);
        }
    }
}

void LaminarFMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
}
