#ifndef FMC_HARDWARE_MAPPING_H
#define FMC_HARDWARE_MAPPING_H

enum class FMCHardwareType : unsigned char {
    HARDWARE_MCDU = 1,
    HARDWARE_PFP3N,
    HARDWARE_PFP4,
    HARDWARE_PFP7,
};

enum class FMCKey : unsigned char {
    LSK1L = 1,
    LSK1R,
    LSK2L,
    LSK2R,
    LSK3L,
    LSK3R,
    LSK4L,
    LSK4R,
    LSK5L,
    LSK5R,
    LSK6L,
    LSK6R,
    BRIGHTNESS_DOWN,
    BRIGHTNESS_UP,
    PAGE_UP,
    PAGE_DOWN,
    PAGE_NEXT,
    PAGE_PREV,
    AIRPORT_OR_FIX,
    ALTN,
    ATC,
    CLB,
    CRZ,
    DATA,
    DEP_ARR,
    DES,
    DIRTO,
    MCDU_EMPTY_BOTTOM_LEFT,
    EXEC_OR_MCDU_EMPTY_TOP_RIGHT,
    FMC_COMM,
    FPLN,
    FUEL,
    HOLD,
    INIT_REF,
    LEGS,
    MENU,
    SLASH,
    N1_LIMIT_OR_PERF,
    PROG,
    RAD_NAV,
    ROUTE,
    SEC_FPLN,
    VNAV,
    KEY0,
    KEY1,
    KEY2,
    KEY3,
    KEY4,
    KEY5,
    KEY6,
    KEY7,
    KEY8,
    KEY9,
    KEYA,
    KEYB,
    KEYC,
    KEYD,
    KEYE,
    KEYF,
    KEYG,
    KEYH,
    KEYI,
    KEYJ,
    KEYK,
    KEYL,
    KEYM,
    KEYN,
    KEYO,
    KEYP,
    KEYQ,
    KEYR,
    KEYS,
    KEYT,
    KEYU,
    KEYV,
    KEYW,
    KEYX,
    KEYY,
    KEYZ,
    PERIOD,
    PLUSMINUS,
    SPACE,
    CLR,
    OVERFLY_OR_DEL,
};

struct FMCButtonDef {
    FMCKey key;
    std::string dataref;
    double value = 0.0;
};

