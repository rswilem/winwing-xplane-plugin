#include "usbdevice.h"
#include <XPLMUtilities.h>
#include "appstate.h"
#include "product-fmc.h"
#include "product-ursa-minor-joystick.h"
#include "product-fcu-efis.h"
#include "pap3_device.h"

USBDevice *USBDevice::Device(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName)
{
    if (vendorId != WINWING_VENDOR_ID)
    {
        debug("Vendor ID mismatch: 0x%04X != 0x%04X\n", vendorId, WINWING_VENDOR_ID);
        return nullptr;
    }

    switch (productId) {
        case 0xBC27: // URSA MINOR Airline Joystick L
        case 0xBC28: // URSA MINOR Airline Joystick R
            return new ProductUrsaMinorJoystick(hidDevice, vendorId, productId, vendorName, productName);

        case 0xBB36: // MCDU-32 (Captain)
        case 0xBB3E: // MCDU-32 (First Officer)
        case 0xBB3A: { // MCDU-32 (Observer)
            constexpr uint8_t identifierByte = 0x32;
            return new ProductFMC(hidDevice, vendorId, productId, vendorName, productName, FMCHardwareType::HARDWARE_MCDU, identifierByte);
        }

        case 0xBB35: // PFP 3N (Captain)
        case 0xBB39: // PFP 3N (First Officer)
        case 0xBB3D: { // PFP 3N (Observer)
            constexpr uint8_t identifierByte = 0x31;
            return new ProductFMC(hidDevice, vendorId, productId, vendorName, productName, FMCHardwareType::HARDWARE_PFP3N, identifierByte);
        }

        case 0xBB38: // PFP 4 (Captain)
        case 0xBB40: // PFP 4 (First Officer)
        case 0xBB3C: { // PFP 4 (Observer)
            constexpr uint8_t identifierByte = 0x31;
            return new ProductFMC(hidDevice, vendorId, productId, vendorName, productName, FMCHardwareType::HARDWARE_PFP4, identifierByte);
        }

        case 0xBB37: // PFP 7 (Captain)
        case 0xBB3F: // PFP 7 (First Officer)
        case 0xBB3B: { // PFP 7 (Observer)
            constexpr uint8_t identifierByte = 0x31;
            return new ProductFMC(hidDevice, vendorId, productId, vendorName, productName, FMCHardwareType::HARDWARE_PFP7, identifierByte);
        }

        case 0xBB10: // FCU only
        case 0xBC1E: // FCU + EFIS-R
        case 0xBC1D: // FCU + EFIS-L
        case 0xBA01: // FCU + EFIS-L + EFIS-R
            return new ProductFCUEfis(hidDevice, vendorId, productId, vendorName, productName);

        case 0xBF0F: // PAP3
            debug_force("Detected PAP3 MCP device - vendorId: 0x%04X, productId: 0x%04X\n", vendorId, productId);
            return new pap3::device::PAP3Device(hidDevice, vendorId, productId, vendorName, productName);

        default:
            debug_force("Unknown Winwing device - vendorId: 0x%04X, productId: 0x%04X\n", vendorId, productId);
            return nullptr;
    }
}

const char *USBDevice::classIdentifier() {
    return "USBDevice (none)";
}

void USBDevice::didReceiveData(int reportId, uint8_t *report, int reportLength) {
    // noop, expect override
}

void USBDevice::processOnMainThread(const InputEvent &event) {
    std::lock_guard<std::mutex> lock(eventQueueMutex);
    eventQueue.push(event);
}

void USBDevice::processQueuedEvents() {
    std::lock_guard<std::mutex> lock(eventQueueMutex);
    while (!eventQueue.empty())
    {
        InputEvent event = eventQueue.front();
        eventQueue.pop();

        didReceiveData(event.reportId, event.reportData.data(), event.reportLength);
    }
}
