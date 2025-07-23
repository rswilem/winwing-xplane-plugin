#include "ff777-pfp-profile.h"
#include "product-mcdu.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>

constexpr unsigned int PAGE_LINES = 14;
constexpr unsigned int PAGE_CHARS_PER_LINE = 24;
constexpr unsigned int PAGE_BYTES_PER_CHAR = 3;

// Static member definitions
std::vector<std::string> FlightFactor777PfpProfile::datarefsList = {
    "1-sim/cduL/display/symbols",      // 336 letters (14 lines × 24 chars)
    "1-sim/cduL/display/symbolsColor", // 336 numbers, 1 is white, 5 is green. Rest unknown
    "1-sim/cduL/display/symbolsEffects", // 336 numbers, unknown, all zero?
    "1-sim/cduL/display/symbolsSize"   // 336 numbers, it seems that 1 is normal, 2 is small.
};

std::vector<PFPButtonDef> FlightFactor777PfpProfile::buttonsList = {
    // FlightFactor777 737 FMC LSK/RSK buttons
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
    {14, "CLB", ""},
    {15, "CRZ", ""},
    {16, "DES", ""},
    {17, "UNUSED1", ""},
    {18, "BRT", ""},
    {19, "MENU", ""},
    {20, "LEGS", ""},
    {21, "DEPARR", ""},
    {22, "HOLD", ""},
    {23, "PROG", ""},
    {24, "EXEC", ""},
    {25, "DIM", ""},
    {26, "N1LIMIT", ""},
    {27, "FIX", ""},
    {28, "PREV_PAGE", ""},
    {29, "NEXT_PAGE", ""},
    {30, "UNUSED2", ""},
    {31, "UNUSED3", ""},
    {32, "KEY1", ""},
    {33, "KEY2", ""},
    {34, "KEY3", ""},
    {35, "KEY4", ""},
    {36, "KEY5", ""},
    {37, "KEY6", ""},
    {38, "KEY7", ""},
    {39, "KEY8", ""},
    {40, "KEY9", ""},
    {41, "PERIOD", ""},
    {42, "KEY0", ""},
    {43, "PLUSMINUS", ""},
    {44, "KEYA", ""},
    {45, "KEYB", ""},
    {46, "KEYC", ""},
    {47, "KEYD", ""},
    {48, "KEYE", ""},
    {49, "KEYF", ""},
    {50, "KEYG", ""},
    {51, "KEYH", ""},
    {52, "KEYI", ""},
    {53, "KEYJ", ""},
    {54, "KEYK", ""},
    {55, "KEYL", ""},
    {56, "KEYM", ""},
    {57, "KEYN", ""},
    {58, "KEYO", ""},
    {59, "KEYP", ""},
    {60, "KEYQ", ""},
    {61, "KEYR", ""},
    {62, "KEYS", ""},
    {63, "KEYT", ""},
    {64, "KEYU", ""},
    {65, "KEYV", ""},
    {66, "KEYW", ""},
    {67, "KEYX", ""},
    {68, "KEYY", ""},
    {69, "KEYZ", ""},
    {70, "SPACE", ""},
    {71, "DEL", ""},
    {72, "SLASH", ""},
    {73, "CLR", ""},
};

std::map<char, int> FlightFactor777PfpProfile::colors = {
    // FlightFactor 777 FMC colors mapped to PFP display values
    {'W', 0x0042},  // White text
    {'G', 0x0084},  // Green text
    {'C', 0x0063},  // Cyan (blue) text
    {'M', 0x00A5},  // Magenta text
    {'Y', 0x00E6},  // Yellow text (if used)
    {'R', 0x00FF},  // Red text (if used)
    
    // Fallback
    {' ', 0x0042},  // Space = white
};

FlightFactor777PfpProfile::FlightFactor777PfpProfile() {
    // Initialize LED brightness monitoring for FlightFactor 777
    // Check for common brightness datarefs or use default values
    if (Dataref::getInstance()->exists("sim/cockpit/electrical/instrument_brightness")) {
        const PFPLed ledsToSet[] = {
            PFPLed::CALL,
            PFPLed::FAIL,
            PFPLed::MSG,
            PFPLed::OFST,
        };

        // Turn off status LEDs initially
        for (auto led : ledsToSet) {
            if (ledBrightnessCallback) {
                ledBrightnessCallback(led, 0);
            }
        }
        
        // Monitor general instrument brightness
        Dataref::getInstance()->monitorExistingDataref<float>("sim/cockpit/electrical/instrument_brightness", [this](float brightness) {
            if (!ledBrightnessCallback) {
                return;
            }
            
            uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness * 255.0f : 0;
            ledBrightnessCallback(PFPLed::BACKLIGHT, target);
            ledBrightnessCallback(PFPLed::SCREEN_BACKLIGHT, target);
        });
        
        // Monitor avionics power
        Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
            Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/electrical/instrument_brightness");
        });
    } else {
        // Fallback: Set default brightness values
        if (ledBrightnessCallback) {
            ledBrightnessCallback(PFPLed::BACKLIGHT, 128);
            ledBrightnessCallback(PFPLed::SCREEN_BACKLIGHT, 128);
            
            const PFPLed ledsToSet[] = {
                PFPLed::CALL,
                PFPLed::FAIL,
                PFPLed::MSG,
                PFPLed::OFST,
            };
            
            for (auto led : ledsToSet) {
                ledBrightnessCallback(led, 0);
            }
        }
    }
}

