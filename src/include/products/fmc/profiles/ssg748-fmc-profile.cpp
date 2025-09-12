#include "ssg748-fmc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "font.h"
#include "product-fmc.h"

#include <algorithm>
#include <cstring>
#include <regex>

SSG748FMCProfile::SSG748FMCProfile(ProductFMC *product) :
    FMCAircraftProfile(product) {
    datarefRegex = std::regex("SSG/UFMC/LINE_([0-9]+)");

    product->setAllLedsEnabled(false);
    product->setFont(Font::GlyphData(FontVariant::FontVGA1, product->identifierByte));

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("ssg/LGT/mcdu_brt_sw", [product](std::vector<float> brightness) {
        if (brightness.size() < 27) {
            return;
        }

        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[10] * 255.0f : 0;
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, target);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("ssg/LGT/mcdu_brt_sw");
    });
}

SSG748FMCProfile::~SSG748FMCProfile() {
    Dataref::getInstance()->unbind("ssg/LGT/mcdu_brt_sw");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool SSG748FMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("SSG/748/simtime");
}

const std::vector<std::string> &SSG748FMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        "SSG/UFMC/LINE_1",
        "SSG/UFMC/LINE_2",
        "SSG/UFMC/LINE_3",
        "SSG/UFMC/LINE_4",
        "SSG/UFMC/LINE_5",
        "SSG/UFMC/LINE_6",
        "SSG/UFMC/LINE_7",
        "SSG/UFMC/LINE_8",
        "SSG/UFMC/LINE_9",
        "SSG/UFMC/LINE_10",
        "SSG/UFMC/LINE_11",
        "SSG/UFMC/LINE_12",
        "SSG/UFMC/LINE_13",
        "SSG/UFMC/LINE_14"};

    return datarefs;
}

const std::vector<FMCButtonDef> &SSG748FMCProfile::buttonDefs() const {
    // SSG/CDU/cdu1_init_sw
    // SSG/CDU/cdu1_g_sw set to 1 then 0
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, "SSG/CDU/cdu1_lk1_sw"},
        {FMCKey::LSK2L, "SSG/CDU/cdu1_lk2_sw"},
        {FMCKey::LSK3L, "SSG/CDU/cdu1_lk3_sw"},
        {FMCKey::LSK4L, "SSG/CDU/cdu1_lk4_sw"},
        {FMCKey::LSK5L, "SSG/CDU/cdu1_lk5_sw"},
        {FMCKey::LSK6L, "SSG/CDU/cdu1_lk6_sw"},
        {FMCKey::LSK1R, "SSG/CDU/cdu1_rk1_sw"},
        {FMCKey::LSK2R, "SSG/CDU/cdu1_rk2_sw"},
        {FMCKey::LSK3R, "SSG/CDU/cdu1_rk3_sw"},
        {FMCKey::LSK4R, "SSG/CDU/cdu1_rk4_sw"},
        {FMCKey::LSK5R, "SSG/CDU/cdu1_rk5_sw"},
        {FMCKey::LSK6R, "SSG/CDU/cdu1_rk6_sw"},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, "SSG/CDU/cdu1_init_sw"},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, "SSG/CDU/cdu1_rte_sw"},
        {std::vector<FMCKey>{FMCKey::PFP4_ATC, FMCKey::MCDU_ATC_COMM}, "SSG/CDU/cdu1_atc_sw"},
        {std::vector<FMCKey>{FMCKey::PFP4_VNAV, FMCKey::MCDU_DATA, FMCKey::PFP7_VNAV}, "SSG/CDU/cdu1_vnav_sw"},
        {FMCKey::BRIGHTNESS_DOWN, "SSG/CDU/cdu1_brt_sw", -1},
        {FMCKey::BRIGHTNESS_UP, "SSG/CDU/cdu1_brt_sw", 1},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, "SSG/CDU/cdu1_fix_sw"},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, "SSG/CDU/cdu1_legs_sw"},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, "SSG/CDU/cdu1_dep_arr_sw"},
        {FMCKey::PFP_HOLD, "SSG/CDU/cdu1_hold_sw"},
        {std::vector<FMCKey>{FMCKey::PFP4_FMC_COMM, FMCKey::PFP7_FMC_COMM}, "SSG/CDU/cdu1_comm_sw"},
        {FMCKey::PROG, "SSG/CDU/cdu1_prog_sw"},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, "SSG/CDU/cdu1_exec_sw"},
        {FMCKey::MENU, "SSG/CDU/cdu1_menu_sw"},
        {std::vector<FMCKey>{FMCKey::PFP4_NAV_RAD, FMCKey::MCDU_RAD_NAV, FMCKey::PFP7_NAV_RAD}, "SSG/CDU/cdu1_radio_sw"},
        {FMCKey::PAGE_PREV, "SSG/CDU/cdu1_prev_sw"},
        {FMCKey::PAGE_NEXT, "SSG/CDU/cdu1_next_sw"},
        {FMCKey::KEY1, "SSG/CDU/cdu1_1_sw"},
        {FMCKey::KEY2, "SSG/CDU/cdu1_2_sw"},
        {FMCKey::KEY3, "SSG/CDU/cdu1_3_sw"},
        {FMCKey::KEY4, "SSG/CDU/cdu1_4_sw"},
        {FMCKey::KEY5, "SSG/CDU/cdu1_5_sw"},
        {FMCKey::KEY6, "SSG/CDU/cdu1_6_sw"},
        {FMCKey::KEY7, "SSG/CDU/cdu1_7_sw"},
        {FMCKey::KEY8, "SSG/CDU/cdu1_8_sw"},
        {FMCKey::KEY9, "SSG/CDU/cdu1_9_sw"},
        {FMCKey::PERIOD, "SSG/CDU/cdu1_dot_sw"},
        {FMCKey::KEY0, "SSG/CDU/cdu1_0_sw"},
        {FMCKey::PLUSMINUS, "SSG/CDU/cdu1_dash_sw"},
        {FMCKey::KEYA, "SSG/CDU/cdu1_a_sw"},
        {FMCKey::KEYB, "SSG/CDU/cdu1_b_sw"},
        {FMCKey::KEYC, "SSG/CDU/cdu1_c_sw"},
        {FMCKey::KEYD, "SSG/CDU/cdu1_d_sw"},
        {FMCKey::KEYE, "SSG/CDU/cdu1_e_sw"},
        {FMCKey::KEYF, "SSG/CDU/cdu1_f_sw"},
        {FMCKey::KEYG, "SSG/CDU/cdu1_g_sw"},
        {FMCKey::KEYH, "SSG/CDU/cdu1_h_sw"},
        {FMCKey::KEYI, "SSG/CDU/cdu1_i_sw"},
        {FMCKey::KEYJ, "SSG/CDU/cdu1_j_sw"},
        {FMCKey::KEYK, "SSG/CDU/cdu1_k_sw"},
        {FMCKey::KEYL, "SSG/CDU/cdu1_l_sw"},
        {FMCKey::KEYM, "SSG/CDU/cdu1_m_sw"},
        {FMCKey::KEYN, "SSG/CDU/cdu1_n_sw"},
        {FMCKey::KEYO, "SSG/CDU/cdu1_o_sw"},
        {FMCKey::KEYP, "SSG/CDU/cdu1_p_sw"},
        {FMCKey::KEYQ, "SSG/CDU/cdu1_q_sw"},
        {FMCKey::KEYR, "SSG/CDU/cdu1_r_sw"},
        {FMCKey::KEYS, "SSG/CDU/cdu1_s_sw"},
        {FMCKey::KEYT, "SSG/CDU/cdu1_t_sw"},
        {FMCKey::KEYU, "SSG/CDU/cdu1_u_sw"},
        {FMCKey::KEYV, "SSG/CDU/cdu1_v_sw"},
        {FMCKey::KEYW, "SSG/CDU/cdu1_w_sw"},
        {FMCKey::KEYX, "SSG/CDU/cdu1_x_sw"},
        {FMCKey::KEYY, "SSG/CDU/cdu1_y_sw"},
        {FMCKey::KEYZ, "SSG/CDU/cdu1_z_sw"},
        {FMCKey::SPACE, "SSG/CDU/cdu1_sp_sw"},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, "SSG/CDU/cdu1_del_sw"},
        {FMCKey::SLASH, "SSG/CDU/cdu1_slash_sw"},
        {FMCKey::CLR, "SSG/CDU/cdu1_clr_sw"}};

    return buttons;
}

