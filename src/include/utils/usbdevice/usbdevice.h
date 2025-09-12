#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "config.h"

#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#if APL
#include <IOKit/hid/IOHIDLib.h>
typedef IOHIDDeviceRef HIDDeviceHandle;
#elif IBM
#include <hidsdi.h>
#include <setupapi.h>
#include <windows.h>
typedef HANDLE HIDDeviceHandle;
#elif LIN
#include <linux/hidraw.h>
typedef int HIDDeviceHandle;
#endif

struct InputEvent {
        int reportId;
        std::vector<uint8_t> reportData;
        int reportLength;
};

class USBDevice {
    private:
        uint8_t *inputBuffer = nullptr;
        std::queue<InputEvent> eventQueue;
        std::mutex eventQueueMutex;

        void processQueuedEvents();

#if APL
        static void InputReportCallback(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength);
#elif IBM
        static void InputReportCallback(void *context, DWORD bytesRead, uint8_t *report);
#elif LIN
        static void InputReportCallback(void *context, int bytesRead, uint8_t *report);
#endif

    public:
        USBDevice(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        virtual ~USBDevice();

        HIDDeviceHandle hidDevice;
        bool connected = false;
        bool profileReady = false;
        uint16_t vendorId;
        uint16_t productId;
        std::string vendorName;
        std::string productName;

        virtual const char *classIdentifier();
        virtual bool connect();
        virtual void disconnect();
        virtual void update();
        virtual void didReceiveData(int reportId, uint8_t *report, int reportLength);

        void processOnMainThread(const InputEvent &event);

        bool writeData(std::vector<uint8_t> data);

        static USBDevice *Device(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
};

#endif
