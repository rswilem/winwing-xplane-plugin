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

    // Try to open the device - if it's already opened by the manager, this will return kIOReturnExclusiveAccess
    // or succeed if it wasn't opened yet. Either way, we'll have an open device.
    try {
        IOReturn result = IOHIDDeviceOpen(hidDevice, kIOHIDOptionsTypeNone);
        if (result != kIOReturnSuccess && result != kIOReturnExclusiveAccess) {
            throw std::system_error(std::make_error_code(std::errc::io_error), std::string("IOHIDDeviceOpen failed: ") + std::to_string(result));
        }
    } catch (const std::exception &ex) {
        debug("Failed to open HID device: %s\nError: %s\n", productName.c_str(), ex.what());
        hidDevice = nullptr;
        return false;
    }

    if (hidDevice) {
        hidQueue = IOHIDQueueCreate(kCFAllocatorDefault, hidDevice, kInputReportSize, 0);

        CFArrayRef elements = IOHIDDeviceCopyMatchingElements(hidDevice, nullptr, kIOHIDOptionsTypeNone);
        CFIndex count = CFArrayGetCount(elements);
        for (CFIndex i = 0; i < count; i++) {
            IOHIDElementRef elem = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, i);
            if (IOHIDElementGetType(elem) == kIOHIDElementTypeInput_Button) {
                IOHIDQueueAddElement(hidQueue, elem);
            }
        }
        CFRelease(elements);

        IOHIDQueueScheduleWithRunLoop(hidQueue, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        IOHIDQueueStart(hidQueue);
    }

    connected = true;
    return true;
}

void USBDevice::update() {
    if (!connected) {
        return;
    }

    if (!hidQueue) {
        return;
    }

    IOHIDValueRef value = nullptr;
    while ((value = IOHIDQueueCopyNextValue(hidQueue))) {
        handleHIDValue(value);
        CFRelease(value);
    }
}

void USBDevice::disconnect() {
    connected = false;

    if (hidDevice) {
        IOHIDQueueStop(hidQueue);
        IOHIDQueueUnscheduleFromRunLoop(hidQueue, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        hidQueue = nullptr;

        // Force run loop to process any remaining queued callbacks to drain them
        // This ensures any pending callbacks are processed while connected=false
        for (int i = 0; i < 10; i++) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, true);
        }

        IOHIDDeviceClose(hidDevice, kIOHIDOptionsTypeNone);
        hidDevice = nullptr;
    }
}

void USBDevice::forceStateSync() {
    if (!connected || !hidDevice) {
        return;
    }
    
    CFArrayRef elements = IOHIDDeviceCopyMatchingElements(hidDevice, nullptr, 0);
    for (CFIndex i = 0; i < CFArrayGetCount(elements); i++) {
        IOHIDElementRef element = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, i);
        if (IOHIDElementGetType(element) != kIOHIDElementTypeInput_Button) {
            continue;
        }

        IOHIDValueRef value = nullptr;
        if (IOHIDDeviceGetValue(hidDevice, element, &value) == kIOReturnSuccess) {
            handleHIDValue(value);
        }
    }
    
    CFRelease(elements);
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

void USBDevice::handleHIDValue(IOHIDValueRef value) {
    IOHIDElementRef element = IOHIDValueGetElement(value);
    if (!element) {
        return;
    }

    uint32_t reportID = IOHIDElementGetReportID(element);
    if (reportID != 1) {
        return;
    }

    uint32_t hardwareButtonIndex = IOHIDElementGetUsage(element);
    if (hardwareButtonIndex <= 0 || hardwareButtonIndex > UINT16_MAX) {
        return;
    }

    CFIndex len = IOHIDValueGetLength(value);
    if (len == 0) {
        return;
    }
    const uint8_t *data = static_cast<const uint8_t *>(IOHIDValueGetBytePtr(value));

    bool pressed = data[0] == 1;
    didReceiveButton(hardwareButtonIndex - 1, pressed);
}
#endif
