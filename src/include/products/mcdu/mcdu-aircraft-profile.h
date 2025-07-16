#ifndef MCDU_AIRCRAFT_PROFILE_H
#define MCDU_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <regex>

struct MCDUButtonDef {
    int id;
    std::string name;
    std::string dataref;
};

struct MCDUColorInfo {
    char color;
    bool fontSmall;
};

enum class MCDULed : int {
    BACKLIGHT = 0,
    SCREEN_BACKLIGHT = 1,
    FAIL = 8,
    FM = 9,
    MCDU = 10,
    MENU = 11,
    FM1 = 12,
    IND = 13,
    RDY = 14,
    STATUS = 15,
    FM2 = 16
};


class McduAircraftProfile {
public:
    McduAircraftProfile() {};
    virtual ~McduAircraftProfile() = default;
    
    std::function<void(MCDULed keypad, unsigned char brightness)> ledBrightnessCallback = nullptr;

    virtual const std::vector<std::string>& displayDatarefs() const = 0;
    virtual const std::vector<MCDUButtonDef>& buttonDefs() const = 0;
    virtual const std::map<char, int>& colorMap() const = 0;
    virtual void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) = 0;
};

#endif
