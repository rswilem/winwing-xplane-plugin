#include "pap3_menu.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <XPLMMenus.h>
#include <XPLMUtilities.h>

namespace pap3menu {

    static XPLMMenuID s_mainMenuId = nullptr;
    static XPLMMenuID s_pap3MenuId = nullptr;
    static int s_itemShowLabels = -1;
    static bool s_showLabels = false;

    static std::string GetIniPath() {
        char sys[1024] = {0};
        XPLMGetSystemPath(sys);
        std::string p(sys);
        if (!p.empty() && p.back() != '/' && p.back() != '\\') {
            p += '/';
        }
        p += "Output/preferences/winwing_pap3.ini";
        return p;
    }

    static void LoadIni() {
        s_showLabels = false; // default
        std::ifstream f(GetIniPath());
        if (!f.is_open()) {
            return;
        }
        std::string line;
        while (std::getline(f, line)) {
            if (line.rfind("show_labels=", 0) == 0) {
                s_showLabels = std::atoi(line.c_str() + 12) != 0;
            }
        }
    }

    static void SaveIniAll() {
        const std::string ini = GetIniPath();
        // keep unrelated lines
        std::ifstream in(ini);
        std::string acc;
        if (in.is_open()) {
            std::string s;
            while (std::getline(in, s)) {
                if (s.rfind("show_labels=", 0) == 0) {
                    continue;
                }
                acc += s + "\n";
            }
            in.close();
        }
        std::ofstream out(ini, std::ios::trunc);
        if (!out.is_open()) {
            return;
        }
        out << acc;
        out << "show_labels=" << (s_showLabels ? 1 : 0) << "\n";
    }

    static void RefreshChecks() {
        if (s_pap3MenuId && s_itemShowLabels >= 0) {
            XPLMCheckMenuItem(s_pap3MenuId, s_itemShowLabels, s_showLabels ? xplm_Menu_Checked : xplm_Menu_Unchecked);
        }
    }

    static void MenuCallback(void *, void *ref) {
        const char *action = reinterpret_cast<const char *>(ref);
        if (!action) {
            return;
        }

        if (std::strcmp(action, "toggle_labels") == 0) {
            s_showLabels = !s_showLabels;
            SaveIniAll();
            RefreshChecks();
            return;
        }
    }

    void Initialize(XPLMMenuID mainMenuId) {
        s_mainMenuId = mainMenuId;

        LoadIni();

        const int pap3Idx = XPLMAppendMenuItem(s_mainMenuId, "PAP3", nullptr, 0);
        s_pap3MenuId = XPLMCreateMenu("PAP3", s_mainMenuId, pap3Idx, MenuCallback, nullptr);

        s_itemShowLabels = XPLMAppendMenuItem(s_pap3MenuId, "Display LCD Labels", (void *) "toggle_labels", 0);

        RefreshChecks();
    }

    bool GetShowLcdLabels() {
        return s_showLabels;
    }

    void SaveIni() {
        SaveIniAll();
    }

    void ReloadIni() {
        LoadIni();
        RefreshChecks();
    }

    void SetConnected(bool) {}

    std::string GetConfigPath() {
        return GetIniPath();
    }

    void Shutdown() {
        s_mainMenuId = s_pap3MenuId = nullptr;
        s_itemShowLabels = -1;
    }

} // namespace pap3menu
