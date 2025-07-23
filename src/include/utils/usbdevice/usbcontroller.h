#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H

#include <vector>
#include "usbdevice.h"

#if APL
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>
typedef IOHIDManagerRef HIDManagerHandle;
typedef IOHIDDeviceRef HIDDeviceHandle;
#elif IBM
#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <dbt.h>
typedef void* HIDManagerHandle;  // Not used in polling approach
typedef HANDLE HIDDeviceHandle;
#elif LIN
#include <libudev.h>
typedef struct udev_monitor* HIDManagerHandle;
typedef int HIDDeviceHandle;
#endif

class USBController {
private:
    HIDManagerHandle hidManager;
    
    USBController();
    ~USBController();
    static USBController* instance;
    
#if APL
    static void DeviceAddedCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device);
    static void DeviceRemovedCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device);
#elif IBM
    void checkForDeviceChanges();
    void enumerateDevices();
#elif LIN
    static void DeviceAddedCallback(void *context, struct udev_device *device);
    static void DeviceRemovedCallback(void *context, struct udev_device *device);
    void monitorDevices();
    void enumerateDevices();
#endif

public:
    std::vector<USBDevice *> devices;
    static USBController* getInstance();
    void initialize();
    void destroy();
    
    bool allProfilesReady();
};

#endif
