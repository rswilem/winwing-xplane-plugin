#include "xcrafts-fmc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

XCraftsFMCProfile::XCraftsFMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    datarefRegex = std::regex("XCrafts/FMS/CDU_[0-9]+_([0-9]{2}|ScratchPad)");

    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::FontXCrafts, product->identifierByte));

    const std::string cdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "CDU1" : "CDU2";
    Dataref::getInstance()->monitorExistingDataref<float>(("XCrafts/FMS/" + cdu + "_brt").c_str(), [product](float rawBrightness) {
        bool poweredOn = Dataref::getInstance()->getCached<bool>("XCrafts/FMS/power_stat");
        uint8_t brightness = poweredOn ? rawBrightness * 255.0f : 0;
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, brightness);
        product->setLedBrightness(FMCLed::BACKLIGHT, brightness);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("XCrafts/FMS/power_stat", [this, cdu](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref(("XCrafts/FMS/" + cdu + "_brt").c_str());
    });
}

XCraftsFMCProfile::~XCraftsFMCProfile() {
    const std::string cdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "CDU1" : "CDU2";
    Dataref::getInstance()->unbind(("XCrafts/FMS/" + cdu + "_brt").c_str());
    Dataref::getInstance()->unbind("XCrafts/FMS/power_stat");
}

bool XCraftsFMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("XCrafts/FMS/CDU_1_01");
}

const std::vector<std::string> &XCraftsFMCProfile::displayDatarefs() const {
    const std::string cduNumber = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "1" : "2";
    static std::unordered_map<FMCDeviceVariant, std::vector<std::string>> cache;

    return cache.try_emplace(product->deviceVariant,
                    [cduNumber]() {
                        std::vector<std::string> datarefs;
                        datarefs.push_back("XCrafts/FMS/data_count" + cduNumber);

                        for (int i = 1; i <= 70; i++) {
                            char buffer[32];
                            snprintf(buffer, sizeof(buffer), "XCrafts/FMS/CDU_%s_%02d", cduNumber.c_str(), i);
                            datarefs.push_back(std::string(buffer));
                        }

                        datarefs.push_back("XCrafts/FMS/CDU_" + cduNumber + "_ScratchPad");
                        return datarefs;
                    }())
        .first->second;
}