class FMCHardwareMapping {
public:
    static FMCKey ButtonIdentifierForIndex(FMCHardwareType hardwareType, int index) {
        switch (hardwareType) {
            case FMCHardwareType::HARDWARE_MCDU:
                return getMCDUButtonForIndex(index);
            case FMCHardwareType::HARDWARE_PFP3N:
                return getPFP3NButtonForIndex(index);
            case FMCHardwareType::HARDWARE_PFP4:
                return getPFP4ButtonForIndex(index);
            case FMCHardwareType::HARDWARE_PFP7:
                return getPFP7ButtonForIndex(index);
            default:
                return FMCKey::LSK1L;
        }
    }

private:
    static FMCKey getMCDUButtonForIndex(int index) {
        switch (index) {
            case 0: return FMCKey::LSK1L;
            case 1: return FMCKey::LSK2L;
            case 2: return FMCKey::LSK3L;
            case 3: return FMCKey::LSK4L;
            case 4: return FMCKey::LSK5L;
            case 5: return FMCKey::LSK6L;
            case 6: return FMCKey::LSK1R;
            case 7: return FMCKey::LSK2R;
            case 8: return FMCKey::LSK3R;
            case 9: return FMCKey::LSK4R;
            case 10: return FMCKey::LSK5R;
            case 11: return FMCKey::LSK6R;
            case 12: return FMCKey::DIRTO;
            case 13: return FMCKey::PROG;
            case 14: return FMCKey::N1_LIMIT_OR_PERF;
            case 15: return FMCKey::INIT_REF;
            case 16: return FMCKey::DATA;
            case 17: return FMCKey::EXEC_OR_MCDU_EMPTY_TOP_RIGHT;
            case 18: return FMCKey::BRIGHTNESS_UP;
            case 19: return FMCKey::FPLN;
            case 20: return FMCKey::RAD_NAV;
            case 21: return FMCKey::FUEL;
            case 22: return FMCKey::SEC_FPLN;
            case 23: return FMCKey::ATC;
            case 24: return FMCKey::MENU;
            case 25: return FMCKey::BRIGHTNESS_DOWN;
            case 26: return FMCKey::AIRPORT_OR_FIX;
            case 27: return FMCKey::MCDU_EMPTY_BOTTOM_LEFT;
            case 28: return FMCKey::PAGE_PREV;
            case 29: return FMCKey::PAGE_UP;
            case 30: return FMCKey::PAGE_NEXT;
            case 31: return FMCKey::PAGE_DOWN;
            case 32: return FMCKey::KEY1;
            case 33: return FMCKey::KEY2;
            case 34: return FMCKey::KEY3;
            case 35: return FMCKey::KEY4;
            case 36: return FMCKey::KEY5;
            case 37: return FMCKey::KEY6;
            case 38: return FMCKey::KEY7;
            case 39: return FMCKey::KEY8;
            case 40: return FMCKey::KEY9;
            case 41: return FMCKey::PERIOD;
            case 42: return FMCKey::KEY0;
            case 43: return FMCKey::PLUSMINUS;
            case 44: return FMCKey::KEYA;
            case 45: return FMCKey::KEYB;
            case 46: return FMCKey::KEYC;
            case 47: return FMCKey::KEYD;
            case 48: return FMCKey::KEYE;
            case 49: return FMCKey::KEYF;
            case 50: return FMCKey::KEYG;
            case 51: return FMCKey::KEYH;
            case 52: return FMCKey::KEYI;
            case 53: return FMCKey::KEYJ;
            case 54: return FMCKey::KEYK;
            case 55: return FMCKey::KEYL;
            case 56: return FMCKey::KEYM;
            case 57: return FMCKey::KEYN;
            case 58: return FMCKey::KEYO;
            case 59: return FMCKey::KEYP;
            case 60: return FMCKey::KEYQ;
            case 61: return FMCKey::KEYR;
            case 62: return FMCKey::KEYS;
            case 63: return FMCKey::KEYT;
            case 64: return FMCKey::KEYU;
            case 65: return FMCKey::KEYV;
            case 66: return FMCKey::KEYW;
            case 67: return FMCKey::KEYX;
            case 68: return FMCKey::KEYY;
            case 69: return FMCKey::KEYZ;
            case 70: return FMCKey::SLASH;
            case 71: return FMCKey::SPACE;
            case 72: return FMCKey::OVERFLY_OR_DEL;
            case 73: return FMCKey::CLR;
        }
        
        return FMCKey::KEY0;
    }

    static FMCKey getPFP3NButtonForIndex(int index) {
        switch (index) {
            case 0: return FMCKey::LSK1L;
            case 1: return FMCKey::LSK2L;
            case 2: return FMCKey::LSK3L;
            case 3: return FMCKey::LSK4L;
            case 4: return FMCKey::LSK5L;
            case 5: return FMCKey::LSK6L;
            case 6: return FMCKey::LSK1R;
            case 7: return FMCKey::LSK2R;
            case 8: return FMCKey::LSK3R;
            case 9: return FMCKey::LSK4R;
            case 10: return FMCKey::LSK5R;
            case 11: return FMCKey::LSK6R;
            case 12: return FMCKey::INIT_REF;
            case 13: return FMCKey::ROUTE;
            case 14: return FMCKey::CLB;
            case 15: return FMCKey::CRZ;
            case 16: return FMCKey::DES;
            case 17: return FMCKey::BRIGHTNESS_DOWN;
            case 18: return FMCKey::BRIGHTNESS_UP;
            case 19: return FMCKey::MENU;
            case 20: return FMCKey::LEGS;
            case 21: return FMCKey::DEP_ARR;
            case 22: return FMCKey::HOLD;
            case 23: return FMCKey::PROG;
            case 24: return FMCKey::EXEC_OR_MCDU_EMPTY_TOP_RIGHT;
            case 25: return FMCKey::N1_LIMIT_OR_PERF;
            case 26: return FMCKey::AIRPORT_OR_FIX;
            case 27: return FMCKey::PAGE_PREV;
            case 28: return FMCKey::PAGE_NEXT;
            case 29: return FMCKey::KEY1;
            case 30: return FMCKey::KEY2;
            case 31: return FMCKey::KEY3;
            case 32: return FMCKey::KEY4;
            case 33: return FMCKey::KEY5;
            case 34: return FMCKey::KEY6;
            case 35: return FMCKey::KEY7;
            case 36: return FMCKey::KEY8;
            case 37: return FMCKey::KEY9;
            case 38: return FMCKey::PERIOD;
            case 39: return FMCKey::KEY0;
            case 40: return FMCKey::PLUSMINUS;
            case 41: return FMCKey::KEYA;
            case 42: return FMCKey::KEYB;
            case 43: return FMCKey::KEYC;
            case 44: return FMCKey::KEYD;
            case 45: return FMCKey::KEYE;
            case 46: return FMCKey::KEYF;
            case 47: return FMCKey::KEYG;
            case 48: return FMCKey::KEYH;
            case 49: return FMCKey::KEYI;
            case 50: return FMCKey::KEYJ;
            case 51: return FMCKey::KEYK;
            case 52: return FMCKey::KEYL;
            case 53: return FMCKey::KEYM;
            case 54: return FMCKey::KEYN;
            case 55: return FMCKey::KEYO;
            case 56: return FMCKey::KEYP;
            case 57: return FMCKey::KEYQ;
            case 58: return FMCKey::KEYR;
            case 59: return FMCKey::KEYS;
            case 60: return FMCKey::KEYT;
            case 61: return FMCKey::KEYU;
            case 62: return FMCKey::KEYV;
            case 63: return FMCKey::KEYW;
            case 64: return FMCKey::KEYX;
            case 65: return FMCKey::KEYY;
            case 66: return FMCKey::KEYZ;
            case 67: return FMCKey::SPACE;
            case 68: return FMCKey::OVERFLY_OR_DEL;
            case 69: return FMCKey::SLASH;
            case 70: return FMCKey::CLR;
        }
        
        return FMCKey::KEY0;
    }

