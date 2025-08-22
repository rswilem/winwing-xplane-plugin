#include "product-pap3-mcp.h"
#include <cstring>

// ============ LEDs (ton format: 02 0F BF 00 00 03 49 XX YY 00 00 00 00) ============
void ProductPAP3MCP::setLed(Led id, bool on) {
    std::vector<uint8_t> buf{
        0x02, 0x0F, 0xBF, 0x00, 0x00, 0x03, 0x49,
        static_cast<uint8_t>(id), static_cast<uint8_t>(on ? 0x01 : 0x00),
        0x00,0x00,0x00,0x00
    };
    writeData(buf);
}

// ============ 7-seg helpers ============
void ProductPAP3MCP::charToSeg(char ch, bool& A,bool& B,bool& C,bool& D,bool& E,bool& F,bool& G) {
    A=B=C=D=E=F=G=false;
    switch (ch) {
        case '0': A=B=C=D=E=F=true; break;
        case '1': B=C=true; break;
        case '2': A=B=D=E=G=true; break;
        case '3': A=B=C=D=G=true; break;
        case '4': B=C=F=G=true; break;
        case '5': A=C=D=F=G=true; break;
        case '6': A=C=D=E=F=G=true; break;
        case '7': A=B=C=true; break;
        case '8': A=B=C=D=E=F=G=true; break;
        case '9': A=B=C=D=F=G=true; break;
        case '-': G=true; break;
        default: break; // ' ' etc.
    }
}

// ============ LCD high-level ============
void ProductPAP3MCP::sendLCD(const Display& d) {
    auto norm = [](std::string s, size_t n){
        if (s.size() < n) s = std::string(n - s.size(), ' ') + s;
        if (s.size() > n) s = s.substr(s.size()-n);
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::toupper(c); });
        return s;
    };
    const std::string SPD = norm(d.speed4,      4);
    const std::string HDG = norm(d.heading3,    3);
    const std::string ALT = norm(d.altitude5,   5);
    const std::string VS  = norm(d.vs4,         4);
    const std::string CRS = norm(d.captCourse3, 3);
    const std::string FCR = norm(d.foCourse3,   3);

    // On prépare un "plan" 64 octets, qu’on remplira sur 0x1D.. pour la 1ère 0x38
    std::vector<uint8_t> map(64, 0x00);

    // Speed (M,C,D,U) + Captain course (PilCC, PilCD, PilCU)
    placeDigit(map, OFF_SPEED_CAPT, POS_M,      SPD[0]);
    placeDigit(map, OFF_SPEED_CAPT, POS_C,      SPD[1]);
    placeDigit(map, OFF_SPEED_CAPT, POS_D,      SPD[2]);
    placeDigit(map, OFF_SPEED_CAPT, POS_U,      SPD[3]);
    placeDigit(map, OFF_SPEED_CAPT, POS_PilCC,  CRS[0]);
    placeDigit(map, OFF_SPEED_CAPT, POS_PilCD,  CRS[1]);
    placeDigit(map, OFF_SPEED_CAPT, POS_PilCU,  CRS[2]);

    // Heading (HdC,HdD,HdU) + Alt high (T,K,C)
    placeDigit(map, OFF_HEAD_ALT_HI, POS_HdC,   HDG[0]);
    placeDigit(map, OFF_HEAD_ALT_HI, POS_HdD,   HDG[1]);
    placeDigit(map, OFF_HEAD_ALT_HI, POS_HdU,   HDG[2]);
    placeDigit(map, OFF_HEAD_ALT_HI, POS_AltT,  ALT[0]);
    placeDigit(map, OFF_HEAD_ALT_HI, POS_AltK,  ALT[1]);
    placeDigit(map, OFF_HEAD_ALT_HI, POS_AltC,  ALT[2]);

    // FO course
    placeDigit(map, OFF_FO_COURSE,   POS_FoCC,  FCR[0]);
    placeDigit(map, OFF_FO_COURSE,   POS_FoCD,  FCR[1]);
    placeDigit(map, OFF_FO_COURSE,   POS_FoCU,  FCR[2]);

    // VS (K,C,D,U) + signe
    placeDigit(map, OFF_VS_ALT_LO,   POS_VsK,   VS[0]);
    placeDigit(map, OFF_VS_ALT_LO,   POS_VsC,   VS[1]);
    placeDigit(map, OFF_VS_ALT_LO,   POS_VsD,   VS[2]);
    placeDigit(map, OFF_VS_ALT_LO,   POS_VsU,   VS[3]);
    if (d.vsNegative) {
        map[OFF_VS_ALT_LO.G] |= POS_Vs10K; // ‘-’
    } else if (d.vsPositive) {
        map[OFF_VS_ALT_LO.G] |= POS_Vs10K; // ‘-’
        map[OFF_FO_COURSE.D] |= POS_PlusBar; // barres "+"
        map[OFF_FO_COURSE.E] |= POS_PlusBar;
    }

    // Alt low (D,U)
    placeDigit(map, OFF_VS_ALT_LO,   POS_AltD,  ALT[3]);
    placeDigit(map, OFF_VS_ALT_LO,   POS_AltU,  ALT[4]);

    // Envoi des 4 trames selon ton format exact
    sendLCDFrames(map);
}

