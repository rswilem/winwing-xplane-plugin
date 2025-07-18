#ifndef XPLM410
    #error This is made to be compiled against the XPLM410 SDK for XP12
#endif

#include "config.h"
#include <algorithm>
#include <XPLMDisplay.h>
#include <XPLMPlugin.h>
#include <XPLMMenus.h>
#include <XPLMProcessing.h>
#include <XPLMMenus.h>
#include <cmath>
#include <cstring>
#include "appstate.h"
#include "dataref.h"
#include "usbcontroller.h"
#include "product-mcdu.h"

#if IBM
#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    
    return TRUE;
}
#endif

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, long msg, void* params);
void menuAction(void* mRef, void* iRef);

PLUGIN_API int XPluginStart(char * name, char * sig, char * desc)
{
    strcpy(name, FRIENDLY_NAME);
    strcpy(sig, BUNDLE_ID);
    strcpy(desc, "Winwing X-Plane plugin");
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", 1);
    XPLMEnableFeature("XPLM_WANTS_DATAREF_NOTIFICATIONS", 1);
    
    int item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), FRIENDLY_NAME, nullptr, 1);
    XPLMMenuID id = XPLMCreateMenu(FRIENDLY_NAME, XPLMFindPluginsMenu(), item, menuAction, nullptr);
    XPLMAppendMenuItem(id, "Reload devices", (void *)"ActionReloadDevices", 0);
    
    debug("Plugin started (version %s)\n", VERSION);

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    AppState::getInstance()->deinitialize();
    debug("Plugin stopped\n");
}

PLUGIN_API int XPluginEnable(void) {
    XPluginReceiveMessage(0, XPLM_MSG_PLANE_LOADED, nullptr);
    
    return 1;
}

PLUGIN_API void XPluginDisable(void) {
    debug("Disabling plugin...\n");
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, long msg, void* params) {
    switch (msg) {
        case XPLM_MSG_PLANE_LOADED: {
            if ((intptr_t)params != 0) {
                // It was not the user's plane. Ignore.
                return;
            }
            
            
            AppState::getInstance()->initialize();
            break;
        }
            
        case XPLM_MSG_AIRPORT_LOADED: {
            for (auto device : USBController::getInstance()->devices) {
                if (auto mcdu = dynamic_cast<ProductMCDU*>(device)) {
                    mcdu->setLedBrightness(MCDULed::MENU, 0);
                }
            }
            break;
        }
            
        case XPLM_MSG_WILL_WRITE_PREFS:
            //AppState::getInstance()->saveState();
            break;
            
        default:
            break;
    }
}

void menuAction(void* mRef, void* iRef) {
    if (!strcmp((char *)iRef, "ActionReloadDevices")) {
        USBController::getInstance()->destroy();
        
        USBController::getInstance()->initialize();
    }
}

