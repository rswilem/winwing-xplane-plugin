#include "appstate.h"
#include <fstream>
#include <XPLMProcessing.h>
#include "config.h"
#include "dataref.h"
#include "usbcontroller.h"
#include "usbdevice.h"

AppState* AppState::instance = nullptr;

AppState::AppState() {
    pluginInitialized = false;
}

AppState::~AppState() {
    instance = nullptr;
}

AppState* AppState::getInstance() {
    if (instance == nullptr) {
        instance = new AppState();
    }
    
    return instance;
}

bool AppState::initialize() {
    if (pluginInitialized) {
        return false;
    }

    USBController::getInstance()->initialize();

    XPLMRegisterFlightLoopCallback(AppState::Update, REFRESH_INTERVAL_SECONDS, nullptr);
    
    pluginInitialized = true;
    
    debug("Plugin initialized, found %lu devices.\n", USBController::getInstance()->devices.size());
    return true;
}

void AppState::deinitialize() {
    if (!pluginInitialized) {
        return;
    }
    
    debug("Plugin deinitializing...\n");
    XPLMUnregisterFlightLoopCallback(AppState::Update, nullptr);
    
    USBController::getInstance()->destroy();
    
    Dataref::getInstance()->destroyAllBindings();

    pluginInitialized = false;
    instance = nullptr;
    taskQueue.clear();
}

float AppState::Update(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon) {
    AppState::getInstance()->update();
    return REFRESH_INTERVAL_SECONDS;
}

void AppState::update() {
    auto now = std::chrono::steady_clock::now();
    taskQueue.erase(std::remove_if(taskQueue.begin(), taskQueue.end(), [&](DelayedTask &task) {
        if (now >= task.runAt) {
            task.func();
            return true;
        }
        return false;
    }), taskQueue.end());
    
    if (!pluginInitialized) {
        return;
    }
    
    Dataref::getInstance()->update();
    
    for (auto device : USBController::getInstance()->devices) {
        device->update();
    }
}

void AppState::executeAfter(int milliseconds, std::function<void()> func) {
    taskQueue.push_back({
        "",
        std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds),
        func
    });
}

void AppState::executeAfterDebounced(std::string taskName, int milliseconds, std::function<void()> func) {
    auto now = std::chrono::steady_clock::now();
    auto it = std::find_if(taskQueue.begin(), taskQueue.end(), [&](const DelayedTask& t) {
        return t.name == taskName;
    });

    if (it != taskQueue.end()) {
        it->runAt = now + std::chrono::milliseconds(milliseconds);
        it->func = func;
    } else {
        taskQueue.push_back({taskName, now + std::chrono::milliseconds(milliseconds), func});
    }
}
