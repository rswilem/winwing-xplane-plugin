#include "path.h"

#include "config.h"
#include "dataref.h"

#include <XPLMPlanes.h>
#include <XPLMUtilities.h>

Path *Path::instance = nullptr;

Path::Path() {
    rootDirectory = "";
    pluginDirectory = "";
    aircraftDirectory = "";
    aircraftFilename = "";
}

Path::~Path() {
    instance = nullptr;
}

Path *Path::getInstance() {
    if (instance == nullptr) {
        instance = new Path();
    }

    return instance;
}

void Path::reloadPaths() {
    char systemPath[512];
    XPLMGetSystemPath(systemPath);
    rootDirectory = systemPath;
    if (rootDirectory.ends_with("/")) {
        rootDirectory = rootDirectory.substr(0, rootDirectory.length() - 1); // Remove trailing slash
    }
    pluginDirectory = rootDirectory + PLUGIN_DIRECTORY;

    char filename[256];
    char modelPath[512];
    XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, filename, modelPath);
    std::string aircraftExecutable = modelPath;
    if (!aircraftExecutable.empty()) {
        size_t filenamePos = aircraftExecutable.find(filename);
        if (filenamePos != std::string::npos) {
            aircraftDirectory = aircraftExecutable.substr(0, filenamePos);
        } else {
            aircraftDirectory = aircraftExecutable;
        }

        if (aircraftDirectory.ends_with("/")) {
            aircraftDirectory.pop_back();
        }

        aircraftFilename = filename;
    } else {
        aircraftDirectory = "";
        aircraftFilename = "";
    }
}