    static FMCKey getPFP4ButtonForIndex(int index) {
        switch (index) {
            case 0: return FMCKey::LSK1L;
            case 1: return FMCKey::LSK2L;
            case 2: return FMCKey::LSK3L;
            case 3: return FMCKey::LSK4L;
            case 4: return FMCKey::LSK5L;
            case 5: return FMCKey::LSK6L;
            case 6: return FMCKey::LSK1R;
            case 7: return FMCKey::LSK2R;
            case 8: return FMCKey::LSK3R;
            case 9: return FMCKey::LSK4R;
            case 10: return FMCKey::LSK5R;
            case 11: return FMCKey::LSK6R;
            case 12: return FMCKey::INIT_REF;
            case 13: return FMCKey::ROUTE;
            case 14: return FMCKey::DEP_ARR;
            case 15: return FMCKey::ATC;
            case 16: return FMCKey::VNAV;
            case 17: return FMCKey::BRIGHTNESS_DOWN;
            case 18: return FMCKey::BRIGHTNESS_UP;
            case 19: return FMCKey::AIRPORT_OR_FIX;
            case 20: return FMCKey::LEGS;
            case 21: return FMCKey::HOLD;
            case 22: return FMCKey::FMC_COMM;
            case 23: return FMCKey::PROG;
            case 24: return FMCKey::EXEC_OR_MCDU_EMPTY_TOP_RIGHT;
            case 25: return FMCKey::MENU;
            case 26: return FMCKey::RAD_NAV;
            case 27: return FMCKey::PAGE_PREV;
            case 28: return FMCKey::PAGE_NEXT;
            case 29: return FMCKey::KEY1;
            case 30: return FMCKey::KEY2;
            case 31: return FMCKey::KEY3;
            case 32: return FMCKey::KEY4;
            case 33: return FMCKey::KEY5;
            case 34: return FMCKey::KEY6;
            case 35: return FMCKey::KEY7;
            case 36: return FMCKey::KEY8;
            case 37: return FMCKey::KEY9;
            case 38: return FMCKey::PERIOD;
            case 39: return FMCKey::KEY0;
            case 40: return FMCKey::PLUSMINUS;
            case 41: return FMCKey::KEYA;
            case 42: return FMCKey::KEYB;
            case 43: return FMCKey::KEYC;
            case 44: return FMCKey::KEYD;
            case 45: return FMCKey::KEYE;
            case 46: return FMCKey::KEYF;
            case 47: return FMCKey::KEYG;
            case 48: return FMCKey::KEYH;
            case 49: return FMCKey::KEYI;
            case 50: return FMCKey::KEYJ;
            case 51: return FMCKey::KEYK;
            case 52: return FMCKey::KEYL;
            case 53: return FMCKey::KEYM;
            case 54: return FMCKey::KEYN;
            case 55: return FMCKey::KEYO;
            case 56: return FMCKey::KEYP;
            case 57: return FMCKey::KEYQ;
            case 58: return FMCKey::KEYR;
            case 59: return FMCKey::KEYS;
            case 60: return FMCKey::KEYT;
            case 61: return FMCKey::KEYU;
            case 62: return FMCKey::KEYV;
            case 63: return FMCKey::KEYW;
            case 64: return FMCKey::KEYX;
            case 65: return FMCKey::KEYY;
            case 66: return FMCKey::KEYZ;
            case 67: return FMCKey::SPACE;
            case 68: return FMCKey::OVERFLY_OR_DEL;
            case 69: return FMCKey::SLASH;
            case 70: return FMCKey::CLR;
        }
        
        return FMCKey::KEY0;
    }

