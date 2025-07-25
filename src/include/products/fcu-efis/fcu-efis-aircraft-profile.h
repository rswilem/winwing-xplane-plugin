#ifndef FCUEFIS_AIRCRAFT_PROFILE_H
#define FCUEFIS_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>
#include <functional>
#include <map>

struct FCUEfisButtonDef {
    int id;
    std::string name;
    std::string dataref;
    int value = -1;  // Optional: for buttons that set specific values (-1 means use command)
};

enum class FCUEfisLed : int {
    // FCU LEDs
    BACKLIGHT = 0,
    SCREEN_BACKLIGHT = 1,
    LOC_GREEN = 3,
    AP1_GREEN = 5,
    AP2_GREEN = 7,
    ATHR_GREEN = 9,
    EXPED_GREEN = 11,
    APPR_GREEN = 13,
    FLAG_GREEN = 17,
    EXPED_YELLOW = 30,
    
    // EFIS Right LEDs (100-199)
    EFISR_BACKLIGHT = 100,
    EFISR_SCREEN_BACKLIGHT = 101,
    EFISR_FLAG_GREEN = 102,
    EFISR_FD_GREEN = 103,
    EFISR_LS_GREEN = 104,
    EFISR_CSTR_GREEN = 105,
    EFISR_WPT_GREEN = 106,
    EFISR_VORD_GREEN = 107,
    EFISR_NDB_GREEN = 108,
    EFISR_ARPT_GREEN = 109,
    
    // EFIS Left LEDs (200-299)
    EFISL_BACKLIGHT = 200,
    EFISL_SCREEN_BACKLIGHT = 201,
    EFISL_FLAG_GREEN = 202,
    EFISL_FD_GREEN = 203,
    EFISL_LS_GREEN = 204,
    EFISL_CSTR_GREEN = 205,
    EFISL_WPT_GREEN = 206,
    EFISL_VORD_GREEN = 207,
    EFISL_NDB_GREEN = 208,
    EFISL_ARPT_GREEN = 209
};

struct FCUDisplayData {
    std::string speed;
    std::string heading;
    std::string altitude;
    std::string verticalSpeed;
    std::string efisRBaro;
    std::string efisLBaro;
    
    // Display flags
    bool spdMach = false;
    bool hdgTrk = false;
    bool altManaged = false;
    bool spdManaged = false;
    bool hdgManaged = false;
    bool vsMode = false;
    bool fpaMode = false;
    bool efisRQnh = false;
    bool efisLQnh = false;
    bool efisRHpaDec = false;
    bool efisLHpaDec = false;
};

class FCUEfisAircraftProfile {
public:
    FCUEfisAircraftProfile() {};
    virtual ~FCUEfisAircraftProfile() = default;
    
    std::function<void(FCUEfisLed led, unsigned char brightness)> ledBrightnessCallback = nullptr;

    virtual const std::vector<std::string>& displayDatarefs() const = 0;
    virtual const std::vector<FCUEfisButtonDef>& buttonDefs() const = 0;
    virtual void updateDisplayData(FCUDisplayData& displayData, const std::map<std::string, std::string>& cachedDatarefValues) = 0;
    virtual bool hasEfisRight() const = 0;
    virtual bool hasEfisLeft() const = 0;
};

#endif
