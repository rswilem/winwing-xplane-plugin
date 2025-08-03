#include "ssg748-pfp-profile.h"
#include "product-pfp.h"
#include "dataref.h"
#include "appstate.h"
#include <cstring>
#include <algorithm>
#include <regex>

SSG748PfpProfile::SSG748PfpProfile(ProductPFP *product) : PfpAircraftProfile(product) {
    datarefRegex = std::regex("SSG/UFMC/LINE_([0-9]+)");
    
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
        
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("ssg/LGT/mcdu_brt_sw", [product](std::vector<float> brightness) {
        if (brightness.size() < 27) {
            return;
        }
        
        uint8_t target = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on") ? brightness[10] * 255.0f : 0;
        product->setLedBrightness(PFPLed::BACKLIGHT, target);
        product->setLedBrightness(PFPLed::SCREEN_BACKLIGHT, target);
    });
    
    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("ssg/LGT/mcdu_brt_sw");
    });
}

SSG748PfpProfile::~SSG748PfpProfile() {
    Dataref::getInstance()->unbind("ssg/LGT/mcdu_brt_sw");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
}

bool SSG748PfpProfile::IsEligible() {
    return Dataref::getInstance()->exists("SSG/748/simtime");
}

const std::vector<std::string>& SSG748PfpProfile::displayDatarefs() const {
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
        "SSG/UFMC/LINE_14"
    };
    
    return datarefs;
}

const std::vector<PFPButtonDef>& SSG748PfpProfile::buttonDefs() const {
    //SSG/CDU/cdu1_init_sw
    //SSG/CDU/cdu1_g_sw set to 1 then 0
    static const std::vector<PFPButtonDef> sevenFourSevenButtonLayout = {
        {0, "LSK1L", "SSG/CDU/cdu1_lk1_sw"},
        {1, "LSK2L", "SSG/CDU/cdu1_lk2_sw"},
        {2, "LSK3L", "SSG/CDU/cdu1_lk3_sw"},
        {3, "LSK4L", "SSG/CDU/cdu1_lk4_sw"},
        {4, "LSK5L", "SSG/CDU/cdu1_lk5_sw"},
        {5, "LSK6L", "SSG/CDU/cdu1_lk6_sw"},
        {6, "LSK1R", "SSG/CDU/cdu1_rk1_sw"},
        {7, "LSK2R", "SSG/CDU/cdu1_rk2_sw"},
        {8, "LSK3R", "SSG/CDU/cdu1_rk3_sw"},
        {9, "LSK4R", "SSG/CDU/cdu1_rk4_sw"},
        {10, "LSK5R", "SSG/CDU/cdu1_rk5_sw"},
        {11, "LSK6R", "SSG/CDU/cdu1_rk6_sw"},
        {12, "INITREF", "SSG/CDU/cdu1_init_sw"},
        {13, "RTE", "SSG/CDU/cdu1_rte_sw"},
        {14, "DEP/ARR", "SSG/CDU/cdu1_dep_arr_sw"},
        {15, "ATC", "SSG/CDU/cdu1_atc_sw"},
        {16, "VNAV", "SSG/CDU/cdu1_vnav_sw"},
        {17, "BRT-", "SSG/CDU/cdu1_brt_sw", -1},
        {18, "BRT+", "SSG/CDU/cdu1_brt_sw", 1},
        {19, "FIX", "SSG/CDU/cdu1_fix_sw"},
        {20, "LEGS", "SSG/CDU/cdu1_legs_sw"},
        {21, "HOLD", "SSG/CDU/cdu1_hold_sw"},
        {22, "FMC/COMM", "SSG/CDU/cdu1_comm_sw"},
        {23, "PROG", "SSG/CDU/cdu1_prog_sw"},
        {24, "EXEC", "SSG/CDU/cdu1_exec_sw"},
        {25, "MENU", "SSG/CDU/cdu1_menu_sw"},
        {26, "NAV/RAD", "SSG/CDU/cdu1_radio_sw"},
        {27, "PREV_PAGE", "SSG/CDU/cdu1_prev_sw"},
        {28, "NEXT_PAGE", "SSG/CDU/cdu1_next_sw"},
        {29, "KEY1", "SSG/CDU/cdu1_1_sw"},
        {30, "KEY2", "SSG/CDU/cdu1_2_sw"},
        {31, "KEY3", "SSG/CDU/cdu1_3_sw"},
        {32, "KEY4", "SSG/CDU/cdu1_4_sw"},
        {33, "KEY5", "SSG/CDU/cdu1_5_sw"},
        {34, "KEY6", "SSG/CDU/cdu1_6_sw"},
        {35, "KEY7", "SSG/CDU/cdu1_7_sw"},
        {36, "KEY8", "SSG/CDU/cdu1_8_sw"},
        {37, "KEY9", "SSG/CDU/cdu1_9_sw"},
        {38, "PERIOD", "SSG/CDU/cdu1_dot_sw"},
        {39, "KEY0", "SSG/CDU/cdu1_0_sw"},
        {40, "PLUSMINUS", "SSG/CDU/cdu1_dash_sw"},
        {41, "KEYA", "SSG/CDU/cdu1_a_sw"},
        {42, "KEYB", "SSG/CDU/cdu1_b_sw"},
        {43, "KEYC", "SSG/CDU/cdu1_c_sw"},
        {44, "KEYD", "SSG/CDU/cdu1_d_sw"},
        {45, "KEYE", "SSG/CDU/cdu1_e_sw"},
        {46, "KEYF", "SSG/CDU/cdu1_f_sw"},
        {47, "KEYG", "SSG/CDU/cdu1_g_sw"},
        {48, "KEYH", "SSG/CDU/cdu1_h_sw"},
        {49, "KEYI", "SSG/CDU/cdu1_i_sw"},
        {50, "KEYJ", "SSG/CDU/cdu1_j_sw"},
        {51, "KEYK", "SSG/CDU/cdu1_k_sw"},
        {52, "KEYL", "SSG/CDU/cdu1_l_sw"},
        {53, "KEYM", "SSG/CDU/cdu1_m_sw"},
        {54, "KEYN", "SSG/CDU/cdu1_n_sw"},
        {55, "KEYO", "SSG/CDU/cdu1_o_sw"},
        {56, "KEYP", "SSG/CDU/cdu1_p_sw"},
        {57, "KEYQ", "SSG/CDU/cdu1_q_sw"},
        {58, "KEYR", "SSG/CDU/cdu1_r_sw"},
        {59, "KEYS", "SSG/CDU/cdu1_s_sw"},
        {60, "KEYT", "SSG/CDU/cdu1_t_sw"},
        {61, "KEYU", "SSG/CDU/cdu1_u_sw"},
        {62, "KEYV", "SSG/CDU/cdu1_v_sw"},
        {63, "KEYW", "SSG/CDU/cdu1_w_sw"},
        {64, "KEYX", "SSG/CDU/cdu1_x_sw"},
        {65, "KEYY", "SSG/CDU/cdu1_y_sw"},
        {66, "KEYZ", "SSG/CDU/cdu1_z_sw"},
        {67, "SPACE", "SSG/CDU/cdu1_sp_sw"},
        {68, "DEL", "SSG/CDU/cdu1_del_sw"},
        {69, "SLASH", "SSG/CDU/cdu1_slash_sw"},
        {70, "CLR", "SSG/CDU/cdu1_clr_sw"}
    };
    
    return sevenFourSevenButtonLayout;
}

