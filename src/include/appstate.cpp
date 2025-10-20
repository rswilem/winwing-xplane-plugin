#include "appstate.h"

#include "config.h"
#include "dataref.h"
#include "usbcontroller.h"
#include "usbdevice.h"

#include <fstream>
#include <XPLMProcessing.h>

AppState *AppState::instance = nullptr;

AppState::AppState() {
    pluginInitialized = false;
    debuggingEnabled = false;
}

AppState::~AppState() {
    instance = nullptr;
}

AppState *AppState::getInstance() {
    if (instance == nullptr) {
        instance = new AppState();
    }

    return instance;
}

bool AppState::initialize() {
    if (pluginInitialized) {
        return false;
    }

    XPLMRegisterFlightLoopCallback(AppState::Update, REFRESH_INTERVAL_SECONDS_FAST, nullptr);

    pluginInitialized = true;

    debug_force("Plugin initialized.\n");
    return true;
}

void AppState::deinitialize() {
    if (!pluginInitialized) {
        return;
    }

    debug_force("Plugin deinitializing...\n");
    XPLMUnregisterFlightLoopCallback(AppState::Update, nullptr);

    USBController::getInstance()->destroy();

    Dataref::getInstance()->destroyAllBindings();

    pluginInitialized = false;
    instance = nullptr;
    taskQueue.clear();
}

float AppState::Update(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon) {
    auto appstate = AppState::getInstance();

    appstate->update();

    if (!USBController::getInstance()->allProfilesReady()) {
        return REFRESH_INTERVAL_SECONDS_SLOW;
    }

    return REFRESH_INTERVAL_SECONDS_FAST;
}

void AppState::update() {
    auto now = std::chrono::steady_clock::now();
    for (auto &task : taskQueue) {
        if (now >= task.runAt && task.func) {
            task.func();
        }
    }

    taskQueue.erase(std::remove_if(taskQueue.begin(), taskQueue.end(), [&](auto &task) {
                        return now >= task.runAt;
                    }),
                    taskQueue.end());

    if (!pluginInitialized) {
        return;
    }

    Dataref::getInstance()->update();

    for (auto *device : USBController::getInstance()->devices) {
        device->update();
    }
}

void AppState::executeAfter(int milliseconds, std::function<void()> func) {
    taskQueue.push_back({"",
                         std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds),
                         func});
}

void AppState::executeAfterDebounced(std::string taskName, int milliseconds, std::function<void()> func) {
    auto now = std::chrono::steady_clock::now();
    auto it = std::find_if(taskQueue.begin(), taskQueue.end(), [&](const DelayedTask &t) {
        return t.name == taskName;
    });

    if (it != taskQueue.end()) {
        it->runAt = now + std::chrono::milliseconds(milliseconds);
        it->func = func;
    } else {
        taskQueue.push_back({taskName, now + std::chrono::milliseconds(milliseconds), func});
    }
}
