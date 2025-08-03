#include "ff777-pfp-profile.h"
#include "product-pfp.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>

FlightFactor777PfpProfile::FlightFactor777PfpProfile(ProductPFP *product) : PfpAircraftProfile(product) {
    const PFPLed ledsToSet[] = {
        PFPLed::CALL,
        PFPLed::FAIL,
        PFPLed::MSG,
        PFPLed::OFST,
        PFPLed::EXEC
    };
    
    for (auto led : ledsToSet) {
        product->setLedBrightness(led, 0);
    }
    
    Dataref::getInstance()->monitorExistingDataref<float>("sim/cockpit/electrical/instrument_brightness", [product](float brightness) {
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
        product->setLedBrightness(PFPLed::BACKLIGHT, target);
        product->setLedBrightness(PFPLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/electrical/instrument_brightness");
    });
}

FlightFactor777PfpProfile::~FlightFactor777PfpProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");    
}

bool FlightFactor777PfpProfile::IsEligible() {
    return Dataref::getInstance()->exists("1-sim/cduL/display/symbols");
}

const std::vector<std::string>& FlightFactor777PfpProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "1-sim/cduL/display/symbols", // 336 letters (14 lines x 24 chars)
        "1-sim/cduL/display/symbolsColor", // 336 numbers, 1 is white, 5 is green. Rest unknown
        "1-sim/cduL/display/symbolsEffects", // 336 numbers, unknown, all zero?
        "1-sim/cduL/display/symbolsSize" // 336 numbers, it seems that 1 is normal, 2 is small.
    };
    
    return datarefs;
}

const std::vector<PFPButtonDef>& FlightFactor777PfpProfile::buttonDefs() const {
    static const std::vector<PFPButtonDef> tripleSevenButtonLayout = {
        {0, "LSK1L", ""},
        {1, "LSK2L", ""},
        {2, "LSK3L", ""},
        {3, "LSK4L", ""},
        {4, "LSK5L", ""},
        {5, "LSK6L", ""},
        {6, "LSK1R", ""},
        {7, "LSK2R", ""},
        {8, "LSK3R", ""},
        {9, "LSK4R", ""},
        {10, "LSK5R", ""},
        {11, "LSK6R", ""},
        {12, "INITREF", ""},
        {13, "RTE", ""},
        {14, "DEP/ARR", ""},
        {15, "ALTN", ""},
        {16, "VNAV", ""},
        {17, "BRT-", ""},
        {18, "BRT+", ""},
        {19, "FIX", ""},
        {20, "LEGS", ""},
        {21, "HOLD", ""},
        {22, "FMC/COMM", ""},
        {23, "PROG", ""},
        {24, "EXEC", ""},
        {25, "MENU", ""},
        {26, "NAV/RAD", ""},
        {27, "PREV_PAGE", ""},
        {28, "NEXT_PAGE", ""},
        {29, "KEY1", ""},
        {30, "KEY2", ""},
        {31, "KEY3", ""},
        {32, "KEY4", ""},
        {33, "KEY5", ""},
        {34, "KEY6", ""},
        {35, "KEY7", ""},
        {36, "KEY8", ""},
        {37, "KEY9", ""},
        {38, "PERIOD", ""},
        {39, "KEY0", ""},
        {40, "PLUSMINUS", ""},
        {41, "KEYA", ""},
        {42, "KEYB", ""},
        {43, "KEYC", ""},
        {44, "KEYD", ""},
        {45, "KEYE", ""},
        {46, "KEYF", ""},
        {47, "KEYG", ""},
        {48, "KEYH", ""},
        {49, "KEYI", ""},
        {50, "KEYJ", ""},
        {51, "KEYK", ""},
        {52, "KEYL", ""},
        {53, "KEYM", ""},
        {54, "KEYN", ""},
        {55, "KEYO", ""},
        {56, "KEYP", ""},
        {57, "KEYQ", ""},
        {58, "KEYR", ""},
        {59, "KEYS", ""},
        {60, "KEYT", ""},
        {61, "KEYU", ""},
        {62, "KEYV", ""},
        {63, "KEYW", ""},
        {64, "KEYX", ""},
        {65, "KEYY", ""},
        {66, "KEYZ", ""},
        {67, "SPACE", ""},
        {68, "DEL", ""},
        {69, "SLASH", ""},
        {70, "CLR", ""}
    };
    
    return tripleSevenButtonLayout;
}

const std::map<char, int>& FlightFactor777PfpProfile::colorMap() const {
    static const std::map<char, int> colMap = {
        {'W', 0x0042}, // White text
        {'G', 0x0084}, // Green text
        {'C', 0x0063}, // Cyan (blue) text
        {'M', 0x00A5}, // Magenta text
        {'Y', 0x00E6}, // Yellow text (if used)
        {'R', 0x00FF}, // Red text (if used)
    };
    
    return colMap;
}

void FlightFactor777PfpProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductPFP::PageLines, std::vector<char>(ProductPFP::PageCharsPerLine * ProductPFP::PageBytesPerChar, ' '));
    
    std::string symbols = Dataref::getInstance()->getCached<std::string>("1-sim/cduL/display/symbols");
    std::string colors = Dataref::getInstance()->getCached<std::string>("1-sim/cduL/display/symbolsColor");
    std::string sizes = Dataref::getInstance()->getCached<std::string>("1-sim/cduL/display/symbolsSize");
    
    for (int line = 0; line < ProductPFP::PageLines && line * ProductPFP::PageCharsPerLine < symbols.length(); ++line) {
        for (int pos = 0; pos < ProductPFP::PageCharsPerLine; ++pos) {
            int index = line * ProductPFP::PageCharsPerLine + pos;
            
            if (index >= symbols.length()) {
                break;
            }
            
            char symbol = symbols[index];
            if (symbol == 0x00 || symbol == 0x20) {
                continue;
            }
            
            char color = 'W'; // Default white
            if (index < colors.length()) {
                int colorValue = static_cast<unsigned char>(colors[index]);
                switch (colorValue) {
                    case 1: color = 'W'; break; // White
                    case 5: color = 'G'; break; // Green
                    case 2: color = 'C'; break; // Cyan
                    case 3: color = 'M'; break; // Magenta
                    default: color = 'W'; break; // Default to white
                }
            }
            
            bool fontSmall = false;
            if (index < sizes.length()) {
                int sizeValue = static_cast<unsigned char>(sizes[index]);
                fontSmall = (sizeValue == 2);
            }
            
            product->writeLineToPage(page, line, pos, std::string(1, symbol), color, fontSmall);
        }
    }
}

void FlightFactor777PfpProfile::buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) {
    Dataref::getInstance()->executeCommand(button->dataref.c_str(), phase);
}
