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
void* getFMCHandle(int deviceIndex);
void* getFCUEfisHandle(int deviceIndex);

// Generic device functions via handle
bool device_connect(void* deviceHandle);
void device_disconnect(void* deviceHandle);
void device_update(void* deviceHandle);
void device_force_state_sync(void* deviceHandle);

// Joystick functions via handle
bool joystick_setVibration(void* joystickHandle, uint8_t vibration);
bool joystick_setLedBrightness(void* joystickHandle, uint8_t brightness);

// FMC functions via handle  
void fmc_clearDisplay(void* fmcHandle);
void fmc_unloadProfile(void* fmcHandle);
void fmc_showBackground(void* fmcHandle, int variant);
bool fmc_setLed(void* fmcHandle, int ledId, uint8_t value);
void fmc_setLedBrightness(void* fmcHandle, int ledId, uint8_t brightness);
bool fmc_writeData(void* fmcHandle, const uint8_t* data, int length);
void fmc_setFont(void* fmcHandle, int fontType);
void fmc_setFontUpdatingEnabled(void* fmcHandle, bool enabled);

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
