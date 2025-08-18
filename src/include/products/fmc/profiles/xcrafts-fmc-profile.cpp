#include "xcrafts-fmc-profile.h"
#include "product-fmc.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <XPLMUtilities.h>
#include <XPLMProcessing.h>

XCraftsFMCProfile::XCraftsFMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    datarefRegex = std::regex("XCrafts/FMS/CDU_1_([0-9]{2}|ScratchPad)");
    
    product->setAllLedsEnabled(false);
    
    // Monitor avionics power for LED control
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [product](bool poweredOn) {
        // Set appropriate brightness based on avionics power
        uint8_t brightness = poweredOn ? 255 : 0;
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, brightness);
        product->setLedBrightness(FMCLed::BACKLIGHT, brightness);
    });
    
#if DEBUG
    AppState::getInstance()->executeAfter(3000, [](){
        Dataref::getInstance()->executeCommand("XCrafts/ERJ/OPS/GPU_toggle");
    });
    
    AppState::getInstance()->executeAfter(4000, [](){
        Dataref::getInstance()->set<int>("XCrafts/electric/GPU_sw", 1);
    });
#endif
}

XCraftsFMCProfile::~XCraftsFMCProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool XCraftsFMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("XCrafts/FMS/CDU_1_01");
}

const std::vector<std::string>& XCraftsFMCProfile::displayDatarefs() const {
    static std::vector<std::string> datarefs;
    
    // Generate all available XCrafts FMS datarefs (up to 70 as mentioned)
    if (datarefs.empty()) {
        // Add data count dataref
        datarefs.push_back("XCrafts/FMS/data_count1");
        
        // Add all CDU display datarefs
        for (int i = 1; i <= 70; i++) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "XCrafts/FMS/CDU_1_%02d", i);
            datarefs.push_back(std::string(buffer));
        }
        
        // Add scratchpad
        datarefs.push_back("XCrafts/FMS/CDU_1_ScratchPad");
    }
    
    return datarefs;
}