const std::vector<FMCButtonDef> &XCraftsFMCProfile::buttonDefs() const {
    const std::string cdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "CDU_1" : "CDU_2";
    static std::unordered_map<FMCDeviceVariant, std::vector<FMCButtonDef>> cache;

    return cache.try_emplace(product->deviceVariant,
                    std::vector<FMCButtonDef>{
                        // Line Select Keys (XCrafts ERJ format)
                        {FMCKey::LSK1L, "XCrafts/ERJ/" + cdu + "/LSK1"},
                        {FMCKey::LSK2L, "XCrafts/ERJ/" + cdu + "/LSK2"},
                        {FMCKey::LSK3L, "XCrafts/ERJ/" + cdu + "/LSK3"},
                        {FMCKey::LSK4L, "XCrafts/ERJ/" + cdu + "/LSK4"},
                        {FMCKey::LSK5L, "XCrafts/ERJ/" + cdu + "/LSK5"},
                        {FMCKey::LSK6L, "XCrafts/ERJ/" + cdu + "/LSK6"},
                        {FMCKey::LSK1R, "XCrafts/ERJ/" + cdu + "/RSK1"},
                        {FMCKey::LSK2R, "XCrafts/ERJ/" + cdu + "/RSK2"},
                        {FMCKey::LSK3R, "XCrafts/ERJ/" + cdu + "/RSK3"},
                        {FMCKey::LSK4R, "XCrafts/ERJ/" + cdu + "/RSK4"},
                        {FMCKey::LSK5R, "XCrafts/ERJ/" + cdu + "/RSK5"},
                        {FMCKey::LSK6R, "XCrafts/ERJ/" + cdu + "/RSK6"},

                        {std::vector<FMCKey>{FMCKey::MCDU_SEC_FPLN, FMCKey::PFP_INIT_REF}, "XCrafts/ERJ/" + cdu + "/Key_NAV;XCrafts/ERJ/" + cdu + "/LSK4"}, // Nav IDENT
                        {FMCKey::PFP3_CLB, "XCrafts/ERJ/" + cdu + "/Key_PERF;XCrafts/ERJ/" + cdu + "/RSK2"},                                                // Perf CLIMB
                        {FMCKey::PFP3_CRZ, "XCrafts/ERJ/" + cdu + "/Key_PERF;XCrafts/ERJ/" + cdu + "/LSK2"},                                                // Perf CRUISE
                        {std::vector<FMCKey>{FMCKey::MCDU_AIRPORT, FMCKey::PFP3_DES}, "XCrafts/ERJ/" + cdu + "/Key_PERF;XCrafts/ERJ/" + cdu + "/LSK3"},     // Perf DESCENT
                                                                                                                                                            //        {std::vector<FMCKey>{FMCKey::PFP4_VNAV, FMCKey::PFP7_VNAV}, "XCrafts/ERJ/"+cdu+"/Key_NAV;XCrafts/ERJ/"+cdu+"/LSK3"}, // Nav TOD details
                                                                                                                                                            //        {FMCKey::PFP_HOLD, "XCrafts/ERJ/"+cdu+"/Key_NAV;XCrafts/ERJ/"+cdu+"/LSK3"}, // Nav HOLD
                                                                                                                                                            //        {FMCKey::PFP_FIX, "XCrafts/ERJ/"+cdu+"/Key_NAV;XCrafts/ERJ/"+cdu+"/LSK3"}, // Nav PILOT WAYPOINT

                        //        {FMCKey::PFP_EXEC, "custom_tbd"},
                        //        {FMCKey::PFP_DEP_ARR, "custom_tbd"},

                        {FMCKey::PROG, "XCrafts/ERJ/" + cdu + "/Key_PROG"},
                        {FMCKey::PFP3_N1_LIMIT, "XCrafts/ERJ/" + cdu + "/Key_TRS"},
                        {FMCKey::MCDU_PERF, "XCrafts/ERJ/" + cdu + "/Key_PERF"},
                        {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP_ROUTE}, "XCrafts/ERJ/" + cdu + "/Key_RTE"},
                        {std::vector<FMCKey>{FMCKey::MCDU_DATA, FMCKey::PFP4_FMC_COMM, FMCKey::PFP7_FMC_COMM}, "XCrafts/ERJ/" + cdu + "/Key_DLK"},
                        {std::vector<FMCKey>{FMCKey::MCDU_FPLN, FMCKey::PFP_LEGS}, "XCrafts/ERJ/" + cdu + "/Key_FPL"},
                        {std::vector<FMCKey>{FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD, FMCKey::PFP7_NAV_RAD}, "XCrafts/ERJ/" + cdu + "/Key_RADIO"},
                        {FMCKey::MENU, "XCrafts/ERJ/" + cdu + "/Key_DLK"},
                        {FMCKey::PAGE_PREV, "XCrafts/ERJ/" + cdu + "/Key_PREV"},
                        {FMCKey::PAGE_NEXT, "XCrafts/ERJ/" + cdu + "/Key_NEXT"},

                        // Numeric Keys
                        {FMCKey::KEY1, "XCrafts/ERJ/" + cdu + "/Key_1"},
                        {FMCKey::KEY2, "XCrafts/ERJ/" + cdu + "/Key_2"},
                        {FMCKey::KEY3, "XCrafts/ERJ/" + cdu + "/Key_3"},
                        {FMCKey::KEY4, "XCrafts/ERJ/" + cdu + "/Key_4"},
                        {FMCKey::KEY5, "XCrafts/ERJ/" + cdu + "/Key_5"},
                        {FMCKey::KEY6, "XCrafts/ERJ/" + cdu + "/Key_6"},
                        {FMCKey::KEY7, "XCrafts/ERJ/" + cdu + "/Key_7"},
                        {FMCKey::KEY8, "XCrafts/ERJ/" + cdu + "/Key_8"},
                        {FMCKey::KEY9, "XCrafts/ERJ/" + cdu + "/Key_9"},
                        {FMCKey::KEY0, "XCrafts/ERJ/" + cdu + "/Key_0"},
                        {FMCKey::PERIOD, "XCrafts/ERJ/" + cdu + "/Key_Decimal"},
                        {FMCKey::PLUSMINUS, "XCrafts/ERJ/" + cdu + "/Key_PlusMinus"},

                        // Alpha Keys
                        {FMCKey::KEYA, "XCrafts/ERJ/" + cdu + "/Key_A"},
                        {FMCKey::KEYB, "XCrafts/ERJ/" + cdu + "/Key_B"},
                        {FMCKey::KEYC, "XCrafts/ERJ/" + cdu + "/Key_C"},
                        {FMCKey::KEYD, "XCrafts/ERJ/" + cdu + "/Key_D"},
                        {FMCKey::KEYE, "XCrafts/ERJ/" + cdu + "/Key_E"},
                        {FMCKey::KEYF, "XCrafts/ERJ/" + cdu + "/Key_F"},
                        {FMCKey::KEYG, "XCrafts/ERJ/" + cdu + "/Key_G"},
                        {FMCKey::KEYH, "XCrafts/ERJ/" + cdu + "/Key_H"},
                        {FMCKey::KEYI, "XCrafts/ERJ/" + cdu + "/Key_I"},
                        {FMCKey::KEYJ, "XCrafts/ERJ/" + cdu + "/Key_J"},
                        {FMCKey::KEYK, "XCrafts/ERJ/" + cdu + "/Key_K"},
                        {FMCKey::KEYL, "XCrafts/ERJ/" + cdu + "/Key_L"},
                        {FMCKey::KEYM, "XCrafts/ERJ/" + cdu + "/Key_M"},
                        {FMCKey::KEYN, "XCrafts/ERJ/" + cdu + "/Key_N"},
                        {FMCKey::KEYO, "XCrafts/ERJ/" + cdu + "/Key_O"},
                        {FMCKey::KEYP, "XCrafts/ERJ/" + cdu + "/Key_P"},
                        {FMCKey::KEYQ, "XCrafts/ERJ/" + cdu + "/Key_Q"},
                        {FMCKey::KEYR, "XCrafts/ERJ/" + cdu + "/Key_R"},
                        {FMCKey::KEYS, "XCrafts/ERJ/" + cdu + "/Key_S"},
                        {FMCKey::KEYT, "XCrafts/ERJ/" + cdu + "/Key_T"},
                        {FMCKey::KEYU, "XCrafts/ERJ/" + cdu + "/Key_U"},
                        {FMCKey::KEYV, "XCrafts/ERJ/" + cdu + "/Key_V"},
                        {FMCKey::KEYW, "XCrafts/ERJ/" + cdu + "/Key_W"},
                        {FMCKey::KEYX, "XCrafts/ERJ/" + cdu + "/Key_X"},
                        {FMCKey::KEYY, "XCrafts/ERJ/" + cdu + "/Key_Y"},
                        {FMCKey::KEYZ, "XCrafts/ERJ/" + cdu + "/Key_Z"},

                        // Brightness Controls
                        {FMCKey::BRIGHTNESS_UP, "XCrafts/ERJ/" + cdu + "/Key_BRT"},
                        {FMCKey::BRIGHTNESS_DOWN, "XCrafts/ERJ/" + cdu + "/Key_DIM"},

                        // Special Keys
                        {FMCKey::SPACE, "XCrafts/ERJ/" + cdu + "/Key_Space"},
                        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, "XCrafts/ERJ/" + cdu + "/Key_DEL"},
                        {FMCKey::SLASH, "XCrafts/ERJ/" + cdu + "/Key_Slash_Command"},
                        {FMCKey::CLR, "XCrafts/ERJ/" + cdu + "/Key_CLR"}})
        .first->second;
}

