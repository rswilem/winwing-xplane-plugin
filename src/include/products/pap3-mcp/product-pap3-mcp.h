#ifndef PRODUCT_PAP3_MCP_H
#define PRODUCT_PAP3_MCP_H

#include "usbdevice.h"
#include "mcp-aircraft-profile.h"
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <set>

class ProductPAP3MCP : public USBDevice {
public:
    static constexpr uint8_t  OPCODE_UPDATE = 0x38;
    static constexpr uint8_t  OPCODE_COMMIT = 0x2A;

    // LEDs (depuis ton dissector)
    enum class Led : uint8_t {
        N1=0x03, SPEED=0x04, VNAV=0x05, LVL_CHG=0x06, HDG_SEL=0x07, LNAV=0x08,
        VOR_LOC=0x09, APP=0x0A, ALT_HLD=0x0B, VS=0x0C, CMD_A=0x0D, CWS_A=0x0E,
        CMD_B=0x0F, CWS_B=0x10, AT_ARM=0x11, MA_CAPT=0x12, MA_FO=0x13
    };

    struct Display { // interne driver; identique aux champs du profil
        std::string speed4, heading3, altitude5, vs4, captCourse3, foCourse3;
        bool vsNegative=false, vsPositive=false;
    };

    explicit ProductPAP3MCP(HIDDeviceHandle h, uint16_t vid, uint16_t pid,
                            std::string vname, std::string pname);
    ~ProductPAP3MCP() override;

    const char* classIdentifier() override { return "Product-PAP3-MCP"; }
    bool connect() override;
    void disconnect() override;
    void update() override;            // poll: rafraîchir LCD/LED si datarefs changent
    void didReceiveData(int, uint8_t*, int) override; // TODO: input report PAP3

    // Sorties matérielles
    void setLed(Led id, bool on);
    void sendLCD(const Display& d);

    // Helpers profil
    void requestDisplayRefresh(); // force update (ex: au connect)
    Display& getDisplay() { return display_; }

private:
    MCPAircraftProfile* profile_ = nullptr;
    int lastUpdateCycle_ = 0;
    Display display_;
    std::set<int> pressedButtons_; // pour l’input quand on le branchera

    // encoding 7seg (offsets/pos identiques à ton dissector)
    struct Offs { uint8_t A,B,C,D,E,F,G; };
    static constexpr Offs OFF_SPEED_CAPT  {0x35,0x31,0x2D,0x29,0x25,0x21,0x1D};
    static constexpr Offs OFF_HEAD_ALT_HI {0x36,0x32,0x2E,0x2A,0x26,0x22,0x1E};
    static constexpr Offs OFF_FO_COURSE   {0x38,0x34,0x30,0x2C,0x28,0x24,0x20};
    static constexpr Offs OFF_VS_ALT_LO   {0x37,0x33,0x2F,0x2B,0x27,0x23,0x1F};

    static constexpr uint8_t POS_U=0x01, POS_D=0x02, POS_C=0x04, POS_M=0x08;
    static constexpr uint8_t POS_PilCU=0x20, POS_PilCD=0x40, POS_PilCC=0x80;

    static constexpr uint8_t POS_AltC=0x01, POS_AltK=0x02, POS_AltT=0x04;
    static constexpr uint8_t POS_HdU=0x10, POS_HdD=0x20, POS_HdC=0x40;

    static constexpr uint8_t POS_FoCU=0x10, POS_FoCD=0x20, POS_FoCC=0x40;
    static constexpr uint8_t POS_PlusBar=0x80;

    static constexpr uint8_t POS_VsU=0x01, POS_VsD=0x02, POS_VsC=0x04, POS_VsK=0x08;
    static constexpr uint8_t POS_Vs10K=0x10, POS_AltU=0x40, POS_AltD=0x80;

    static void charToSeg(char ch, bool& A,bool& B,bool& C,bool& D,bool& E,bool& F,bool& G);
    static void placeDigit(std::vector<uint8_t>& frm, const Offs& o, uint8_t posMask, char ch);

    static void build38_first(std::vector<uint8_t>& f, uint8_t pkt);
    static void build38_empty(std::vector<uint8_t>& f, uint8_t pkt);
    static void build2A_commit(std::vector<uint8_t>& f, uint8_t pkt);
    void sendLCDFrames(const std::vector<uint8_t>& packed);

    // profil
    void setProfileForCurrentAircraft();
    void monitorDatarefs();
    void updateDisplays(); // calquée sur ProductFCUEfis::updateDisplays
};

#endif
