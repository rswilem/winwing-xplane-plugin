#include "segments.h"
#include <algorithm>

namespace pap3::lcd::segments {

// Seven-seg mask per digit using logical segments A..G mapped to bits:
// A=bit0 (top), B=bit1 (top-right), C=bit2 (bottom-right),
// D=bit3 (bottom), E=bit4 (bottom-left), F=bit5 (top-left), G=bit6 (middle).
static constexpr uint8_t A=1u<<0, B=1u<<1, C=1u<<2, D=1u<<3, E=1u<<4, F=1u<<5, G=1u<<6;

static constexpr uint8_t kDigitMask[10] = {
    /*0*/ (A|B|C|D|E|F),
    /*1*/ (B|C),
    /*2*/ (A|B|G|E|D),
    /*3*/ (A|B|C|D|G),
    /*4*/ (F|G|B|C),
    /*5*/ (A|F|G|C|D),
    /*6*/ (A|F|E|D|C|G),
    /*7*/ (A|B|C),
    /*8*/ (A|B|C|D|E|F|G),
    /*9*/ (A|B|C|D|F|G)
};
// Mask for the letter 'A' (segments A,B,C,E,F,G ON, D OFF)
static constexpr uint8_t kLetterA = (A|B|C|E|F|G);

// Index helper: absolute offset [0x19..0x38] -> payload index [0..31]
static inline int idx(uint8_t absOff) {
    return static_cast<int>(absOff) - 0x19;
}

// Writes one segment position in the device order: [Mid,TopL,BotL,Bot,BotR,TopR,Top]
// Maps to logical bits: [G, F,   E,    D,   C,    B,    A]
static inline void applyOnePos(Payload& p, uint8_t absOff, uint8_t flag, bool on) {
    if (!on) return;
    const int i = idx(absOff);
    if (i < 0 || i >= kPayloadSize) return;
    p[static_cast<size_t>(i)] |= flag;
}

void clear(Payload& p) {
    p.fill(0);
}

void drawDigit(const GroupOffsets& g, Payload& p, uint8_t flag, int digit) {
    digit = std::clamp(digit, 0, 9);
    const uint8_t m = kDigitMask[digit];

    // Device order: [Mid, TopL, BotL, Bot, BotR, TopR, Top]
    // Logical bits:  [ G ,  F  ,  E  ,  D ,  C  ,  B  ,  A ]
    applyOnePos(p, g.mid,  flag, (m & G) != 0);
    applyOnePos(p, g.topL, flag, (m & F) != 0);
    applyOnePos(p, g.botL, flag, (m & E) != 0);
    applyOnePos(p, g.bot,  flag, (m & D) != 0);
    applyOnePos(p, g.botR, flag, (m & C) != 0);
    applyOnePos(p, g.topR, flag, (m & B) != 0);
    applyOnePos(p, g.top,  flag, (m & A) != 0);
}

void setFlag(Payload& p, uint8_t absOffset, uint8_t mask, bool enable) {
    const int i = idx(absOffset);
    if (i < 0 || i >= kPayloadSize) return;
    if (enable) p[static_cast<size_t>(i)] |= mask;
    // No clear-path here by design; caller composes from an empty payload every time.
}

void drawLetterA(const GroupOffsets& g, Payload& p, uint8_t flag) {
    // Device order: [Mid, TopL, BotL, Bot, BotR, TopR, Top]
    // Logical bits: [ G ,  F  ,  E  ,  D ,  C  ,  B  ,  A ]
    applyOnePos(p, g.mid,  flag, (kLetterA & G) != 0);
    applyOnePos(p, g.topL, flag, (kLetterA & F) != 0);
    applyOnePos(p, g.botL, flag, (kLetterA & E) != 0);
    applyOnePos(p, g.bot,  flag, (kLetterA & D) != 0);
    applyOnePos(p, g.botR, flag, (kLetterA & C) != 0);
    applyOnePos(p, g.topR, flag, (kLetterA & B) != 0);
    applyOnePos(p, g.top,  flag, (kLetterA & A) != 0);
}

} // namespace pap3::lcd::segments