const std::unordered_map<FMCKey, const FMCButtonDef *> &XCraftsFMCProfile::buttonKeyMap() const {
    static std::unordered_map<FMCDeviceVariant, std::unordered_map<FMCKey, const FMCButtonDef *>> cache;

    auto it = cache.find(product->deviceVariant);
    if (it == cache.end()) {
        std::unordered_map<FMCKey, const FMCButtonDef *> map;
        const auto &buttons = buttonDefs();
        for (const auto &button : buttons) {
            std::visit([&](auto &&k) {
                using T = std::decay_t<decltype(k)>;
                if constexpr (std::is_same_v<T, FMCKey>) {
                    map[k] = &button;
                } else {
                    for (const auto &key : k) {
                        map[key] = &button;
                    }
                }
            },
                button.key);
        }
        it = cache.emplace(product->deviceVariant, std::move(map)).first;
    }
    return it->second;
}

const std::map<char, FMCTextColor> &XCraftsFMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colors = {
        {0x00, COLOR_WHITE},
        {0x01, COLOR_CYAN},
        {0x02, COLOR_GREEN},
        {0x03, COLOR_YELLOW},
        {0x04, COLOR_MAGENTA},
        {0x05, COLOR_RED},
        {0x06, COLOR_GREY},
        {0x07, COLOR_AMBER},
        {0x08, COLOR_DARKBROWN},

        {0xF0, COLOR_WHITE_BG},
        {0xF1, COLOR_CYAN_BG},
        {0xF2, COLOR_GREEN_BG},
        {0xF3, COLOR_YELLOW_BG},
        {0xF4, COLOR_MAGENTA_BG},
        {0xF5, COLOR_RED_BG},
        {0xF6, COLOR_GREY_BG},
        {0xF7, COLOR_AMBER_BG},
        {0xF8, COLOR_DARKBROWN_BG}};

    return colors;
}

void XCraftsFMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '?':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;

        default:
            buffer->push_back(character);
            break;
    }
}

void XCraftsFMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    const std::string cduNumber = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "1" : "2";

    int dataCount = datarefManager->getCached<int>(("XCrafts/FMS/data_count" + cduNumber).c_str());
    for (int i = 1; i <= std::min(dataCount, 70); i++) {
        char datarefName[32];
        snprintf(datarefName, sizeof(datarefName), "XCrafts/FMS/CDU_%s_%02d", cduNumber.c_str(), i);

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

        unsigned short row = (text[0] - '0') * 10 + (text[1] - '0');
        unsigned short col = (text[2] - '0') * 10 + (text[3] - '0');

        XCraftsFMCFontStyle fontStyle = XCraftsFMCFontStyle::Large;
        if (text.size() >= 5) {
            fontStyle = XCraftsFMCFontStyle(text[4] - '0');
        }

        unsigned char colorCode = text[5] - '0';
        if (fontStyle >= XCraftsFMCFontStyle::LargeReversed) {
            colorCode = 0xF0 + colorCode;
        }

        int lineIndex = row - 1;
        int colIndex = col - 1;

        if (lineIndex < 0 || lineIndex >= ProductFMC::PageLines || colIndex < 0) {
            continue;
        }

        constexpr unsigned char textStartIndex = 6;
        if (text.size() > textStartIndex) {
            for (int j = textStartIndex; j < text.size() && (colIndex + (j - textStartIndex)) < ProductFMC::PageCharsPerLine; j++) {
                unsigned char c = text[j];
                if (c == 0x00) {
                    break;
                }

                int displayCol = colIndex + (j - textStartIndex);
                bool isSmallFont = fontStyle == XCraftsFMCFontStyle::Small || fontStyle == XCraftsFMCFontStyle::SmallReversed || fontStyle == XCraftsFMCFontStyle::SmallReversedBox;
                product->writeLineToPage(page, lineIndex, displayCol, std::string(1, (char) c), colorCode, isSmallFont);
            }
        }
    }

    std::vector<unsigned char> scratchpadText = datarefManager->getCached<std::vector<unsigned char>>(("XCrafts/FMS/CDU_" + cduNumber + "_ScratchPad").c_str());
    if (!scratchpadText.empty()) {
        for (int i = 0; i < scratchpadText.size() && i < ProductFMC::PageCharsPerLine; ++i) {
            unsigned char c = scratchpadText[i];
            if (c == 0x00 || c == '|') {
                break;
            }
            product->writeLineToPage(page, 13, i, std::string(1, (char) c), 0, false);
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
            if (index >= 0 && index < (int) vec.size()) {
                vec[index] = std::clamp(vec[index] + button->value, 0.0, 1.0);
                Dataref::getInstance()->set<std::vector<float>>(baseRef.c_str(), vec);
            }
        } else {
            Dataref::getInstance()->set<float>(ref.c_str(), button->value);
        }
    } else {
        // Handle command buttons, optionally handle multiple with ;
        std::string ref = button->dataref;
        std::stringstream ss(ref);
        std::string cmd;
        int milliseconds = 0;
        while (std::getline(ss, cmd, ';')) {
            if (!cmd.empty()) {
                AppState::getInstance()->executeAfter(milliseconds, [cmd, phase]() {
                    Dataref::getInstance()->executeCommand(cmd.c_str(), phase);
                });

                milliseconds += 200;
            }
        }
    }
}