const std::map<char, FMCTextColor> &SSG748FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {'c', FMCTextColor::COLOR_CYAN},
        {'g', FMCTextColor::COLOR_GREEN},
        {'p', FMCTextColor::COLOR_MAGENTA},
        {'w', FMCTextColor::COLOR_WHITE},
        {'l', FMCTextColor::COLOR_RED},      // l = Large/white
        {'s', FMCTextColor::COLOR_GREY},     // s = Small/white
        {'x', FMCTextColor::COLOR_RED},      // x = Special/labels (white)
        {'i', FMCTextColor::COLOR_WHITE_BG}, // i = Inverted
    };

    return colMap;
}

void SSG748FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;

        case '=':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;

        default:
            buffer->push_back(character);
            break;
    }
}

void SSG748FMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    for (const auto &ref : displayDatarefs()) {
        std::smatch match;
        if (!std::regex_match(ref, match, datarefRegex)) {
            continue;
        }

        int lineNum = std::stoi(match[1]);
        int lineIndex = lineNum - 1;

        if (lineIndex < 0 || lineIndex >= ProductFMC::PageLines) {
            continue;
        }

        std::string text = datarefManager->getCached<std::string>(ref.c_str());
        if (text.empty()) {
            continue;
        }

        char currentColor = 'W';
        bool fontSmall = lineIndex % 2 == 1;
        int displayPos = 0;

        for (int i = 0; i < text.size() && displayPos < ProductFMC::PageCharsPerLine; ++i) {
            char c = text[i];
            if (c == 0x00) {
                break;
            }

            if (c == ';' && i + 1 < text.size()) {
                char colorCode = text[i + 1];
                currentColor = colorCode;
                i++; // Skip the color code character
                continue;
            }

            if (c == '[' && i + 1 < text.size() && text[i + 1] == ']') {
                product->writeLineToPage(page, lineIndex, displayPos, "#", currentColor, fontSmall);
                i++; // Skip the closing bracket
                displayPos++;
                continue;
            }

            if (c != 0x20) {
                product->writeLineToPage(page, lineIndex, displayPos, std::string(1, toupper(c)), currentColor, fontSmall);
            }
            displayPos++;
        }
    }
}

void SSG748FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (phase == xplm_CommandContinue) {
        return;
    }

    Dataref::getInstance()->set<int>(button->dataref.c_str(), phase == xplm_CommandBegin ? 1 : 0);

    // If the dataref contains "cdu1", also execute for "cdu2" because the SSG/CDU/LINE datarefs don't update
    auto pos = button->dataref.find("cdu1");
    if (pos != std::string::npos) {
        std::string cdu2Dataref = button->dataref;
        cdu2Dataref.replace(pos, 4, "cdu2");
        Dataref::getInstance()->set<int>(cdu2Dataref.c_str(), phase == xplm_CommandBegin ? 1 : 0);
    }
}
