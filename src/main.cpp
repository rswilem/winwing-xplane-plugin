#ifndef XPLM410
#error This is made to be compiled against the XPLM410 SDK for XP12
#endif

#include "appstate.h"
#include "config.h"
#include "dataref.h"
#include "usbcontroller.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <XPLMDisplay.h>
#include <XPLMMenus.h>
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>

#if IBM
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
#endif

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, long msg, void *params);
void menuAction(void *mRef, void *iRef);

XPLMMenuID mainMenuId;
int debugLoggingMenuItemIndex;

PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
    strcpy(name, FRIENDLY_NAME);
    strcpy(sig, BUNDLE_ID);
    strcpy(desc, "Winwing X-Plane plugin");
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", 1);
    XPLMEnableFeature("XPLM_WANTS_DATAREF_NOTIFICATIONS", 1);

    int item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), FRIENDLY_NAME, nullptr, 1);
    mainMenuId = XPLMCreateMenu(FRIENDLY_NAME, XPLMFindPluginsMenu(), item, menuAction, nullptr);
    XPLMAppendMenuItem(mainMenuId, "Reload devices", (void *) "ActionReloadDevices", 0);
    debugLoggingMenuItemIndex = XPLMAppendMenuItem(mainMenuId, "Enable debug logging", (void *) "ActionToggleDebugLogging", 0);
    XPLMCheckMenuItem(mainMenuId, debugLoggingMenuItemIndex, xplm_Menu_Unchecked);

    debug_force("Plugin started (version %s)\n", VERSION);

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    AppState::getInstance()->deinitialize();
    debug_force("Plugin stopped\n");
}

PLUGIN_API int XPluginEnable(void) {
    XPluginReceiveMessage(0, XPLM_MSG_PLANE_LOADED, nullptr);

    return 1;
}

PLUGIN_API void XPluginDisable(void) {
    debug_force("Disabling plugin...\n");
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, long msg, void *params) {
    switch (msg) {
        case XPLM_MSG_PLANE_LOADED: {
            if ((intptr_t) params != 0) {
                // It was not the user's plane. Ignore.
                return;
            }

            AppState::getInstance()->initialize();
            USBController::getInstance()->connectAllDevices();
            break;
        }

        case XPLM_MSG_PLANE_UNLOADED: {
            if ((intptr_t) params != 0) {
                // It was not the user's plane. Ignore.
                return;
            }

            USBController::getInstance()->disconnectAllDevices();
            break;
        }

        case XPLM_MSG_AIRPORT_LOADED: {
            break;
        }

        case XPLM_MSG_WILL_WRITE_PREFS:
            // AppState::getInstance()->saveState();
            break;

        default:
            break;
    }
}

void menuAction(void *mRef, void *iRef) {
    if (!strcmp((char *) iRef, "ActionReloadDevices")) {
        debug_force("Reloading devices...\n");
        USBController::getInstance()->disconnectAllDevices();
        USBController::getInstance()->connectAllDevices();
    } else if (!strcmp((char *) iRef, "ActionToggleDebugLogging")) {
        XPLMMenuCheck currentState;
        XPLMCheckMenuItemState(mainMenuId, debugLoggingMenuItemIndex, &currentState);

        bool debugLoggingEnabled = (currentState != xplm_Menu_Checked);
        XPLMSetMenuItemName(mainMenuId, debugLoggingMenuItemIndex, debugLoggingEnabled ? "Disable debug logging" : "Enable debug logging", 0);
        XPLMCheckMenuItem(mainMenuId, debugLoggingMenuItemIndex, debugLoggingEnabled ? xplm_Menu_Checked : xplm_Menu_Unchecked);
        AppState::getInstance()->debuggingEnabled = debugLoggingEnabled;

        if (debugLoggingEnabled) {
            debug_force("Debug logging was enabled. Currently connected devices (%lu):\n", USBController::getInstance()->devices.size());

            for (auto &device : USBController::getInstance()->devices) {
                debug_force("- (vendorId: 0x%04X, productId: 0x%04X, handler: %s) %s\n", device->vendorId, device->productId, device->classIdentifier(), device->productName.c_str());
            }
        } else {
            debug_force("Debug logging was disabled.\n");
        }
    }
}
