#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// General functions
void update();
void disconnectAll();

// Device enumeration and info
int enumerateDevices(char *buffer, int bufferLen);
int getDeviceCount();
const char* getDeviceName(int deviceIndex);
const char* getDeviceType(int deviceIndex);
uint16_t getDeviceProductId(int deviceIndex);
bool isDeviceConnected(int deviceIndex);

// Direct device handle access
void* getDeviceHandle(int deviceIndex);
void* getJoystickHandle(int deviceIndex);
void* getMCDUHandle(int deviceIndex);
void* getPFPHandle(int deviceIndex);

// Generic device functions via handle
bool device_connect(void* deviceHandle);
void device_disconnect(void* deviceHandle);
void device_update(void* deviceHandle);

// Joystick functions via handle
bool joystick_setVibration(void* joystickHandle, uint8_t vibration);
bool joystick_setLedBrightness(void* joystickHandle, uint8_t brightness);

// MCDU functions via handle  
void mcdu_clear2(void* mcduHandle, int displayId);
void mcdu_clear(void* mcduHandle);
bool mcdu_setLed(void* mcduHandle, int ledId, uint8_t value);
void mcdu_setLedBrightness(void* mcduHandle, int ledId, uint8_t brightness);

// PFP functions via handle
void pfp_clear2(void* pfpHandle, int displayId);
void pfp_clear(void* pfpHandle);
bool pfp_setLed(void* pfpHandle, int ledId, uint8_t value);
void pfp_setLedBrightness(void* pfpHandle, int ledId, uint8_t brightness);

void clearDatarefCache();
void setDatarefHexC(const char* ref, const uint8_t* hex, int len);

#ifdef __cplusplus
}
#endif
