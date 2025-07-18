#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

#if DEBUG
#define debug(format, ...) { char buffer[1024]; snprintf(buffer, sizeof(buffer), "[Winwing] " format, ##__VA_ARGS__); XPLMDebugString(buffer); printf("%s", buffer); }
#else
#define debug(format, ...) { char buffer[1024]; snprintf(buffer, sizeof(buffer), "[Winwing] " format, ##__VA_ARGS__); XPLMDebugString(buffer); }
#endif

#define PRODUCT_NAME "winwing"
#define FRIENDLY_NAME "Winwing"
#define VERSION "0.0.3"
#define ALL_PLUGINS_DIRECTORY "/Resources/plugins/"
#define PLUGIN_DIRECTORY (ALL_PLUGINS_DIRECTORY PRODUCT_NAME)
#define BUNDLE_ID "com.ramonster." PRODUCT_NAME

#define REFRESH_INTERVAL_SECONDS 0.4

#define WINWING_VENDOR_ID 0x4098
