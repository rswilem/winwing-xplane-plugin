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
void* getFCUEfisHandle(int deviceIndex);

// Generic device functions via handle
bool device_connect(void* deviceHandle);
void device_disconnect(void* deviceHandle);
void device_update(void* deviceHandle);

// Joystick functions via handle
bool joystick_setVibration(void* joystickHandle, uint8_t vibration);
bool joystick_setLedBrightness(void* joystickHandle, uint8_t brightness);

// MCDU functions via handle  
void mcdu_clearDisplay(void* mcduHandle);
void mcdu_showBackground(void* mcduHandle, int variant);
bool mcdu_setLed(void* mcduHandle, int ledId, uint8_t value);
void mcdu_setLedBrightness(void* mcduHandle, int ledId, uint8_t brightness);

// PFP functions via handle
void pfp_clearDisplay(void* pfpHandle);
void pfp_showBackground(void* pfpHandle, int variant);
bool pfp_setLed(void* pfpHandle, int ledId, uint8_t value);
void pfp_setLedBrightness(void* pfpHandle, int ledId, uint8_t brightness);

// FCU-EFIS functions via handle
void fcuefis_clear(void* fcuefisHandle);
void fcuefis_efisRightClear(void* fcuefisHandle);
void fcuefis_efisLeftClear(void* fcuefisHandle);
bool fcuefis_setLed(void* fcuefisHandle, int ledId, uint8_t value);
void fcuefis_setLedBrightness(void* fcuefisHandle, int ledId, uint8_t brightness);
void fcuefis_testDisplay(void* fcuefisHandle, const char* testType);
void fcuefis_efisRightTestDisplay(void* fcuefisHandle, const char* testType);
void fcuefis_efisLeftTestDisplay(void* fcuefisHandle, const char* testType);

void clearDatarefCache();
void setDatarefHexC(const char* ref, const uint8_t* hex, int len);
void setDatarefFloat(const char* ref, float value);
void setDatarefInt(const char* ref, int value);
void setDatarefFloatVector(const char* ref, const float* values, int count);
void setDatarefFloatVectorRepeated(const char* ref, float value, int count);
void setDatarefIntVector(const char* ref, const int* values, int count);

#ifdef __cplusplus
}
#endif
