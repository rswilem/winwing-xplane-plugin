#include "usbdevice.h"
#include <XPLMUtilities.h>
#include "appstate.h"
#include "product-mcdu.h"
#include "product-pfp.h"
#include "product-ursa-minor-joystick.h"

USBDevice *USBDevice::Device(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName) {
    debug_force("USBDevice::Device called with vendorId: 0x%04X, productId: 0x%04X, vendor: %s, product: %s\n", vendorId, productId, vendorName.c_str(), productName.c_str());
    
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
            return new ProductPFP(hidDevice, vendorId, productId, vendorName, productName);

        case 0xBB35: // PFP 3N (Captain)
        case 0xBB39: // PFP 3N (First Officer)
        case 0xBB3D: // PFP 3N (Observer)
        case 0xBC1D: // PFP 4
        case 0xBA01: // PFP 7
            return new ProductPFP(hidDevice, vendorId, productId, vendorName, productName);
            
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