const std::vector<FMCButtonDef>& XCraftsFMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        // Line Select Keys (XCrafts ERJ format)
        {FMCKey::LSK1L, "XCrafts/ERJ/CDU_1/LSK1"},
        {FMCKey::LSK2L, "XCrafts/ERJ/CDU_1/LSK2"},
        {FMCKey::LSK3L, "XCrafts/ERJ/CDU_1/LSK3"},
        {FMCKey::LSK4L, "XCrafts/ERJ/CDU_1/LSK4"},
        {FMCKey::LSK5L, "XCrafts/ERJ/CDU_1/LSK5"},
        {FMCKey::LSK6L, "XCrafts/ERJ/CDU_1/LSK6"},
        {FMCKey::LSK1R, "XCrafts/ERJ/CDU_1/RSK1"},
        {FMCKey::LSK2R, "XCrafts/ERJ/CDU_1/RSK2"},
        {FMCKey::LSK3R, "XCrafts/ERJ/CDU_1/RSK3"},
        {FMCKey::LSK4R, "XCrafts/ERJ/CDU_1/RSK4"},
        {FMCKey::LSK5R, "XCrafts/ERJ/CDU_1/RSK5"},
        {FMCKey::LSK6R, "XCrafts/ERJ/CDU_1/RSK6"},
        
        // Function Keys
        {FMCKey::N1_LIMIT_OR_PERF, "XCrafts/ERJ/CDU_1/Key_PERF"},
        {FMCKey::FPLN, "XCrafts/ERJ/CDU_1/Key_NAV"},
        
        // Navigation
        {FMCKey::PAGE_PREV, "XCrafts/ERJ/CDU_1/Key_PREV"},
        {FMCKey::PAGE_NEXT, "XCrafts/ERJ/CDU_1/Key_NEXT"},
        
        // Numeric Keys
        {FMCKey::KEY1, "XCrafts/ERJ/CDU_1/Key_1"},
        {FMCKey::KEY2, "XCrafts/ERJ/CDU_1/Key_2"},
        {FMCKey::KEY3, "XCrafts/ERJ/CDU_1/Key_3"},
        {FMCKey::KEY4, "XCrafts/ERJ/CDU_1/Key_4"},
        {FMCKey::KEY5, "XCrafts/ERJ/CDU_1/Key_5"},
        {FMCKey::KEY6, "XCrafts/ERJ/CDU_1/Key_6"},
        {FMCKey::KEY7, "XCrafts/ERJ/CDU_1/Key_7"},
        {FMCKey::KEY8, "XCrafts/ERJ/CDU_1/Key_8"},
        {FMCKey::KEY9, "XCrafts/ERJ/CDU_1/Key_9"},
        {FMCKey::KEY0, "XCrafts/ERJ/CDU_1/Key_0"},
        {FMCKey::PERIOD, "XCrafts/ERJ/CDU_1/Key_Decimal"},
        {FMCKey::PLUSMINUS, "XCrafts/ERJ/CDU_1/Key_PlusMinus"},
        
        // Alpha Keys
        {FMCKey::KEYA, "XCrafts/ERJ/CDU_1/Key_A"},
        {FMCKey::KEYB, "XCrafts/ERJ/CDU_1/Key_B"},
        {FMCKey::KEYC, "XCrafts/ERJ/CDU_1/Key_C"},
        {FMCKey::KEYD, "XCrafts/ERJ/CDU_1/Key_D"},
        {FMCKey::KEYE, "XCrafts/ERJ/CDU_1/Key_E"},
        {FMCKey::KEYF, "XCrafts/ERJ/CDU_1/Key_F"},
        {FMCKey::KEYG, "XCrafts/ERJ/CDU_1/Key_G"},
        {FMCKey::KEYH, "XCrafts/ERJ/CDU_1/Key_H"},
        {FMCKey::KEYI, "XCrafts/ERJ/CDU_1/Key_I"},
        {FMCKey::KEYJ, "XCrafts/ERJ/CDU_1/Key_J"},
        {FMCKey::KEYK, "XCrafts/ERJ/CDU_1/Key_K"},
        {FMCKey::KEYL, "XCrafts/ERJ/CDU_1/Key_L"},
        {FMCKey::KEYM, "XCrafts/ERJ/CDU_1/Key_M"},
        {FMCKey::KEYN, "XCrafts/ERJ/CDU_1/Key_N"},
        {FMCKey::KEYO, "XCrafts/ERJ/CDU_1/Key_O"},
        {FMCKey::KEYP, "XCrafts/ERJ/CDU_1/Key_P"},
        {FMCKey::KEYQ, "XCrafts/ERJ/CDU_1/Key_Q"},
        {FMCKey::KEYR, "XCrafts/ERJ/CDU_1/Key_R"},
        {FMCKey::KEYS, "XCrafts/ERJ/CDU_1/Key_S"},
        {FMCKey::KEYT, "XCrafts/ERJ/CDU_1/Key_T"},
        {FMCKey::KEYU, "XCrafts/ERJ/CDU_1/Key_U"},
        {FMCKey::KEYV, "XCrafts/ERJ/CDU_1/Key_V"},
        {FMCKey::KEYW, "XCrafts/ERJ/CDU_1/Key_W"},
        {FMCKey::KEYX, "XCrafts/ERJ/CDU_1/Key_X"},
        {FMCKey::KEYY, "XCrafts/ERJ/CDU_1/Key_Y"},
        {FMCKey::KEYZ, "XCrafts/ERJ/CDU_1/Key_Z"},
        
        // Brightness Controls
        {FMCKey::BRIGHTNESS_UP, "XCrafts/ERJ/CDU_1/Key_BRT"},
        {FMCKey::BRIGHTNESS_DOWN, "XCrafts/ERJ/CDU_1/Key_DIM"},
        
        // Special Keys
        {FMCKey::SPACE, "XCrafts/ERJ/CDU_1/Key_Space"},
        {FMCKey::OVERFLY_OR_DEL, "XCrafts/ERJ/CDU_1/Key_DEL"},
        {FMCKey::SLASH, "XCrafts/ERJ/CDU_1/Key_Slash_Command"},
        {FMCKey::CLR, "XCrafts/ERJ/CDU_1/Key_CLR"}
    };
    
    return buttons;
}

const std::map<char, FMCTextColor>& XCraftsFMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colors = {
        {0x00, COLOR_WHITE},
        {0x01, COLOR_CYAN},
        {0x02, COLOR_GREEN},
        {0x03, COLOR_YELLOW},
        {0x04, COLOR_MAGENTA},
        {0x05, COLOR_RED},
        {0x06, COLOR_GREY},
        {0x07, COLOR_AMBER},
        {0x08, COLOR_WHITE_BG},
        {0x09, COLOR_WHITE_BG},
        {0x0A, COLOR_WHITE_BG},
        {0x0b, COLOR_WHITE_BG},
        {0x0c, COLOR_WHITE_BG},
        {0x0d, COLOR_WHITE_BG},
        {0x0e, COLOR_WHITE_BG},
        {0x0f, COLOR_WHITE_BG},
        {0x10, COLOR_WHITE_BG},
        {0x11, COLOR_WHITE_BG},
        {0x12, COLOR_WHITE_BG},
    };
    return colors;
}

void XCraftsFMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {            
        case 'b':
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_DOWN.begin(), FMCSpecialCharacter::ARROW_DOWN.end()); // Should be arrow left and right
            break;
            
        case 'd':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;
            
        case 'v':
            buffer->insert(buffer->end(), FMCSpecialCharacter::FILLED_ARROW_RIGHT.begin(), FMCSpecialCharacter::FILLED_ARROW_RIGHT.end());
            break;
            
        case '?':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;
            
        case '$':
            buffer->insert(buffer->end(), FMCSpecialCharacter::FILLED_ARROW_LEFT.begin(), FMCSpecialCharacter::FILLED_ARROW_LEFT.end());
            break;

        case '@':
            buffer->insert(buffer->end(), FMCSpecialCharacter::TRIANGLE.begin(), FMCSpecialCharacter::TRIANGLE.end()); // Should be the rotating arrows
            break;
            
        case '\\':
            buffer->insert(buffer->end(), FMCSpecialCharacter::TRIANGLE.begin(), FMCSpecialCharacter::TRIANGLE.end()); // Should be the a dot / square
            break;
        
        default:
            buffer->push_back(character);
            break;
    }
}

void XCraftsFMCProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    
    // Get the number of data fields to display
    int dataCount = datarefManager->getCached<int>("XCrafts/FMS/data_count1");
    
    // Process each display field based on data count
    for (int i = 1; i <= std::min(dataCount, 70); i++) {
        char datarefName[32];
        snprintf(datarefName, sizeof(datarefName), "XCrafts/FMS/CDU_1_%02d", i);
        
        std::vector<unsigned char> text = datarefManager->getCached<std::vector<unsigned char>>(datarefName);
        
        if (text.empty() || text.size() < 6) {
            continue;
        }
        
        // Parse XCrafts format: RRCCFS[TEXT]
        // RR CC F S
        // 13 15 6 010 DEPARTUREv
        // 13 17 2 0 TAKEOFFv
        // 13 01 2 0 $CLEAR            APPLYv
        // 11 01 1 8 $TCAS/XPDR
        // RR = Row (01-99), CC = Column (01-99), F = Font (1-6), S = Color (0-8)
        
        // Extract row number
        int row = 0;
        if (text.size() >= 2 && text[0] >= '0' && text[0] <= '9' && text[1] >= '0' && text[1] <= '9') {
            row = (text[0] - '0') * 10 + (text[1] - '0');
        }
        
        // Extract column number
        int col = 0;
        if (text.size() >= 4 && text[2] >= '0' && text[2] <= '9' && text[3] >= '0' && text[3] <= '9') {
            col = (text[2] - '0') * 10 + (text[3] - '0');
        }
        
        int fontSize = 1;
        if (text.size() >= 5) {
            fontSize = text[4] - '0';
        }
        
        // Extract color code (variable length) and find text start
        unsigned char colorCode = 0;
        int textStartIndex = 5; // Default if no color found
        
        // Parse color starting from position 5, continue until we hit a non-digit or space
        if (text.size() > 5) {
            int colorValue = 0;
            int i = 5;
            while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
                colorValue = colorValue * 10 + (text[i] - '0');
                i++;
            }
            colorCode = colorValue;
            
            // Skip any space after color code
            while (i < text.size() && text[i] == ' ') {
                i++;
            }
            textStartIndex = i;
        }
        
        // Convert to 0-based indexing and validate bounds
        int lineIndex = row - 1;
        int colIndex = col - 1;
        
        if (lineIndex < 0 || lineIndex >= ProductFMC::PageLines || colIndex < 0) {
            continue;
        }
        
        if (text.size() > textStartIndex) {
            for (int j = textStartIndex; j < text.size() && (colIndex + (j - textStartIndex)) < ProductFMC::PageCharsPerLine; j++) {
                unsigned char c = text[j];
                if (c == 0x00) {
                    break;
                }
                
                int displayCol = colIndex + (j - textStartIndex);
                bool isSmallFont = fontSize > 1;
                product->writeLineToPage(page, lineIndex, displayCol, std::string(1, (char)c), colorCode, isSmallFont);
            }
        }
    }
    
    std::vector<unsigned char> scratchpadText = datarefManager->getCached<std::vector<unsigned char>>("XCrafts/FMS/CDU_1_ScratchPad");
    if (!scratchpadText.empty()) {
        for (int i = 0; i < scratchpadText.size() && i < ProductFMC::PageCharsPerLine; ++i) {
            unsigned char c = scratchpadText[i];
            if (c == 0x00 || c == '|') {
                break;
            }
            product->writeLineToPage(page, 13, i, std::string(1, (char)c), 0, false);
        }
    }
}

void XCraftsFMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (std::fabs(button->value) > DBL_EPSILON) {
        // Handle buttons with values (like brightness controls)
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
            if (index >= 0 && index < (int)vec.size()) {
                vec[index] = std::clamp(vec[index] + button->value, 0.0, 1.0);
                Dataref::getInstance()->set<std::vector<float>>(baseRef.c_str(), vec);
            }
        } else {
            Dataref::getInstance()->set<float>(ref.c_str(), button->value);
        }
    } else {
        // Handle command buttons
        Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
    }
}
