#include "usbdevice.h"
#include <XPLMUtilities.h>
#include "appstate.h"
#include "product-mcdu.h"
#include "product-pfp.h"
#include "product-ursa-minor-joystick.h"
#include "product-fcu-efis.h"
#include "product-pap3-mcp.h"

USBDevice *USBDevice::Device(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) {
    if (vendorId != WINWING_VENDOR_ID) {
        debug("Vendor ID mismatch: 0x%04X != 0x%04X\n", vendorId, WINWING_VENDOR_ID);
        return nullptr;
    }
    
    switch (productId) {
        case 0xBC27: // URSA MINOR Airline Joystick L
            return new ProductUrsaMinorJoystick(hidDevice, vendorId, productId, vendorName, productName);
            
        case 0xBB36: // MCDU-32 (Captain)
        case 0xBB3E: // MCDU-32 (First Officer)
        case 0xBB3A: // MCDU-32 (Observer)
#if MCDU_AS_PFP
            return new ProductPFP(hidDevice, vendorId, productId, vendorName, productName);
#else
            return new ProductMCDU(hidDevice, vendorId, productId, vendorName, productName);
#endif
            
        case 0xBB35: // PFP 3N (Captain)
        case 0xBB39: // PFP 3N (First Officer)
        case 0xBB3D: // PFP 3N (Observer)
        case 0xBB38: // PFP 4 (Captain)
        case 0xBB40: // PFP 4 (First Officer)
        case 0xBB3C: // PFP 4 (Observer)
        case 0xBB37: // PFP 7 (Captain)
        case 0xBB3F: // PFP 7 (First Officer)
        case 0xBB3B: // PFP 7 (Observer)
            return new ProductPFP(hidDevice, vendorId, productId, vendorName, productName);
            
        case 0xBB10: // FCU only
        case 0xBC1E: // FCU + EFIS-R
        case 0xBC1D: // FCU + EFIS-L
        case 0xBA01: // FCU + EFIS-L + EFIS-R
            return new ProductFCUEfis(hidDevice, vendorId, productId, vendorName, productName);
            
        case 0xBF0F: // PAP3
            return new ProductPAP3MCP(hidDevice, vendorId, productId, vendorName, productName);
        default:
            debug_force("Unknown Winwing device - vendorId: 0x%04X, productId: 0x%04X\n", vendorId, productId);
            return nullptr;
    }
}

const char *USBDevice::classIdentifier() {
    return "USBDevice (none)";
}

void USBDevice::didReceiveData(int reportId, uint8_t* report, int reportLength) {
    // noop, expect override
}

void USBDevice::processOnMainThread(const InputEvent& event) {
    std::lock_guard<std::mutex> lock(eventQueueMutex);
    eventQueue.push(event);
}

void USBDevice::processQueuedEvents() {
    std::lock_guard<std::mutex> lock(eventQueueMutex);
    while (!eventQueue.empty()) {
        InputEvent event = eventQueue.front();
        eventQueue.pop();
        
        didReceiveData(event.reportId, event.reportData.data(), event.reportLength);
    }
}
