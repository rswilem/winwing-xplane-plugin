#include "rotatemd11-fmc-profile.h"

#include "appstate.h"
#include "config.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>
#include <cstring>

RotateMD11FMCProfile::RotateMD11FMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    
    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::FontMD11, product->identifierByte));

    Dataref::getInstance()->monitorExistingDataref<float>("Rotate/aircraft/controls/mcdu_1_brt", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<float>("Rotate/aircraft/controls/instr_panel_lts", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("Rotate/aircraft/controls/mcdu_1_brt");
        Dataref::getInstance()->executeChangedCallbacksForDataref("Rotate/aircraft/controls/instr_panel_lts");
    });
}

RotateMD11FMCProfile::~RotateMD11FMCProfile() {
    Dataref::getInstance()->unbind("Rotate/aircraft/controls/mcdu_1_brt");
    Dataref::getInstance()->unbind("Rotate/aircraft/controls/instr_panel_lts");
    Dataref::getInstance()->unbind("Rotate/aircraft/systems/elec_dc_batt_bus_pwrd");
}

bool RotateMD11FMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("Rotate/aircraft/controls/cdu_0/mcdu_line_0_content");
}

const std::vector<std::string> &RotateMD11FMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "Rotate/aircraft/controls/cdu_0/mcdu_line_0_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_0_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_1_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_1_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_2_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_2_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_3_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_3_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_4_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_4_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_5_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_5_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_6_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_6_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_7_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_7_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_8_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_8_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_9_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_9_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_10_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_10_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_11_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_11_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_12_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_12_style",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_13_content",
        "Rotate/aircraft/controls/cdu_0/mcdu_line_13_style"
    };

    return datarefs;
}

