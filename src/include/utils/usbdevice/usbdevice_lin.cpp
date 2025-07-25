#if LIN
#include "usbdevice.h"
#include "appstate.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
#include <errno.h>
#include <cstring>
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
    
    connected = true;
    std::thread inputThread([this]() {
        uint8_t buffer[65];
        while (connected && hidDevice >= 0) {
            ssize_t bytesRead = read(hidDevice, buffer, sizeof(buffer));
            if (bytesRead > 0 && connected) {
                InputReportCallback(this, (int)bytesRead, buffer);
            } else if (bytesRead < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    debug_force("ReadFile failed with error: %d\n", errno);
                    break;
                }
            } else if (bytesRead == 0) {
                // Device disconnected
                debug_force("Device disconnected\n");
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        debug_force("Input thread exiting\n");
    });
    inputThread.detach();
    
    return true;
}

void USBDevice::InputReportCallback(void* context, int bytesRead, uint8_t* report) {
    auto* self = static_cast<USBDevice*>(context);
    if (!self || !self->connected || !report || bytesRead <= 0) {
        return;
    }
    
    // Add bounds checking
    if (bytesRead > 65) {
        debug_force("Warning: Received oversized report (%d bytes), truncating\n", bytesRead);
        bytesRead = 65;
    }
    
    // Debug input reports
    debug_force("Received HID report: %d bytes, first 8 bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n", 
          bytesRead, 
          bytesRead > 0 ? report[0] : 0,
          bytesRead > 1 ? report[1] : 0,
          bytesRead > 2 ? report[2] : 0,
          bytesRead > 3 ? report[3] : 0,
          bytesRead > 4 ? report[4] : 0,
          bytesRead > 5 ? report[5] : 0,
          bytesRead > 6 ? report[6] : 0,
          bytesRead > 7 ? report[7] : 0);
    
    // Linux hidraw reports typically include report ID as first byte
    uint8_t reportId = report[0];
    
    try {
        // On Linux hidraw, the report ID is included in the data, but the PFP expects
        // the report data without the report ID (like on macOS). So we pass report+1
        // and reduce the length by 1 to skip the report ID byte.
        if (bytesRead > 0) {
            self->didReceiveData(reportId, report + 1, bytesRead - 1);
        }
    } catch (...) {
        debug_force("Exception in didReceiveData, report ID: %d, length: %d\n", reportId, bytesRead);
    }
}

void USBDevice::update() {
    if (!connected) {
        return;
    }
}

void USBDevice::disconnect() {
    debug_force("Disconnecting device\n");
    connected = false;
    
    // Give input thread time to exit
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    if (hidDevice >= 0) {
        close(hidDevice);
        hidDevice = -1;
    }
    
    if (inputBuffer) {
        delete[] inputBuffer;
        inputBuffer = nullptr;
    }
    
    debug_force("Device disconnected\n");
}

bool USBDevice::writeData(std::vector<uint8_t> data) {
    if (hidDevice < 0 || !connected) {
        debug_force("HID device not open or not connected\n");
        return false;
    }
    
    if (data.empty()) {
        debug_force("Warning: Attempting to write empty data\n");
        return false;
    }
    
    // Add debug output for initialization and display data
    if (data.size() > 0) {
        if (data[0] == 0xf0) {
            debug_force("Writing PFP initialization command, length: %zu, cmd: 0x%02X\n", data.size(), data.size() > 3 ? data[3] : 0);
        } else if (data[0] == 0xf2) {
            debug_force("Writing PFP display data, length: %zu\n", data.size());
        } else if (data[0] == 0x02) {
            debug_force("Writing PFP LED command, length: %zu\n", data.size());
        } else {
            debug_force("Writing unknown PFP command: 0x%02X, length: %zu\n", data[0], data.size());
        }
        
        // Debug: Print first 16 bytes of data
        debug_force("Data bytes: ");
        for (size_t i = 0; i < std::min(data.size(), (size_t)16); i++) {
            debug_force("%02X ", data[i]);
        }
        debug_force("\n");
    }
    
    // Some Linux systems might require different handling for different report types
    // Let's first try the standard write() approach
    ssize_t bytesWritten = write(hidDevice, data.data(), data.size());
    if (bytesWritten != (ssize_t)data.size()) {
        debug_force("Standard write failed: expected %zu bytes, wrote %zd bytes, error: %d (%s)\n", 
              data.size(), bytesWritten, errno, strerror(errno));
        
        // For PFP devices on Linux, try using feature reports for initialization commands
        if (data[0] == 0xf0) {
            debug_force("Trying HIDIOCSFEATURE for initialization command\n");
            if (ioctl(hidDevice, HIDIOCSFEATURE(data.size()), data.data()) >= 0) {
                debug_force("HIDIOCSFEATURE succeeded for initialization\n");
                return true;
            } else {
                debug_force("HIDIOCSFEATURE failed: %d (%s)\n", errno, strerror(errno));
            }
        }
        
        return false;
    }
    
    debug_force("Successfully wrote %zd bytes using standard write()\n", bytesWritten);
    return true;
}
#endif
