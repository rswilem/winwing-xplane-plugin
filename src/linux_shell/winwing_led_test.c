#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#define WINWING_VENDOR_ID 0x4098
#define PFP_LED_BACKLIGHT         0
#define PFP_LED_SCREEN_BACKLIGHT  1
#define PFP_LED_CALL              3
#define PFP_LED_FAIL              4
#define PFP_LED_MSG               5
#define PFP_LED_OFST              6
#define PFP_LED_EXEC              7

static int device_fd = -1;
static int current_led = PFP_LED_CALL;
static int led_rotation[] = {
  PFP_LED_CALL,
  PFP_LED_FAIL,
  PFP_LED_MSG,
  PFP_LED_OFST,
  PFP_LED_EXEC
};
static const int num_leds = sizeof(led_rotation) / sizeof(led_rotation[0]);
static int current_led_index = 0;

const char* get_led_name(int led_id) {
    switch (led_id) {
        case PFP_LED_CALL: return "CALL";
        case PFP_LED_FAIL: return "FAIL";
        case PFP_LED_MSG: return "MSG";
        case PFP_LED_OFST: return "OFST";
        case PFP_LED_EXEC: return "EXEC";
        default: return "UNKNOWN";
    }
}

void send_led_command(int led_id, int state) {
    uint8_t packet[] = {0x02, 0x32, 0xbb, 0x00, 0x00, 0x03, 0x49, led_id, state, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    size_t result = write(device_fd, packet, sizeof(packet));
    if (result == sizeof(packet)) {
        printf("LED packet sent successfully\n");
    } else {
        printf("Failed to send LED packet: %s (wrote %zd of %zu bytes)\n", 
               strerror(errno), result, sizeof(packet));
    }
}

void turn_off_all_leds() {
    printf("Turning off all LEDs...\n");
    for (int i = 0; i < num_leds; i++) {
        send_led_command(led_rotation[i], 0);
    }
}

void signal_handler(int sig) {
    printf("\nReceived signal %d, turning off all LEDs and exiting...\n", sig);
    
    if (device_fd >= 0) {
        turn_off_all_leds();
        close(device_fd);
    }
    
    exit(0);
}

int find_winwing_device() {
    DIR* dir = opendir("/dev");
    if (!dir) {
        printf("Failed to open /dev directory\n");
        return -1;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "hidraw", 6) == 0) {
            char device_path[256];
            snprintf(device_path, sizeof(device_path), "/dev/%s", entry->d_name);
            
            printf("Checking device: %s\n", device_path);
            
            int fd = open(device_path, O_RDWR);
            if (fd < 0) {
                printf("  Failed to open: %s\n", strerror(errno));
                continue;
            }
            
            struct hidraw_devinfo info;
            if (ioctl(fd, HIDIOCGRAWINFO, &info) < 0) {
                printf("  Failed to get device info: %s\n", strerror(errno));
                close(fd);
                continue;
            }
            
            printf("  Vendor ID: 0x%04X, Product ID: 0x%04X\n", info.vendor, info.product);
            
            if (info.vendor == WINWING_VENDOR_ID) {
                char name[256] = {};
                if (ioctl(fd, HIDIOCGRAWNAME(sizeof(name)), name) >= 0) {
                    printf("  Found Winwing device: %s\n", name);
                } else {
                    printf("  Found Winwing device (name unknown)\n");
                }
                
                closedir(dir);
                return fd;
            }
            
            close(fd);
        }
    }
    
    closedir(dir);
    printf("No Winwing devices found\n");
    return -1;
}

int main() {
    printf("Winwing LED Test Tool\n");
    printf("=====================\n");
    
    // Set up signal handler for Ctrl+C
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Searching for Winwing devices...\n");
    device_fd = find_winwing_device();
    
    if (device_fd < 0) {
        printf("Error: No Winwing device found!\n");
        printf("Make sure your Winwing device is connected and you have permission to access /dev/hidraw* devices.\n");
        printf("You may need to run: sudo chmod 666 /dev/hidraw*\n");
        return 1;
    }
    
    printf("\nDevice opened successfully (fd=%d)\n", device_fd);
    
    printf("\nStarting LED rotation test...\n");
    printf("LEDs will rotate from FAIL to FM2 and loop back.\n");
    printf("Press Ctrl+C to turn off all LEDs and exit.\n\n");
    
    while (1) {
        int prev_led_index = (current_led_index - 1 + num_leds) % num_leds;
        int prev_led = led_rotation[prev_led_index];
        send_led_command(prev_led, 0);
        
        current_led = led_rotation[current_led_index];
        printf("=== Activating LED %d/%d: %s ===\n", 
               current_led_index + 1, num_leds, get_led_name(current_led));
        send_led_command(current_led, 1);
        
        current_led_index = (current_led_index + 1) % num_leds;
        
        printf("Waiting a second (press ctrl+C to exit)...\n\n");
        sleep(1);
    }
    
    return 0;
}
