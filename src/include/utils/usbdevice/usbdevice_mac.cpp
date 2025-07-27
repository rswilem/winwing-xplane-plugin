#if APL
#include "usbdevice.h"
#include "config.h"
#include "appstate.h"
#include <XPLMUtilities.h>
#include <iostream>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>

USBDevice::USBDevice(HIDDeviceHandle aHidDevice, uint16_t aVendorId, uint16_t aProductId, std::string aVendorName, std::string aProductName)
: hidDevice(aHidDevice), vendorId(aVendorId), productId(aProductId), vendorName(aVendorName), productName(aProductName), connected(false) {}

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
    IOHIDDeviceRegisterInputReportCallback(hidDevice, inputBuffer, kInputReportSize, &USBDevice::InputReportCallback, this);
    if (hidDevice) {
        IOHIDDeviceScheduleWithRunLoop(hidDevice, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }
    
    connected = true;
    return true;
}

void USBDevice::InputReportCallback(void* context, IOReturn result, void* sender, IOHIDReportType type, uint32_t reportID, uint8_t* report, CFIndex reportLength) {
    if (result != kIOReturnSuccess || !reportLength) {
        return;
    }
    
    auto* self = static_cast<USBDevice*>(context);
    if (!self->connected) {
        return;
    }
    
    self->didReceiveData(reportID, report, (int)reportLength);
}

void USBDevice::update() {
    if (!connected) {
        return;
    }
}

void USBDevice::disconnect() {
    if (hidDevice) {
        IOHIDDeviceUnscheduleFromRunLoop(hidDevice, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }
    
    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }

    connected = false;
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