const std::map<char, int>& SSG748PfpProfile::colorMap() const {
    static const std::map<char, int> colMap = {
        {'c', 0x0063}, // c = Cyan (blue)
        {'g', 0x0084}, // g = Green
        {'p', 0x00A5}, // m = Magenta
        {'w', 0x0042}, // w = White
        {'l', 0x0042}, // l = Large/white
        {'s', 0x0042}, // s = Small/white
        {'x', 0x0042}, // x = Special/labels (white)
        {'i', 0x0042}, // i = Inverted (white for now)
    };

    return colMap;
}

void SSG748PfpProfile::updatePage(std::vector<std::vector<char>>& page) {
    page = std::vector<std::vector<char>>(ProductPFP::PageLines, std::vector<char>(ProductPFP::PageCharsPerLine * ProductPFP::PageBytesPerChar, ' '));
    
    auto datarefManager = Dataref::getInstance();
    for (const auto& ref : displayDatarefs()) {
        std::smatch match;
        if (!std::regex_match(ref, match, datarefRegex)) {
            continue;
        }
        
        int lineNum = std::stoi(match[1]);
        int lineIndex = lineNum - 1;
        
        if (lineIndex < 0 || lineIndex >= ProductPFP::PageLines) {
            continue;
        }
        
        std::string text = datarefManager->getCached<std::string>(ref.c_str());
        if (text.empty()) {
            continue;
        }
        
        char currentColor = 'W';
        bool fontSmall = lineIndex % 2 == 1;
        int displayPos = 0;
        
        for (int i = 0; i < text.size() && displayPos < ProductPFP::PageCharsPerLine; ++i) {
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
                product->writeLineToPage(page, lineIndex, displayPos, "*", currentColor, fontSmall);
                i++; // Skip the closing bracket
                displayPos++;
                continue;
            }
            
            if (c != 0x20) {
                product->writeLineToPage(page, lineIndex, displayPos, std::string(1, c), currentColor, fontSmall);
            }
            displayPos++;
        }
    }
}

void SSG748PfpProfile::buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) {
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
