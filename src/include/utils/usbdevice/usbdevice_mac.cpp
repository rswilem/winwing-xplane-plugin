#if APL
#include "appstate.h"
#include "config.h"
#include "usbdevice.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>
#include <iostream>
#include <XPLMUtilities.h>

USBDevice::USBDevice(HIDDeviceHandle aHidDevice, uint16_t aVendorId, uint16_t aProductId, std::string aVendorName, std::string aProductName) :
    hidDevice(aHidDevice), vendorId(aVendorId), productId(aProductId), vendorName(aVendorName), productName(aProductName), connected(false) {}

USBDevice::~USBDevice() {
    disconnect();
}

bool USBDevice::connect() {
    static const size_t kInputReportSize = 65;
    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }
    inputBuffer = new uint8_t[kInputReportSize];

    // Try to open the device - if it's already opened by the manager, this will return kIOReturnExclusiveAccess
    // or succeed if it wasn't opened yet. Either way, we'll have an open device.
    try {
        IOReturn result = IOHIDDeviceOpen(hidDevice, kIOHIDOptionsTypeNone);
        if (result != kIOReturnSuccess && result != kIOReturnExclusiveAccess) {
            throw std::system_error(std::make_error_code(std::errc::io_error), std::string("IOHIDDeviceOpen failed: ") + std::to_string(result));
        }
    } catch (const std::exception &ex) {
        debug("Failed to open HID device: %s\nError: %s\n", productName.c_str(), ex.what());
        delete[] inputBuffer;
        inputBuffer = nullptr;
        hidDevice = nullptr;
        return false;
    }

    IOHIDDeviceRegisterInputReportCallback(hidDevice, inputBuffer, kInputReportSize, &USBDevice::InputReportCallback, this);
    if (hidDevice) {
        IOHIDDeviceScheduleWithRunLoop(hidDevice, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }

    connected = true;
    return true;
}

void USBDevice::InputReportCallback(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength) {
    if (result != kIOReturnSuccess || !reportLength || !context) {
        return;
    }

    auto *self = static_cast<USBDevice *>(context);

    if (!self->connected || self->hidDevice != sender) {
        return;
    }

    try {
        InputEvent event;
        event.reportId = reportID;
        event.reportData.assign(report, report + reportLength);
        event.reportLength = (int) reportLength;

        self->processOnMainThread(event);
    } catch (const std::system_error &e) {
        // Silently ignore mutex errors that occur during shutdown
        return;
    }
}

void USBDevice::update() {
    if (!connected) {
        return;
    }

    processQueuedEvents();
}

void USBDevice::disconnect() {
    // Set connected to false first to prevent callback processing
    connected = false;

    if (hidDevice) {
        IOHIDDeviceUnscheduleFromRunLoop(hidDevice, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

        // Force run loop to process any remaining queued callbacks to drain them
        // This ensures any pending callbacks are processed while connected=false
        for (int i = 0; i < 10; i++) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, true);
        }

        IOHIDDeviceClose(hidDevice, kIOHIDOptionsTypeNone);
        hidDevice = nullptr;
    }

    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }
}

bool USBDevice::writeData(std::vector<uint8_t> data) {
    if (!hidDevice || !connected || data.empty()) {
        debug("HID device not open, not connected, or empty data\n");
        return false;
    }

    uint8_t reportID = data[0];
    IOReturn kr = IOHIDDeviceSetReport(hidDevice, kIOHIDReportTypeOutput, reportID, data.data(), data.size());
    if (kr != kIOReturnSuccess) {
        debug("IOHIDDeviceSetReport failed: %d\n", kr);
        return false;
    }
    return true;
}
#endif