const std::vector<FMCButtonDef> &RotateMD11FMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        // Line Select Keys
        {FMCKey::LSK1L, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FL1"},
        {FMCKey::LSK2L, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FL2"},
        {FMCKey::LSK3L, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FL3"},
        {FMCKey::LSK4L, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FL4"},
        {FMCKey::LSK5L, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FL5"},
        {FMCKey::LSK6L, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FL6"},
        {FMCKey::LSK1R, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FR1"},
        {FMCKey::LSK2R, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FR2"},
        {FMCKey::LSK3R, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FR3"},
        {FMCKey::LSK4R, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FR4"},
        {FMCKey::LSK5R, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FR5"},
        {FMCKey::LSK6R, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FR6"},
        
        // Function Keys
        {std::vector<FMCKey>{FMCKey::MCDU_DIR, FMCKey::PFP_LEGS}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_DIR"},
        {FMCKey::PROG, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_PROG"},
        {std::vector<FMCKey>{FMCKey::MCDU_PERF, FMCKey::PFP3_N1_LIMIT}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_PERF"},
        {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP_INIT_REF}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_INIT"},
        {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP_HOLD}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_REF"},
        {std::vector<FMCKey>{FMCKey::MCDU_FPLN, FMCKey::PFP_ROUTE}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FPLN"},
        {std::vector<FMCKey>{FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD, FMCKey::PFP7_NAV_RAD}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_NAVRAD"},
        {std::vector<FMCKey>{FMCKey::MCDU_SEC_FPLN}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_SECFPLN"},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_TOAPPR"},
        {FMCKey::MENU, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_MENU"},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_FIX"},
        {FMCKey::MCDU_EMPTY_TOP_RIGHT, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_ENGOUT"},
        
        // Navigation Keys
        {std::vector<FMCKey>{FMCKey::MCDU_PAGE_UP, FMCKey::PAGE_PREV}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_UP"},
        {std::vector<FMCKey>{FMCKey::MCDU_PAGE_DOWN, FMCKey::PAGE_NEXT}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_DOWN"},
        {FMCKey::PFP_EXEC, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_PAGE"},

                // Brightness Controls
        {FMCKey::BRIGHTNESS_UP, "Rotate/aircraft/controls_c/mcdu_1_brt_up"},
        {FMCKey::BRIGHTNESS_DOWN, "Rotate/aircraft/controls_c/mcdu_1_brt_dn"},
        
        // Numeric Keys
        {FMCKey::KEY1, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_1"},
        {FMCKey::KEY2, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_2"},
        {FMCKey::KEY3, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_3"},
        {FMCKey::KEY4, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_4"},
        {FMCKey::KEY5, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_5"},
        {FMCKey::KEY6, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_6"},
        {FMCKey::KEY7, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_7"},
        {FMCKey::KEY8, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_8"},
        {FMCKey::KEY9, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_9"},
        {FMCKey::KEY0, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_0"},
        {FMCKey::PERIOD, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_POINT"},
        {FMCKey::PLUSMINUS, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_PLUS"},
        {FMCKey::PFP_DEL, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_MINUS"},
        
        // Alpha Keys
        {FMCKey::KEYA, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_A"},
        {FMCKey::KEYB, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_B"},
        {FMCKey::KEYC, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_C"},
        {FMCKey::KEYD, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_D"},
        {FMCKey::KEYE, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_E"},
        {FMCKey::KEYF, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_F"},
        {FMCKey::KEYG, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_G"},
        {FMCKey::KEYH, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_H"},
        {FMCKey::KEYI, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_I"},
        {FMCKey::KEYJ, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_J"},
        {FMCKey::KEYK, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_K"},
        {FMCKey::KEYL, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_L"},
        {FMCKey::KEYM, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_M"},
        {FMCKey::KEYN, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_N"},
        {FMCKey::KEYO, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_O"},
        {FMCKey::KEYP, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_P"},
        {FMCKey::KEYQ, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_Q"},
        {FMCKey::KEYR, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_R"},
        {FMCKey::KEYS, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_S"},
        {FMCKey::KEYT, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_T"},
        {FMCKey::KEYU, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_U"},
        {FMCKey::KEYV, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_V"},
        {FMCKey::KEYW, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_W"},
        {FMCKey::KEYX, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_X"},
        {FMCKey::KEYY, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_Y"},
        {FMCKey::KEYZ, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_Z"},
        
        // Special Keys
        {FMCKey::SPACE, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_SPC"},
        {FMCKey::SLASH, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_BAR"},
        {FMCKey::CLR, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_CLR"},
        {FMCKey::MCDU_OVERFLY, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_PLUS"}
    };

    return buttons;
}

const std::map<char, FMCTextColor> &RotateMD11FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        // Numeric style codes from datarefs
        {1, FMCTextColor::COLOR_GREEN},
        {2, FMCTextColor::COLOR_GREEN},
        {4, FMCTextColor::COLOR_GREEN},
        {5, FMCTextColor::COLOR_GREEN},
        // Character codes for compatibility
        {'w', FMCTextColor::COLOR_GREEN},
        {'W', FMCTextColor::COLOR_GREEN},
        {'g', FMCTextColor::COLOR_GREEN},
        {'G', FMCTextColor::COLOR_GREEN},
        {'e', FMCTextColor::COLOR_GREEN},
        {'E', FMCTextColor::COLOR_GREEN},
    };

    return colMap;
}

std::string RotateMD11FMCProfile::processUTF8Arrows(const std::string &input) {
    // Replace UTF-8 arrow sequences with single-byte ASCII codes
    std::string output;
    
    for (size_t i = 0; i < input.length(); ) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        
        // Check for UTF-8 arrow sequences (0xE2 0x86 0x90-93)
        if (c == 0xE2 && i + 2 < input.length()) {
            unsigned char byte2 = static_cast<unsigned char>(input[i + 1]);
            unsigned char byte3 = static_cast<unsigned char>(input[i + 2]);
            
            if (byte2 == 0x86) {
                // Arrow characters
                if (byte3 == 0x90) {
                    output += static_cast<char>(28);  // Left arrow
                    i += 3;
                } else if (byte3 == 0x92) {
                    output += static_cast<char>(29);  // Right arrow
                    i += 3;
                } else if (byte3 == 0x91) {
                    output += static_cast<char>(30);  // Up arrow
                    i += 3;
                } else if (byte3 == 0x93) {
                    output += static_cast<char>(31);  // Down arrow
                    i += 3;
                } else {
                    i += 3;  // Unknown sequence, skip
                }
            } else {
                i += 3;  // Unknown sequence, skip
            }
        } else if (c >= 0x80) {
            // Other UTF-8 multi-byte sequence - skip
            if ((c & 0xE0) == 0xC0) {
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                i += 3;
            } else if ((c & 0xF8) == 0xF0) {
                i += 4;
            } else {
                i++;
            }
        } else {
            // Regular ASCII character
            output += c;
            i++;
        }
    }
    
    return output;
}

std::vector<int> RotateMD11FMCProfile::buildStylePositionMap(const std::string &content, const std::string &style) {
    // Build mapping: content position -> style index
    // Rules: 
    // - Last word (after 2+ spaces): maps to end of style array, skips 2+ space sequences
    // - Beginning content: maps to beginning of style array, skips 2+ space sequences
    
    std::vector<int> positionMap(content.length(), -1);
    
    // Find last word boundaries
    int lastWordEnd = content.length();
    int trailingSpaces = 0;
    
    for (int i = content.length() - 1; i >= 0; i--) {
        if (content[i] == ' ' || content[i] == 0x00) {
            trailingSpaces++;
        } else {
            lastWordEnd = (trailingSpaces == 1) ? i + 2 : i + 1;
            break;
        }
    }
    
    int lastWordStart = lastWordEnd;
    for (int i = lastWordEnd - 1; i >= 0; i--) {
        // Look for 2+ consecutive spaces to find last word boundary
        if (i >= 1 && content[i] == ' ' && content[i-1] == ' ') {
            lastWordStart = i + 1;
            break;
        }
        if (i == 0) {
            lastWordStart = 0;
            break;
        }
    }
    
    // Count style values needed for last word (skip 2+ space sequences)
    int lastWordStyleCount = 0;
    for (int i = lastWordStart; i < lastWordEnd; ) {
        if (content[i] == ' ') {
            int spaceCount = 0;
            while (i < lastWordEnd && content[i] == ' ') {
                spaceCount++;
                i++;
            }
            if (spaceCount < 2) {
                lastWordStyleCount += spaceCount;
            }
        } else {
            lastWordStyleCount++;
            i++;
        }
    }
    
    // Map last word to end of style array
    if (lastWordStyleCount > 0 && lastWordStyleCount <= style.size()) {
        int styleIdx = style.size() - lastWordStyleCount;
        
        for (int pos = lastWordStart; pos < lastWordEnd; ++pos) {
            if (content[pos] == ' ') {
                // Count consecutive spaces at this position
                int spaceCount = 1;
                for (int j = pos + 1; j < lastWordEnd && content[j] == ' '; j++) spaceCount++;
                for (int j = pos - 1; j >= lastWordStart && content[j] == ' '; j--) spaceCount++;
                
                if (spaceCount < 2) {
                    positionMap[pos] = styleIdx++;
                }
            } else {
                positionMap[pos] = styleIdx++;
            }
        }
    }
    
    // Mark 2+ space sequences in beginning content
    for (int i = 0; i < lastWordStart; ) {
        if (content[i] == ' ') {
            int spaceCount = 0;
            int spaceStart = i;
            while (i < lastWordStart && content[i] == ' ') {
                spaceCount++;
                i++;
            }
            if (spaceCount < 2) {
                // Single space is encoded, will be mapped below
            } else {
                // Mark 2+ spaces as skipped
                for (int j = spaceStart; j < spaceStart + spaceCount; j++) {
                    if (positionMap[j] < 0) {
                        positionMap[j] = -2;  // -2 means explicitly skipped
                    }
                }
            }
        } else {
            i++;
        }
    }
    
    // Map beginning content to beginning of style array
    int styleIdx = 0;
    for (int pos = 0; pos < lastWordStart; ++pos) {
        if (positionMap[pos] == -1) {  // Not yet mapped
            positionMap[pos] = styleIdx++;
        } else if (positionMap[pos] == -2) {
            positionMap[pos] = -1;  // Restore to "no mapping"
        }
    }
    
    return positionMap;
}

void RotateMD11FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '$':
            // Outlined square character
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;
        case '`':
            // Degrees symbol
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;
        
        case 28:  // Left arrow placeholder (will be replaced from UTF-8)
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_LEFT.begin(), FMCSpecialCharacter::ARROW_LEFT.end());
            break;
        
        case 29:  // Right arrow placeholder (will be replaced from UTF-8)
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_RIGHT.begin(), FMCSpecialCharacter::ARROW_RIGHT.end());
            break;
        
        case 30:  // Up arrow (ASCII 30 = 0x1E)
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_UP.begin(), FMCSpecialCharacter::ARROW_UP.end());
            break;
        
        case 31:  // Down arrow (ASCII 31 = 0x1F)
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_DOWN.begin(), FMCSpecialCharacter::ARROW_DOWN.end());
            break;

        default:
            buffer->push_back(character);
            break;
    }
}

void RotateMD11FMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    
    for (int line = 0; line < ProductFMC::PageLines; ++line) {
        std::string contentRef = "Rotate/aircraft/controls/cdu_0/mcdu_line_" + std::to_string(line) + "_content";
        std::string styleRef = "Rotate/aircraft/controls/cdu_0/mcdu_line_" + std::to_string(line) + "_style";
        
        std::string contentStr = datarefManager->getCached<std::string>(contentRef.c_str());
        if (contentStr.empty()) {
            continue;
        }
        
        // Convert UTF-8 arrows to ASCII codes
        std::string processedContent = processUTF8Arrows(contentStr);
        
        // Read style and build position mapping
        std::string styleStr = datarefManager->getCached<std::string>(styleRef.c_str());
        std::vector<int> positionToStyleIndex = buildStylePositionMap(processedContent, styleStr);
        
        // Render all characters with their mapped styles
        for (int pos = 0; pos < ProductFMC::PageCharsPerLine && pos < processedContent.length(); ++pos) {
            unsigned char c = static_cast<unsigned char>(processedContent[pos]);
            
            if (c == 0x00) {
                continue;
            }
            
            // Get style code from mapping
            int styleCode = 4;  // Default small font
            if (positionToStyleIndex[pos] >= 0 && positionToStyleIndex[pos] < styleStr.size()) {
                styleCode = (unsigned char)styleStr[positionToStyleIndex[pos]];
            }
            
            bool fontSmall = (styleCode == 4);
            
            product->writeLineToPage(page, line, pos, std::string(1, c), 'g', fontSmall);
        }
    }
}

void RotateMD11FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }
    
    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
}