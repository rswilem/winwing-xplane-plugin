//
//  xplane-sdk-mock.cpp
//  Winwing
//
//  Created by Ramon Swilem on 04/07/2025.
//

#include <cstdio>
#include <algorithm>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include "dataref.h"
#include <vector>
#include <string>

static std::vector<std::string> registeredCommands = {};

void XPLMCommandBegin(XPLMCommandRef ref) {
    int idx = static_cast<int>(reinterpret_cast<intptr_t>(ref)) - 1;
    if (idx >= 0 && idx < static_cast<int>(registeredCommands.size())) {
        printf("Executing command (start): %s\n", registeredCommands[idx].c_str());
    } else {
        printf("Executing command (start): invalid ref\n");
    }
}

void XPLMCommandEnd(XPLMCommandRef ref) {
    int idx = static_cast<int>(reinterpret_cast<intptr_t>(ref)) - 1;
    if (idx >= 0 && idx < static_cast<int>(registeredCommands.size())) {
        printf("Executing command (end): %s\n", registeredCommands[idx].c_str());
    } else {
        printf("Executing command (end): invalid ref\n");
    }

    if (registeredCommands[idx] == "AirbusFBW/MCDU1KeyBright" || registeredCommands[idx] == "AirbusFBW/MCDU1KeyDim") {
        float brightness = registeredCommands[idx] == "AirbusFBW/MCDU1KeyDim" ? 0.2 : 0.8;
        Dataref::getInstance()->set<std::vector<float>>("AirbusFBW/MCDUIntegBrightness", {brightness, brightness, brightness, brightness}, true);
        Dataref::getInstance()->set<std::vector<float>>("AirbusFBW/DUBrightness", {brightness, brightness, brightness, brightness, brightness, brightness, brightness, brightness}, true);
    }
}

void XPLMCommandOnce(XPLMCommandRef ref) {
    int idx = static_cast<int>(reinterpret_cast<intptr_t>(ref)) - 1;
    if (idx >= 0 && idx < static_cast<int>(registeredCommands.size())) {
        printf("Executing command (once): %s\n", registeredCommands[idx].c_str());
    } else {
        printf("Executing command (once): invalid ref\n");
    }
}

XPLMCommandRef XPLMCreateCommand(const char *name, const char *desc) {
    printf("Creating command: %s\n", name);
    return 0;
}

void XPLMDebugString(const char *data) {
    printf("%s", data);
}

XPLMCommandRef XPLMFindCommand(const char *name) {
    std::string nameStr(name);
    auto it = std::find(registeredCommands.begin(), registeredCommands.end(), nameStr);
    if (it != registeredCommands.end()) {
        return (XPLMCommandRef)(std::distance(registeredCommands.begin(), it) + 1);
    }
    registeredCommands.push_back(nameStr);
    return (XPLMCommandRef)registeredCommands.size();
}

XPLMDataRef XPLMFindDataRef(const char *name) {
    return XPLMFindCommand(name);
}

XPLMDataTypeID XPLMGetDataRefTypes(XPLMDataRef ref) {
    return xplmType_Int;
}

int XPLMGetDatab(XPLMDataRef ref, void *outValue, int offset, int maxBytes) {
    return 0;
}

float XPLMGetDataf(XPLMDataRef ref) {
    return 0;
}

int XPLMGetDatai(XPLMDataRef ref) {
    return 0;
}

int XPLMGetDatavf(XPLMDataRef ref, float *values, int offset, int max) {
    return 0;
}

int XPLMGetDatavi(XPLMDataRef ref, int *values, int offset, int max) {
    return 0;
}

void XPLMRegisterCommandHandler(XPLMDataRef ref, XPLMCommandCallback_f handler, int before, void *refcon) {
}

// Mock implementations for missing XPLM functions

void XPLMUnregisterCommandHandler(XPLMDataRef ref, XPLMCommandCallback_f handler, int before, void *refcon) {
    printf("Unregistering command handler\n");
}

void XPLMUnregisterDataAccessor(XPLMDataRef ref) {
    printf("Unregistering data accessor\n");
}

void XPLMSetDatai(XPLMDataRef ref, int value) {
    printf("Set data int: %d\n", value);
}

void XPLMSetDataf(XPLMDataRef ref, float value) {
    printf("Set data float: %f\n", value);
}

void XPLMSetDatab(XPLMDataRef ref, void *inValue, int inOffset, int inLength) {
    printf("Set data bytes (offset: %d, length: %d)\n", inOffset, inLength);
}

void XPLMSetDatavi(XPLMDataRef ref, int *inValues, int inOffset, int inCount) {
    printf("Set data int array (offset: %d, count: %d)\n", inOffset, inCount);
}

void XPLMSetDatavf(XPLMDataRef ref, float *inValues, int inOffset, int inCount) {
    printf("Set data float array (offset: %d, count: %d)\n", inOffset, inCount);
}

XPLMDataRef XPLMRegisterDataAccessor(
    const char *inDataName,
    XPLMDataTypeID inDataType,
    int inIsWritable,
    XPLMGetDatai_f inReadInt,
    XPLMSetDatai_f inWriteInt,
    XPLMGetDataf_f inReadFloat,
    XPLMSetDataf_f inWriteFloat,
    XPLMGetDatad_f inReadDouble,
    XPLMSetDatad_f inWriteDouble,
    XPLMGetDatavi_f inReadIntArray,
    XPLMSetDatavi_f inWriteIntArray,
    XPLMGetDatavf_f inReadFloatArray,
    XPLMSetDatavf_f inWriteFloatArray,
    XPLMGetDatab_f inReadData,
    XPLMSetDatab_f inWriteData,
    void *inReadRefcon,
    void *inWriteRefcon)
{
    printf("Registering data accessor: %s\n", inDataName);
    return 0;
}

