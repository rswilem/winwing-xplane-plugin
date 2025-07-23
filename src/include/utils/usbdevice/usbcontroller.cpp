#include "usbcontroller.h"

bool USBController::allProfilesReady() {
    for (auto& device : devices) {
        if (!device->profileReady) {
            return false;
        }
    }
    
    return true;
}
