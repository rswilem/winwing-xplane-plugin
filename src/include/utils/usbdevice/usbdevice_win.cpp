#if IBM
#include "appstate.h"
#include "config.h"
#include "usbdevice.h"

#include <chrono>
#include <hidsdi.h>
#include <iostream>
#include <setupapi.h>
#include <thread>
#include <windows.h>
#include <XPLMUtilities.h>

extern "C" {
#include <hidpi.h>
}

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

    PHIDP_PREPARSED_DATA preparsedData = nullptr;
    if (HidD_GetPreparsedData(hidDevice, &preparsedData)) {
        HIDP_CAPS caps;
        if (HidP_GetCaps(preparsedData, &caps) == HIDP_STATUS_SUCCESS) {
            outputReportByteLength = caps.OutputReportByteLength;
            debug("Output report byte length: %u\n", outputReportByteLength);
        } else {
            debug_force("Failed to get HID capabilities\n");
        }
        HidD_FreePreparsedData(preparsedData);
    } else {
        debug_force("Failed to get preparsed data\n");
    }

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
                if (error == ERROR_DEVICE_NOT_CONNECTED) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    });
    inputThread.detach();

    writeThreadRunning = true;
    writeThread = std::thread(&USBDevice::writeThreadLoop, this);

    return true;
}

void USBDevice::InputReportCallback(void *context, DWORD bytesRead, uint8_t *report) {
    auto *self = static_cast<USBDevice *>(context);
    if (!self || !self->connected || !report || bytesRead == 0) {
        return;
    }

    if (self->hidDevice == INVALID_HANDLE_VALUE) {
        return;
    }

    try {
        InputEvent event;
        event.reportId = report[0];
        event.reportData.assign(report, report + bytesRead);
        event.reportLength = (int) bytesRead;

        self->processOnMainThread(event);
    } catch (const std::system_error &e) {
        return;
    } catch (...) {
        debug_force("Unexpected exception in InputReportCallback\n");
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
    // Wait for write queue to drain before disconnecting
    while (cachedWriteQueueSize.load() > 0 && writeThreadRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    writeThreadRunning = false;
    writeQueueCV.notify_all();
    if (writeThread.joinable()) {
        writeThread.join();
    }

    if (hidDevice != INVALID_HANDLE_VALUE) {
        // Give input thread time to exit
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        CloseHandle(hidDevice);
        hidDevice = INVALID_HANDLE_VALUE;
    }

    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }
}

void USBDevice::forceStateSync() {
    // noop, code does not use partial data
}

bool USBDevice::writeData(std::vector<uint8_t> data) {
    if (hidDevice == INVALID_HANDLE_VALUE || !connected || data.empty()) {
        debug_force("HID device not open, not connected, or empty data\n");
        return false;
    }

    if (data.size() > 1024) {
        debug_force("Data size too large: %zu bytes\n", data.size());
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(writeQueueMutex);
        writeQueue.push(std::move(data));
        cachedWriteQueueSize.store(writeQueue.size());
    }
    writeQueueCV.notify_one();

    return true;
}

void USBDevice::writeThreadLoop() {
    while (writeThreadRunning) {
        std::vector<uint8_t> data;

        {
            std::unique_lock<std::mutex> lock(writeQueueMutex);
            writeQueueCV.wait(lock, [this] {
                return !writeQueue.empty() || !writeThreadRunning;
            });

            if (!writeThreadRunning) {
                break;
            }

            if (!writeQueue.empty()) {
                data = std::move(writeQueue.front());
                writeQueue.pop();
                cachedWriteQueueSize.store(writeQueue.size());
            }
        }

        if (!data.empty() && hidDevice != INVALID_HANDLE_VALUE && connected) {
            std::vector<uint8_t> paddedData = data;
            if (outputReportByteLength > 0 && paddedData.size() < outputReportByteLength) {
                paddedData.resize(outputReportByteLength, 0);
            }

            DWORD bytesWritten;
            if (!WriteFile(hidDevice, paddedData.data(), (DWORD) paddedData.size(), &bytesWritten, nullptr)) {
                DWORD error = GetLastError();
                debug_force("WriteFile failed: %lu\n", error);
            }
        }
    }
}
#endif
