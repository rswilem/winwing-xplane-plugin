#if IBM
#include "usbdevice.h"
#include "appstate.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <XPLMUtilities.h>

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
    
    // Start input reading thread
    connected = true;
    std::thread inputThread([this]() {
        uint8_t buffer[65];
        DWORD bytesRead;
        while (connected && hidDevice != INVALID_HANDLE_VALUE) {
            BOOL result = ReadFile(hidDevice, buffer, sizeof(buffer), &bytesRead, nullptr);
            if (result && bytesRead > 0 && connected) {
                InputReportCallback(this, bytesRead, buffer);
            } else if (!result) {
                DWORD error = GetLastError();
                if (error != ERROR_DEVICE_NOT_CONNECTED) {
                    debug("ReadFile failed with error: %s\n", error);
                }
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    inputThread.detach();
    
    return true;
}

void USBDevice::InputReportCallback(void* context, DWORD bytesRead, uint8_t* report) {
    auto* self = static_cast<USBDevice*>(context);
    if (!self || !self->connected) {
        return;
    }
    
    // Try-catch around the processOnMainThread call to handle mutex issues
    try {
        InputEvent event;
        event.reportId = report[0];
        event.reportData.assign(report, report + bytesRead);
        event.reportLength = (int)bytesRead;
        
        self->processOnMainThread(event);
    } catch (const std::system_error& e) {
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
    connected = false;
    
    // Give the reading thread a moment to see connected=false and exit
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    if (hidDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hidDevice);
        hidDevice = INVALID_HANDLE_VALUE;
    }
    
    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }
}

bool USBDevice::writeData(std::vector<uint8_t> data) {
    if (hidDevice == INVALID_HANDLE_VALUE || !connected || data.empty()) {
        debug("HID device not open, not connected, or empty data\n");
        return false;
    }
    
    DWORD bytesWritten;
    BOOL result = WriteFile(hidDevice, data.data(), (DWORD)data.size(), &bytesWritten, nullptr);
    if (!result || bytesWritten != data.size()) {
        debug("WriteFile failed: %s\n", GetLastError());
        return false;
    }
    return true;
}
#endif