FlightFactor777PfpProfile::~FlightFactor777PfpProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/instrument_brightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");    
}

bool FlightFactor777PfpProfile::IsEligible() {
    return Dataref::getInstance()->exists("1-sim/cduL/display/symbols");
}

const std::vector<std::string>& FlightFactor777PfpProfile::displayDatarefs() const {
    return datarefsList;
}

const std::vector<PFPButtonDef>& FlightFactor777PfpProfile::buttonDefs() const {
    return buttonsList;
}

const std::map<char, int>& FlightFactor777PfpProfile::colorMap() const {
    return colors;
}

void FlightFactor777PfpProfile::updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) {
    // Initialize page with spaces
    page = std::vector<std::vector<char>>(PAGE_LINES, std::vector<char>(PAGE_CHARS_PER_LINE * PAGE_BYTES_PER_CHAR, ' '));
    
    // Get the FlightFactor datarefs
    auto symbolsIt = cachedDatarefValues.find("1-sim/cduL/display/symbols");
    auto colorsIt = cachedDatarefValues.find("1-sim/cduL/display/symbolsColor");
    auto sizesIt = cachedDatarefValues.find("1-sim/cduL/display/symbolsSize");
    
    if (symbolsIt == cachedDatarefValues.end()) {
        return; // No symbols data available
    }
    
    const std::string& symbols = symbolsIt->second;
    const std::string& colors = (colorsIt != cachedDatarefValues.end()) ? colorsIt->second : "";
    const std::string& sizes = (sizesIt != cachedDatarefValues.end()) ? sizesIt->second : "";
    
    // FlightFactor format: 336 characters = 14 lines × 24 chars per line
    // Each character has corresponding color and size values
    int expectedLength = PAGE_LINES * PAGE_CHARS_PER_LINE; // 14 × 24 = 336
    
    for (int line = 0; line < PAGE_LINES && line * PAGE_CHARS_PER_LINE < symbols.length(); ++line) {
        for (int pos = 0; pos < PAGE_CHARS_PER_LINE; ++pos) {
            int index = line * PAGE_CHARS_PER_LINE + pos;
            
            if (index >= symbols.length()) {
                break;
            }
            
            char symbol = symbols[index];
            if (symbol == 0x00 || symbol == 0x20) { // Skip null and space characters
                continue;
            }
            
            // Determine color - default to white if no color data
            char color = 'W'; // Default white
            if (index < colors.length()) {
                int colorValue = static_cast<unsigned char>(colors[index]);
                switch (colorValue) {
                    case 1: color = 'W'; break; // White
                    case 5: color = 'G'; break; // Green
                    case 2: color = 'C'; break; // Cyan (guessing)
                    case 3: color = 'M'; break; // Magenta (guessing)
                    default: color = 'W'; break; // Default to white
                }
            }
            
            // Determine font size - default to normal if no size data
            bool fontSmall = false;
            if (index < sizes.length()) {
                int sizeValue = static_cast<unsigned char>(sizes[index]);
                fontSmall = (sizeValue == 2); // 2 = small, 1 = normal
            }
            
            // Write character to page
            writeLineToPage(page, line, pos, std::string(1, symbol), color, fontSmall);
        }
    }
}

void FlightFactor777PfpProfile::writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color, bool fontSmall) {
    if (line < 0 || line >= PAGE_LINES) {
        debug("Not writing line %i: Line number is out of range!\n", line);
        return;
    }
    if (pos < 0 || pos + text.length() > PAGE_CHARS_PER_LINE) {
        debug("Not writing line %i: Position number (%i) is out of range!\n", line, pos);
        return;
    }
    if (text.length() > PAGE_CHARS_PER_LINE) {
        debug("Not writing line %i: Text is too long (%lu) for line.\n", line, text.length());
        return;
    }

    pos = pos * PAGE_BYTES_PER_CHAR;
    for (size_t c = 0; c < text.length(); ++c) {
        page[line][pos + c * PAGE_BYTES_PER_CHAR] = color;
        page[line][pos + c * PAGE_BYTES_PER_CHAR + 1] = fontSmall;
        page[line][pos + c * PAGE_BYTES_PER_CHAR + PAGE_BYTES_PER_CHAR - 1] = text[c];
    }
}
