#include "usbcontroller.h"

#include "appstate.h"

bool USBController::anyProfileReady() {
    for (auto &device : devices) {
        if (device->profileReady) {
            return true;
        }
    }

    return false;
}

void USBController::connectAllDevices() {
    AppState::getInstance()->executeAfter(0, [this]() {
        enumerateDevices();
    });
}

void USBController::disconnectAllDevices() {
    for (auto ptr : devices) {
        delete ptr;
    }
    devices.clear();
}
