#ifndef PFP_AIRCRAFT_PROFILE_H
#define PFP_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>
#include <map>
#include <XPLMUtilities.h>

enum PFPColor : unsigned short {
    PFPColorToDo = 0x0000,
    
    PFPColorL = 0x0000,
    PFPColorAmber = 0x0021,
    PFPColorWhite = 0x0042,
    PFPColorBlue = 0x0063,
    PFPColorGreen = 0x0084,
    PFPColorMagenta = 0x00A5,
    PFPColorRed = 0x00C6,
    PFPColorYellow = 0x00E7,
    PFPColorError = 0x0108,
    PFPColorDefault = PFPColorWhite
};

struct PFPButtonDef {
    int id;
    std::string name;
    std::string dataref;
    double value = 0.0;
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

class ProductPFP;

class PfpAircraftProfile {
protected:
    ProductPFP *product;
    
public:
    PfpAircraftProfile(ProductPFP *product) : product(product) {};
    virtual ~PfpAircraftProfile() = default;

    virtual const std::vector<std::string>& displayDatarefs() const = 0;
    virtual const std::vector<PFPButtonDef>& buttonDefs() const = 0;
    virtual const std::map<char, int>& colorMap() const = 0;
    virtual void updatePage(std::vector<std::vector<char>>& page) = 0;
    virtual void buttonPressed(const PFPButtonDef *button, XPLMCommandPhase phase) = 0;
};

#endif
