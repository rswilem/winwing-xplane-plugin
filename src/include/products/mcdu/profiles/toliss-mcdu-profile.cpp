#include "toliss-mcdu-profile.h"
#include "product-mcdu.h"
#include "dataref.h"
#include <cstring>
#include <algorithm>

constexpr unsigned int PAGE_LINES = 14;
constexpr unsigned int PAGE_CHARS_PER_LINE = 24;
constexpr unsigned int PAGE_BYTES_PER_CHAR = 3;
constexpr unsigned int PAGE_BYTES_PER_LINE = PAGE_CHARS_PER_LINE * PAGE_BYTES_PER_CHAR;

TolissMcduProfile::TolissMcduProfile() {
    datarefRegex = std::regex("AirbusFBW/MCDU(1|2)([s]{0,1})([a-zA-Z]+)([0-6]{0,1})([L]{0,1})([a-z]{1})");
    
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [this](float brightness) {
        if (!ledBrightnessCallback) {
            return;
        }
        
        uint8_t target = brightness * 255.0f;
        ledBrightnessCallback(MCDULed::BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("AirbusFBW/DUBrightness", [this](std::vector<float> brightness) {
        if (!ledBrightnessCallback || brightness.size() < 8) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[6] * 255.0f : 0;
        ledBrightnessCallback(MCDULed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/DUBrightness");
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    });
}

TolissMcduProfile::~TolissMcduProfile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("AirbusFBW/DUBrightness");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool TolissMcduProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::vector<std::string>& TolissMcduProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        //"AirbusFBW/MCDU1titleb",
        "AirbusFBW/MCDU1titleg",
        "AirbusFBW/MCDU1titles",
        "AirbusFBW/MCDU1titlew",
        //"AirbusFBW/MCDU1titley",
        "AirbusFBW/MCDU1stitley",
        "AirbusFBW/MCDU1stitlew",
        "AirbusFBW/MCDU1label1w",
        "AirbusFBW/MCDU1label2w",
        "AirbusFBW/MCDU1label3w",
        "AirbusFBW/MCDU1label4w",
        "AirbusFBW/MCDU1label5w",
        "AirbusFBW/MCDU1label6w",
        //"AirbusFBW/MCDU1label1a",
        "AirbusFBW/MCDU1label2a",
        "AirbusFBW/MCDU1label3a",
        //"AirbusFBW/MCDU1label4a",
        //"AirbusFBW/MCDU1label5a",
        "AirbusFBW/MCDU1label6a",
        "AirbusFBW/MCDU1label1g",
        "AirbusFBW/MCDU1label2g",
        "AirbusFBW/MCDU1label3g",
        "AirbusFBW/MCDU1label4g",
        "AirbusFBW/MCDU1label5g",
        //"AirbusFBW/MCDU1label6g",
        "AirbusFBW/MCDU1label1b",
        //"AirbusFBW/MCDU1label2b",
        "AirbusFBW/MCDU1label3b",
        "AirbusFBW/MCDU1label4b",
        "AirbusFBW/MCDU1label5b",
        "AirbusFBW/MCDU1label6b",
        //"AirbusFBW/MCDU1label1y",
        //"AirbusFBW/MCDU1label2y",
        //"AirbusFBW/MCDU1label3y",
        //"AirbusFBW/MCDU1label4y",
        //"AirbusFBW/MCDU1label5y",
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
        //"AirbusFBW/MCDU1cont4m",
        //"AirbusFBW/MCDU1cont5m",
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
    //        "AirbusFBW/MCDU1cont1c",
    //        "AirbusFBW/MCDU1cont2c",
    //        "AirbusFBW/MCDU1cont3c",
    //        "AirbusFBW/MCDU1cont4c",
        //"AirbusFBW/MCDU1cont5c",
        //"AirbusFBW/MCDU1cont6c",
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
        //"AirbusFBW/MCDU1scont5y",
        //"AirbusFBW/MCDU1scont6y",
        
        "AirbusFBW/MCDU1spw", // scratchpad
        "AirbusFBW/MCDU1spa" // scratchpad
    };
    return datarefs;
}

const std::vector<MCDUButtonDef>& TolissMcduProfile::buttonDefs() const {
    static const std::vector<MCDUButtonDef> buttons = {
        {0, "LSK1L", "AirbusFBW/MCDU1LSK1L"},
        {1, "LSK2L", "AirbusFBW/MCDU1LSK2L"},
        {2, "LSK3L", "AirbusFBW/MCDU1LSK3L"},
        {3, "LSK4L", "AirbusFBW/MCDU1LSK4L"},
        {4, "LSK5L", "AirbusFBW/MCDU1LSK5L"},
        {5, "LSK6L", "AirbusFBW/MCDU1LSK6L"},
        {6, "LSK1R", "AirbusFBW/MCDU1LSK1R"},
        {7, "LSK2R", "AirbusFBW/MCDU1LSK2R"},
        {8, "LSK3R", "AirbusFBW/MCDU1LSK3R"},
        {9, "LSK4R", "AirbusFBW/MCDU1LSK4R"},
        {10, "LSK5R", "AirbusFBW/MCDU1LSK5R"},
        {11, "LSK6R", "AirbusFBW/MCDU1LSK6R"},
        {12, "DIRTO", "AirbusFBW/MCDU1DirTo"},
        {13, "PROG", "AirbusFBW/MCDU1Prog"},
        {14, "PERF", "AirbusFBW/MCDU1Perf"},
        {15, "INIT", "AirbusFBW/MCDU1Init"},
        {16, "DATA", "AirbusFBW/MCDU1Data"},
        {17, "EMPTY_TOP_RIGHT", "toliss_airbus/iscs_open"},
        {18, "BRT", "AirbusFBW/MCDU1KeyBright"},
        {19, "FPLN", "AirbusFBW/MCDU1Fpln"},
        {20, "RADNAV", "AirbusFBW/MCDU1RadNav"},
        {21, "FUEL", "AirbusFBW/MCDU1FuelPred"},
        {22, "SEC-FPLN", "AirbusFBW/MCDU1SecFpln"},
        {23, "ATC", "AirbusFBW/MCDU1ATC"},
        {24, "MENU", "AirbusFBW/MCDU1Menu"},
        {25, "DIM", "AirbusFBW/MCDU1KeyDim"},
        {26, "AIRPORT", "AirbusFBW/MCDU1Airport"},
        {27, "EMPTY_BOTTOM_LEFT", "AirbusFBW/purser/fwd"},
        {28, "SLEW_LEFT", "AirbusFBW/MCDU1SlewLeft"},
        {29, "SLEW_UP", "AirbusFBW/MCDU1SlewUp"},
        {30, "SLEW_RIGHT", "AirbusFBW/MCDU1SlewRight"},
        {31, "SLEW_DOWN", "AirbusFBW/MCDU1SlewDown"},
        {32, "KEY1", "AirbusFBW/MCDU1Key1"},
        {33, "KEY2", "AirbusFBW/MCDU1Key2"},
        {34, "KEY3", "AirbusFBW/MCDU1Key3"},
        {35, "KEY4", "AirbusFBW/MCDU1Key4"},
        {36, "KEY5", "AirbusFBW/MCDU1Key5"},
        {37, "KEY6", "AirbusFBW/MCDU1Key6"},
        {38, "KEY7", "AirbusFBW/MCDU1Key7"},
        {39, "KEY8", "AirbusFBW/MCDU1Key8"},
        {40, "KEY9", "AirbusFBW/MCDU1Key9"},
        {41, "DOT", "AirbusFBW/MCDU1KeyDecimal"},
        {42, "KEY0", "AirbusFBW/MCDU1Key0"},
        {43, "PLUSMINUS", "AirbusFBW/MCDU1KeyPM"},
        {44, "KEYA", "AirbusFBW/MCDU1KeyA"},
        {45, "KEYB", "AirbusFBW/MCDU1KeyB"},
        {46, "KEYC", "AirbusFBW/MCDU1KeyC"},
        {47, "KEYD", "AirbusFBW/MCDU1KeyD"},
        {48, "KEYE", "AirbusFBW/MCDU1KeyE"},
        {49, "KEYF", "AirbusFBW/MCDU1KeyF"},
        {50, "KEYG", "AirbusFBW/MCDU1KeyG"},
        {51, "KEYH", "AirbusFBW/MCDU1KeyH"},
        {52, "KEYI", "AirbusFBW/MCDU1KeyI"},
        {53, "KEYJ", "AirbusFBW/MCDU1KeyJ"},
        {54, "KEYK", "AirbusFBW/MCDU1KeyK"},
        {55, "KEYL", "AirbusFBW/MCDU1KeyL"},
        {56, "KEYM", "AirbusFBW/MCDU1KeyM"},
        {57, "KEYN", "AirbusFBW/MCDU1KeyN"},
        {58, "KEYO", "AirbusFBW/MCDU1KeyO"},
        {59, "KEYP", "AirbusFBW/MCDU1KeyP"},
        {60, "KEYQ", "AirbusFBW/MCDU1KeyQ"},
        {61, "KEYR", "AirbusFBW/MCDU1KeyR"},
        {62, "KEYS", "AirbusFBW/MCDU1KeyS"},
        {63, "KEYT", "AirbusFBW/MCDU1KeyT"},
        {64, "KEYU", "AirbusFBW/MCDU1KeyU"},
        {65, "KEYV", "AirbusFBW/MCDU1KeyV"},
        {66, "KEYW", "AirbusFBW/MCDU1KeyW"},
        {67, "KEYX", "AirbusFBW/MCDU1KeyX"},
        {68, "KEYY", "AirbusFBW/MCDU1KeyY"},
        {69, "KEYZ", "AirbusFBW/MCDU1KeyZ"},
        {70, "SLASH", "AirbusFBW/MCDU1KeySlash"},
        {71, "SPACE", "AirbusFBW/MCDU1KeySpace"},
        {72, "OVERFLY", "AirbusFBW/MCDU1KeyOverfly"},
        {73, "Clear", "AirbusFBW/MCDU1KeyClear"},
    };
    return buttons;
}

const std::map<char, int>& TolissMcduProfile::colorMap() const {
    static const std::map<char, int> colMap = {
        {'L', 0x0000},
        {'A', 0x0021},
        {'W', 0x0042},
        {'B', 0x0063},
        {'G', 0x0084},
        {'M', 0x00A5},
        {'R', 0x00C6},
        {'Y', 0x00E7},
        {'E', 0x0108},
        {' ', 0x0042},
    };
    return colMap;
}

void TolissMcduProfile::updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) {
    std::array<int, PAGE_BYTES_PER_LINE> spw_line{};
    std::array<int, PAGE_BYTES_PER_LINE> spa_line{};

    // Clear the page
    for (int i = 0; i < PAGE_LINES; ++i) {
        std::fill(page[i].begin(), page[i].end(), ' ');
    }

    const std::vector<std::string>& currentDatarefs = displayDatarefs();
    for (const std::string &ref : currentDatarefs) {
        auto it = cachedDatarefValues.find(ref);
        if (it == cachedDatarefValues.end()) {
            continue;
        }
        
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

        // Use cached value
        const std::string &text = it->second;
        
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
                    case 'A': c = 91; targetColor = 'b'; break;
                    case 'B': c = 93; targetColor = 'b'; break;
                    case '0': c = 60; targetColor = 'b'; break;
                    case '1': c = 62; targetColor = 'b'; break;
                    case '2': c = 60; targetColor = 'w'; break;
                    case '3': c = 62; targetColor = 'w'; break;
                    case '4': c = 60; targetColor = 'a'; break;
                    case '5': c = 62; targetColor = 'a'; break;
                    case 'E': c = 35; targetColor = 'a'; break;
                }
            }

            if (type.find("title") != std::string::npos || type.find("stitle") != std::string::npos) {
                writeLineToPage(page, 0, i, std::string(1, c), targetColor, fontSmall);
            } else if (type.find("label") != std::string::npos) {
                unsigned char lbl_line = (match[4].str().empty() ? 1 : std::stoi(match[4])) * 2 - 1;
                writeLineToPage(page, lbl_line, i, std::string(1, c), targetColor, fontSmall);
            } else if (type.find("cont") != std::string::npos || type.find("scont") != std::string::npos) {
                writeLineToPage(page, line, i, std::string(1, c), targetColor, fontSmall);
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

    // Process scratchpad data
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
        if (spw_line[i] == 0) {
            std::fill(spw_line.begin() + i, spw_line.end(), 0);
            break;
        }
    }
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
        if (spa_line[i] == 0) {
            std::fill(spa_line.begin() + i, spa_line.end(), 0);
            break;
        }
    }

    // Merge spw and spa into line 13
    int vertSlewType = Dataref::getInstance()->getCached<int>("AirbusFBW/MCDU1VertSlewKeys");
    for (int i = 0; i < PAGE_CHARS_PER_LINE; ++i) {
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

        if (vertSlewType > 0 && i >= PAGE_CHARS_PER_LINE - 2) {
            if (i == PAGE_CHARS_PER_LINE - 2 && (vertSlewType == 1 || vertSlewType == 2)) {
                dispChar = 30; // Up character
            } else if (i == PAGE_CHARS_PER_LINE - 1 && (vertSlewType == 1 || vertSlewType == 3)) {
                dispChar = 31; // Down character
            }
            
            dispColor = 'w';
            smallFont = true;
        }
                
        writeLineToPage(page, 13, i, std::string(1, dispChar), dispColor, smallFont);
    }
}

void TolissMcduProfile::writeLineToPage(std::vector<std::vector<char>>& page, int line, int pos, const std::string &text, char color, bool fontSmall) {
    if (line < 0 || line >= PAGE_LINES) {
        return;
    }
    if (pos < 0 || pos + text.length() > PAGE_CHARS_PER_LINE) {
        return;
    }
    if (text.length() > PAGE_CHARS_PER_LINE) {
        return;
    }

    pos = pos * PAGE_BYTES_PER_CHAR;
    size_t textLen = text.length();
    for (int c = 0; c < textLen; ++c) {
        int pagePos = pos + c * PAGE_BYTES_PER_CHAR;
        page[line][pagePos] = color;
        page[line][pagePos + 1] = fontSmall;
        page[line][pagePos + PAGE_BYTES_PER_CHAR - 1] = text[c];
    }
}