// ============ Frames builders (selon tes octets) ============
void ProductPAP3MCP::build38_first(std::vector<uint8_t>& f, uint8_t pkt) {
    f.assign(64, 0x00);
    f[0]=0xF0; f[1]=0x00;
    f[2]=pkt;
    f[3]=OPCODE_UPDATE;
    f[4]=0x0F; f[5]=0xBF;     // PID UNIQUEMENT ici
    f[8]=0x02;                // command1=02
    f[9]=0x01;                // always one
    f[12]=0xDF; f[13]=0xA2;   // checksum/const ?
    f[14]=0x50; f[15]=0x00;
    f[16]=0x00; f[17]=0x00;
    f[18]=0xB0;               // présent sur 1ère 0x38
    // 0x19..0x1C restent 0x00
}
void ProductPAP3MCP::build38_empty(std::vector<uint8_t>& f, uint8_t pkt) {
    f.assign(64, 0x00);
    f[0]=0xF0; f[1]=0x00;
    f[2]=pkt;
    f[3]=OPCODE_UPDATE;
    // 04..05 = 00 00 (pas de PID ici)
}
void ProductPAP3MCP::build2A_commit(std::vector<uint8_t>& f, uint8_t pkt) {
    f.assign(64, 0x00);
    f[0]=0xF0; f[1]=0x00;
    f[2]=pkt;
    f[3]=OPCODE_COMMIT;
    // 04..05 = 00 00
    const uint8_t tail[] = {0x0F,0xBF,0x00,0x00,0x03,0x01,0x00,0x00,0xDF,0xA2,0x50,0x00};
    for (size_t i=0; i<sizeof(tail); ++i) f[0x1D+i] = tail[i];
}

void ProductPAP3MCP::sendLCDFrames(const std::vector<uint8_t>& payload) {
    static uint8_t pkt = 0x01; // peu importe le départ, il s’incrémente
    // 1) 0x38 (avec PID + en-tête étendu) + payload copié à partir de 0x1D
    std::vector<uint8_t> f1; build38_first(f1, pkt++);
    for (size_t i=0x1D; i<64; ++i) f1[i] = payload[i];
    writeData(f1);

    // 2) 0x38 vide
    std::vector<uint8_t> f2; build38_empty(f2, pkt++);
    writeData(f2);

    // 3) 0x38 vide
    std::vector<uint8_t> f3; build38_empty(f3, pkt++);
    writeData(f3);

    // 4) 0x2A commit (avec suffixe où le "02" devient "03")
    std::vector<uint8_t> fc; build2A_commit(fc, pkt++);
    writeData(fc);
}