    static FMCKey getPFP7ButtonForIndex(int index) {
        switch (index) {
            case 0: return FMCKey::LSK1L;
            case 1: return FMCKey::LSK2L;
            case 2: return FMCKey::LSK3L;
            case 3: return FMCKey::LSK4L;
            case 4: return FMCKey::LSK5L;
            case 5: return FMCKey::LSK6L;
            case 6: return FMCKey::LSK1R;
            case 7: return FMCKey::LSK2R;
            case 8: return FMCKey::LSK3R;
            case 9: return FMCKey::LSK4R;
            case 10: return FMCKey::LSK5R;
            case 11: return FMCKey::LSK6R;
            case 12: return FMCKey::INIT_REF;
            case 13: return FMCKey::ROUTE;
            case 14: return FMCKey::DEP_ARR;
            case 15: return FMCKey::ALTN;
            case 16: return FMCKey::VNAV;
            case 17: return FMCKey::BRIGHTNESS_DOWN;
            case 18: return FMCKey::BRIGHTNESS_UP;
            case 19: return FMCKey::AIRPORT_OR_FIX;
            case 20: return FMCKey::LEGS;
            case 21: return FMCKey::HOLD;
            case 22: return FMCKey::FMC_COMM;
            case 23: return FMCKey::PROG;
            case 24: return FMCKey::EXEC_OR_MCDU_EMPTY_TOP_RIGHT;
            case 25: return FMCKey::MENU;
            case 26: return FMCKey::RAD_NAV;
            case 27: return FMCKey::PAGE_PREV;
            case 28: return FMCKey::PAGE_NEXT;
            case 29: return FMCKey::KEY1;
            case 30: return FMCKey::KEY2;
            case 31: return FMCKey::KEY3;
            case 32: return FMCKey::KEY4;
            case 33: return FMCKey::KEY5;
            case 34: return FMCKey::KEY6;
            case 35: return FMCKey::KEY7;
            case 36: return FMCKey::KEY8;
            case 37: return FMCKey::KEY9;
            case 38: return FMCKey::PERIOD;
            case 39: return FMCKey::KEY0;
            case 40: return FMCKey::PLUSMINUS;
            case 41: return FMCKey::KEYA;
            case 42: return FMCKey::KEYB;
            case 43: return FMCKey::KEYC;
            case 44: return FMCKey::KEYD;
            case 45: return FMCKey::KEYE;
            case 46: return FMCKey::KEYF;
            case 47: return FMCKey::KEYG;
            case 48: return FMCKey::KEYH;
            case 49: return FMCKey::KEYI;
            case 50: return FMCKey::KEYJ;
            case 51: return FMCKey::KEYK;
            case 52: return FMCKey::KEYL;
            case 53: return FMCKey::KEYM;
            case 54: return FMCKey::KEYN;
            case 55: return FMCKey::KEYO;
            case 56: return FMCKey::KEYP;
            case 57: return FMCKey::KEYQ;
            case 58: return FMCKey::KEYR;
            case 59: return FMCKey::KEYS;
            case 60: return FMCKey::KEYT;
            case 61: return FMCKey::KEYU;
            case 62: return FMCKey::KEYV;
            case 63: return FMCKey::KEYW;
            case 64: return FMCKey::KEYX;
            case 65: return FMCKey::KEYY;
            case 66: return FMCKey::KEYZ;
            case 67: return FMCKey::SPACE;
            case 68: return FMCKey::OVERFLY_OR_DEL;
            case 69: return FMCKey::SLASH;
            case 70: return FMCKey::CLR;
        }
        
        return FMCKey::KEY0;
    }
};

#endif
