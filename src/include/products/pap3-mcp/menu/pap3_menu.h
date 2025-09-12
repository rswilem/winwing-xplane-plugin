#pragma once
#include "XPLMMenus.h"
#include <string>

namespace pap3menu {

// Initialize the PAP3 submenu under your plugin’s main menu.
void Initialize(XPLMMenuID mainMenuId);

// Returns the absolute INI path used by this module (optional).
std::string GetConfigPath();

// Read current “Display LCD Labels” state.
bool GetShowLcdLabels();

// Force-save (optional).
void SaveIni();

// Reload (optional).
void ReloadIni();

// Optional: show/hide menu when device connects.
void SetConnected(bool connected);

// Optional: teardown
void Shutdown();

} // namespace pap3menu