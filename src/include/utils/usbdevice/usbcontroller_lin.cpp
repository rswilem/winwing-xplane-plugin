#if LIN
#include "usbcontroller.h"
#include "usbdevice.h"
#include "config.h"
#include <XPLMUtilities.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <libudev.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>

USBController* USBController::instance = nullptr;
static std::atomic<bool> shouldStopMonitoring{false};

USBController::USBController() {
    struct udev* udev = udev_new();
    if (!udev) {
        debug_force("Failed to create udev context");
        return;
    }
    
    hidManager = udev_monitor_new_from_netlink(udev, "udev");
    if (!hidManager) {
        debug_force("Failed to create udev monitor");
        udev_unref(udev);
        return;
    }
    
    udev_monitor_filter_add_match_subsystem_devtype(hidManager, "hidraw", nullptr);
    udev_monitor_enable_receiving(hidManager);
    
    shouldStopMonitoring = false;
    std::thread monitorThread([this]() {
        monitorDevices();
    });
    monitorThread.detach();
    
    enumerateDevices();
}

USBController::~USBController() {
    destroy();
}

USBController* USBController::getInstance() {
    if (instance == nullptr) {
        instance = new USBController();
    }
    return instance;
}

void USBController::initialize() {
    
}

void USBController::destroy() {
    debug_force("Linux: Stopping monitoring thread...\n");
    shouldStopMonitoring = true;
    
    // Give the monitoring thread time to exit gracefully
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    debug_force("Linux: Destroying USB devices...\n");
    for (auto ptr : devices) {
        delete ptr;
    }
    devices.clear();
    
    debug_force("Linux: Cleaning up hidManager...\n");
    if (hidManager) {
        struct udev* udev = udev_monitor_get_udev(hidManager);
        udev_monitor_unref(hidManager);
        udev_unref(udev);
        hidManager = nullptr;
    }
    
    debug_force("Linux: Clearing instance...\n");
    instance = nullptr;
}

void USBController::enumerateDevices() {
    DIR* dir = opendir("/dev");
    if (!dir) {
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strncmp(entry->d_name, "hidraw", 6) == 0) {
            std::string devicePath = "/dev/" + std::string(entry->d_name);
            int fd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);
            if (fd >= 0) {
                struct hidraw_devinfo info;
                if (ioctl(fd, HIDIOCGRAWINFO, &info) >= 0 && info.vendor == WINWING_VENDOR_ID) {
                    char name[256] = {};
                    if (ioctl(fd, HIDIOCGRAWNAME(sizeof(name)), name) >= 0) {
                        USBDevice* device = USBDevice::Device(fd, info.vendor, info.product, "Winwing", std::string(name));
                        if (device) {
                            devices.push_back(device);
                        }
                    } else {
                        close(fd);
                    }
                } else {
                    close(fd);
                }
            }
        }
    }
    closedir(dir);
}

void USBController::monitorDevices() {
    int fd = udev_monitor_get_fd(hidManager);
    while (!shouldStopMonitoring) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        
        struct timeval timeout = {1, 0};
        int ret = select(fd + 1, &fds, nullptr, nullptr, &timeout);
        
        if (ret > 0 && FD_ISSET(fd, &fds) && !shouldStopMonitoring) {
            struct udev_device* device = udev_monitor_receive_device(hidManager);
            if (device) {
                const char* action = udev_device_get_action(device);
                if (strcmp(action, "add") == 0) {
                    DeviceAddedCallback(this, device);
                } else if (strcmp(action, "remove") == 0) {
                    DeviceRemovedCallback(this, device);
                }
                udev_device_unref(device);
            }
        }
    }
    debug_force("Linux: Monitor thread exiting\n");
}

void USBController::DeviceAddedCallback(void* context, struct udev_device* device) {
    auto* self = static_cast<USBController*>(context);
    
    const char* devicePath = udev_device_get_devnode(device);
    if (!devicePath) {
        return;
    }
    
    int fd = open(devicePath, O_RDWR | O_NONBLOCK);
    if (fd >= 0) {
        struct hidraw_devinfo info;
        if (ioctl(fd, HIDIOCGRAWINFO, &info) >= 0 && info.vendor == WINWING_VENDOR_ID) {
            char name[256] = {};
            if (ioctl(fd, HIDIOCGRAWNAME(sizeof(name)), name) >= 0) {
                USBDevice* newDevice = USBDevice::Device(fd, info.vendor, info.product, "Winwing", std::string(name));
                if (newDevice) {
                    self->devices.push_back(newDevice);
                }
            } else {
                close(fd);
            }
        } else {
            close(fd);
        }
    }
}

void USBController::DeviceRemovedCallback(void* context, struct udev_device* device) {
    auto* self = static_cast<USBController*>(context);
    
    const char* devicePath = udev_device_get_devnode(device);
    if (!devicePath) {
        return;
    }
    
    for (auto it = self->devices.begin(); it != self->devices.end(); ++it) {
        if ((*it)->hidDevice < 0) {
            delete *it;
            self->devices.erase(it);
            break;
        }
    }
}
#endif
