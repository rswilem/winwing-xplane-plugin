#ifndef APPSTATE_H
#define APPSTATE_H

#include <string>
#include <vector>
#include <functional>
#include <chrono>

struct DelayedTask {
    std::string name;
    std::chrono::steady_clock::time_point runAt;
    std::function<void()> func;
};

enum class UpdateSpeed {
    SLOW,    // 5.0s - No profile loaded
    NORMAL,  // 0.4s - Aircraft in the air
    FAST     // 0.1s - Wheels on ground
};

class AppState {
private:
    AppState();
    ~AppState();
    
    static AppState* instance;
    std::vector<DelayedTask> taskQueue;
    void update();

public:
    static float Update(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon);
    
    bool pluginInitialized;
    UpdateSpeed updateSpeed;
    bool debuggingEnabled;
    bool hasActiveProfile;
    
    static AppState* getInstance();
    bool initialize();
    void deinitialize();
    
    void executeAfter(int milliseconds, std::function<void()> func);
    void executeAfterDebounced(std::string taskName, int milliseconds, std::function<void()> func);
};

#endif
