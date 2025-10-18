#include "rotatemd11-fmc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>
#include <cstring>

RotateMD11FMCProfile::RotateMD11FMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::Font737, product->identifierByte));

    // Monitor brightness control - using generic sim dataref as fallback
    Dataref::getInstance()->monitorExistingDataref<float>("sim/cockpit/electrical/instrument_brightness", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/electrical/instrument_brightness");
    });
}

RotateMD11FMCProfile::~RotateMD11FMCProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
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
        {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP_INIT_REF}, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_REF"},
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
        {FMCKey::PAGE_PREV, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_PAGE"},
        
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
        {FMCKey::PLUSMINUS, "Rotate/aircraft/controls_c/cdu_0/mcdu_key_MINUS"},
        
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
        {1, FMCTextColor::COLOR_GREEN},
        {2, FMCTextColor::COLOR_GREEN},
        {4, FMCTextColor::COLOR_AMBER},
        {5, FMCTextColor::COLOR_GREEN},
        {'w', FMCTextColor::COLOR_WHITE},
        {'W', FMCTextColor::COLOR_WHITE},
        {'g', FMCTextColor::COLOR_GREEN},
        {'G', FMCTextColor::COLOR_GREEN},
        {'e', FMCTextColor::COLOR_AMBER},
        {'E', FMCTextColor::COLOR_AMBER},
    };

    return colMap;
}

void RotateMD11FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '$':
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

void RotateMD11FMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    
    // Read all 14 lines (0-13)
    for (int line = 0; line < ProductFMC::PageLines; ++line) {
        std::string contentRef = "Rotate/aircraft/controls/cdu_0/mcdu_line_" + std::to_string(line) + "_content";
        std::string styleRef = "Rotate/aircraft/controls/cdu_0/mcdu_line_" + std::to_string(line) + "_style";
        
        std::string content = datarefManager->getCached<std::string>(contentRef.c_str());
        std::vector<int> style = datarefManager->getCached<std::vector<int>>(styleRef.c_str());
        
        if (content.empty()) {
            continue;
        }
        
        // Process each character in the line
        for (int pos = 0; pos < ProductFMC::PageCharsPerLine && pos < content.size(); ++pos) {
            char c = content[pos];
            
            if (c == 0x00 || c == 0x20) {
                continue; // Skip null and space characters
            }
            
            // Get color from style array, default to green (1) if not available
            int colorCode = (style.size() > pos) ? style[pos] : 1;
            
            // Map color code to character for colorMap lookup
            char color = static_cast<char>(colorCode);
            
            product->writeLineToPage(page, line, pos, std::string(1, c), color, false);
        }
    }
}

void RotateMD11FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }
    
    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
}

