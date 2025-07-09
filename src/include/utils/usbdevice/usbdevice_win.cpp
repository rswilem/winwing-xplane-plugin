#if IBM
#include "usbdevice.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>

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
                    std::cerr << "ReadFile failed with error: " << error << std::endl;
                }
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    inputThread.detach();
    
    printf("HID device connected.\n");
    return true;
}

void USBDevice::InputReportCallback(void* context, DWORD bytesRead, uint8_t* report) {
    auto* self = static_cast<USBDevice*>(context);
    if (!self->connected) {
        return;
    }
    
    // Windows HID reports typically include report ID as first byte
    uint8_t reportId = report[0];
    self->didReceiveData(reportId, report, (int)bytesRead);
}

void USBDevice::update() {
    if (!connected) {
        return;
    }
}

void USBDevice::disconnect() {
    connected = false;
    
    if (hidDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hidDevice);
        hidDevice = INVALID_HANDLE_VALUE;
    }
    
    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }
    
    printf("HID device disconnected.\n");
}

bool USBDevice::writeData(std::vector<uint8_t> data) {
    if (hidDevice == INVALID_HANDLE_VALUE || !connected) {
        std::cerr << "HID device not open\n";
        return false;
    }
    
    DWORD bytesWritten;
    BOOL result = WriteFile(hidDevice, data.data(), (DWORD)data.size(), &bytesWritten, nullptr);
    if (!result || bytesWritten != data.size()) {
        std::cerr << "WriteFile failed with error: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}
#endif
