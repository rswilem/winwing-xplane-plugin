#ifndef PFP_AIRCRAFT_PROFILE_H
#define PFP_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>
#include <functional>
#include <map>

struct PFPButtonDef {
    int id;
    std::string name;
    std::string dataref;
};

enum class PFPLed : int {
    BACKLIGHT = 0,
    SCREEN_BACKLIGHT = 1,
    CALL = 3,
    FAIL = 4,
    MSG = 5,
    OFST = 6,
    EXEC = 7
};

class PfpAircraftProfile {
public:
    PfpAircraftProfile() {};
    virtual ~PfpAircraftProfile() = default;
    
    std::function<void(PFPLed led, unsigned char brightness)> ledBrightnessCallback = nullptr;

    virtual const std::vector<std::string>& displayDatarefs() const = 0;
    virtual const std::vector<PFPButtonDef>& buttonDefs() const = 0;
    virtual const std::map<char, int>& colorMap() const = 0;
    virtual void updatePage(std::vector<std::vector<char>>& page, const std::map<std::string, std::string>& cachedDatarefValues) = 0;
};

#endif
