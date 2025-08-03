#include "usbcontroller.h"
#include "appstate.h"

bool USBController::allProfilesReady() {
    for (auto& device : devices) {
        if (!device->profileReady) {
            return false;
        }
    }
    
    return true;
}


void USBController::connectAllDevices() {
    AppState::getInstance()->executeAfter(0, [this](){
        enumerateDevices();
    });
}

void USBController::disconnectAllDevices() {
    for (auto ptr : devices) {
        delete ptr;
    }
    devices.clear();
}
