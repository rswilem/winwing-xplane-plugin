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
    bool debuggingEnabled;
    
    static AppState* getInstance();
    bool initialize();
    void deinitialize();
    
    void executeAfter(int milliseconds, std::function<void()> func);
    void executeAfterDebounced(std::string taskName, int milliseconds, std::function<void()> func);
};

#endif
