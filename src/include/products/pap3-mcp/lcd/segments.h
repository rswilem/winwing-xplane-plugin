#pragma once
#include <array>
#include <cstdint>

/*
 * PAP3 MCP – LCD segments mapping
 *
 * The LCD payload occupies absolute offsets 0x19..0x38 (inclusive), i.e. 32 bytes.
 * Each decimal digit is drawn by setting bits across SEVEN distinct byte offsets
 * that correspond to seven-segment parts. The order of these seven offsets is:
 *
 *   [Mid, TopL, BotL, Bot, BotR, TopR, Top]
 *
 * The device uses FOUR "groups" of seven offsets (one group per line/area):
 *
 *   G0: 0x1D,0x21,0x25,0x29,0x2D,0x31,0x35  -> SPD & CAPT_CRS digits share these bytes
 *   G1: 0x1E,0x22,0x26,0x2A,0x2E,0x32,0x36  -> HDG & ALT(high) digits share these bytes
 *   G2: 0x1F,0x23,0x27,0x2B,0x2F,0x33,0x37  -> VSPD & ALT(low) digits share these bytes
 *   G3: 0x20,0x24,0x28,0x2C,0x30,0x34,0x38  -> FO_CRS digits share these bytes
 *
 * Within a group, each digit position uses a distinct BIT in each of those seven bytes.
 * For example, in G0 the bit 0x04 represents SPD HUNDREDS; to draw digit '5' in SPD
 * HUNDREDS, the '5' segment pattern is applied to the seven offsets of G0 with bit 0x04.
 *
 * Flags per group (as provided):
 *
 *  G0 flags (SPD & CAPT_CRS):
 *    SPD_UNITS      = 0x01
 *    SPD_TENS       = 0x02
 *    SPD_HUNDREDS   = 0x04
 *    SPD_KILO       = 0x08
 *    CPT_CRS_UNITS  = 0x20
 *    CPT_CRS_TENS   = 0x40
 *    CPT_CRS_HUNDREDS = 0x80
 *
 *  G1 flags (HDG & ALT high digits):
 *    ALT_HUNDREDS   = 0x01
 *    ALT_KILO       = 0x02
 *    ALT_TENS_KILO  = 0x04
 *    HDG_UNITS      = 0x10
 *    HDG_TENS       = 0x20
 *    HDG_HUNDREDS   = 0x40
 *
 *  G2 flags (VSPD & ALT low digits):
 *    VSPD_UNITS     = 0x01
 *    VSPD_TENS      = 0x02
 *    VSPD_HUNDREDS  = 0x04
 *    VSPD_KILO      = 0x08
 *    ALT_UNITS      = 0x40
 *    ALT_TENS       = 0x80
 *
 *  G3 flags (FO_CRS):
 *    FO_CRS_UNITS   = 0x10
 *    FO_CRS_TENS    = 0x20
 *    FO_CRS_HUNDREDS= 0x40
 *
 * Labels / dots / signs (absolute offsets and bit masks):
 *   0x19: 0x04 = SPD decimal dot, 0x20 = CAPT_CRS final dot
 *   0x26: 0x08 = HDG final dot
 *   0x1A: 0x01 = ALT decimal dot
 *   0x1B: 0x04 = VSPD decimal dot
 *   0x1C: 0x10 = FO_CRS final dot
 *   0x1E: 0x80 = SPD A/B sign bottom-mid bar
 *   0x22: 0x80 = SPD A/B sign top-mid bar
 *   0x1F: 0x10 = VSPD minus sign
 *   0x28: 0x80 = VSPD plus sign bottom bar
 *   0x2C: 0x80 = VSPD plus sign top bar
 *   0x34: 0x80 = FPA label (VSPD line)
 *   0x38: 0x80 = V/S label (VSPD line)
 *   0x36: 0x08 = HDG label (left),  0x80 = IAS label (SPD line)
 *   0x32: 0x08 = HDG label (right), 0x80 = MACH label (left half, SPD)
 *   0x2E: 0x08 = TRK label (left),  0x80 = MACH label (right half, SPD)
 *   0x2A: 0x08 = TRK label (right)
 *
 * The builder below operates on a 32-byte buffer where index 0 corresponds
 * to absolute offset 0x19 (so absOffset - 0x19 is the index in the array).
 */

namespace pap3::lcd::segments {

/// Size of the LCD payload [0x19..0x38] inclusive.
inline constexpr int kPayloadSize = 0x38 - 0x19 + 1; // 32

/// Convenience alias for the LCD payload buffer (zero-based over 0x19..0x38).
using Payload = std::array<uint8_t, kPayloadSize>;

/// Absolute offsets ordered as [Mid, TopL, BotL, Bot, BotR, TopR, Top]
struct GroupOffsets {
    uint8_t mid, topL, botL, bot, botR, topR, top;
};

/// Four segment groups (absolute offsets)
inline constexpr GroupOffsets G0 {0x1D,0x21,0x25,0x29,0x2D,0x31,0x35}; // SPD + CAPT_CRS
inline constexpr GroupOffsets G1 {0x1E,0x22,0x26,0x2A,0x2E,0x32,0x36}; // HDG + ALT(hi)
inline constexpr GroupOffsets G2 {0x1F,0x23,0x27,0x2B,0x2F,0x33,0x37}; // VSPD + ALT(lo)
inline constexpr GroupOffsets G3 {0x20,0x24,0x28,0x2C,0x30,0x34,0x38}; // FO_CRS

// ---- Flags per group -------------------------------------------------

// G0 (SPD & CAPT_CRS)
inline constexpr uint8_t SPD_UNITS         = 0x01;
inline constexpr uint8_t SPD_TENS          = 0x02;
inline constexpr uint8_t SPD_HUNDREDS      = 0x04;
inline constexpr uint8_t SPD_KILO          = 0x08;
inline constexpr uint8_t CPT_CRS_UNITS     = 0x20;
inline constexpr uint8_t CPT_CRS_TENS      = 0x40;
inline constexpr uint8_t CPT_CRS_HUNDREDS  = 0x80;

// G1 (HDG & ALT high)
inline constexpr uint8_t ALT_HUNDREDS      = 0x01;
inline constexpr uint8_t ALT_KILO          = 0x02;
inline constexpr uint8_t ALT_TENS_KILO     = 0x04;
inline constexpr uint8_t HDG_UNITS         = 0x10;
inline constexpr uint8_t HDG_TENS          = 0x20;
inline constexpr uint8_t HDG_HUNDREDS      = 0x40;

// G2 (VSPD & ALT low)
inline constexpr uint8_t VSPD_UNITS        = 0x01;
inline constexpr uint8_t VSPD_TENS         = 0x02;
inline constexpr uint8_t VSPD_HUNDREDS     = 0x04;
inline constexpr uint8_t VSPD_KILO         = 0x08;
inline constexpr uint8_t ALT_UNITS         = 0x40;
inline constexpr uint8_t ALT_TENS          = 0x80;

// G3 (FO_CRS)
inline constexpr uint8_t FO_CRS_UNITS      = 0x10;
inline constexpr uint8_t FO_CRS_TENS       = 0x20;
inline constexpr uint8_t FO_CRS_HUNDREDS   = 0x40;

// ---- Labels / dots / signs (absolute offsets + masks) ---------------

inline constexpr uint8_t OFF_19 = 0x19;
inline constexpr uint8_t OFF_1A = 0x1A;
inline constexpr uint8_t OFF_1B = 0x1B;
inline constexpr uint8_t OFF_1C = 0x1C;
inline constexpr uint8_t OFF_1E = 0x1E;
inline constexpr uint8_t OFF_1F = 0x1F;
inline constexpr uint8_t OFF_22 = 0x22;
inline constexpr uint8_t OFF_26 = 0x26;
inline constexpr uint8_t OFF_28 = 0x28;
inline constexpr uint8_t OFF_2A = 0x2A;
inline constexpr uint8_t OFF_2C = 0x2C;
inline constexpr uint8_t OFF_2E = 0x2E;
inline constexpr uint8_t OFF_32 = 0x32;
inline constexpr uint8_t OFF_34 = 0x34;
inline constexpr uint8_t OFF_36 = 0x36;
inline constexpr uint8_t OFF_38 = 0x38;

// dots
inline constexpr uint8_t DOT_SPD      = 0x04; // at 0x19
inline constexpr uint8_t DOT_CPT_CRS  = 0x20; // at 0x19 (final)
inline constexpr uint8_t DOT_HDG      = 0x08; // at 0x26 (final)
inline constexpr uint8_t DOT_ALT      = 0x01; // at 0x1A
inline constexpr uint8_t DOT_VSPD     = 0x04; // at 0x1B
inline constexpr uint8_t DOT_FO_CRS   = 0x10; // at 0x1C (final)

// SPD "A/B" bars
inline constexpr uint8_t SPD_BAR_BOTTOM = 0x80; // at 0x1E
inline constexpr uint8_t SPD_BAR_TOP    = 0x80; // at 0x22

// VSPD signs
inline constexpr uint8_t VSPD_MINUS     = 0x10; // at 0x1F
inline constexpr uint8_t VSPD_PLUS_BOT  = 0x80; // at 0x28
inline constexpr uint8_t VSPD_PLUS_TOP  = 0x80; // at 0x2C

// Labels
inline constexpr uint8_t LBL_FPA        = 0x80; // at 0x34
inline constexpr uint8_t LBL_VS         = 0x80; // at 0x38
inline constexpr uint8_t LBL_HDG_L      = 0x08; // at 0x36 (left)
inline constexpr uint8_t LBL_HDG_R      = 0x08; // at 0x32 (right)
inline constexpr uint8_t LBL_TRK_L      = 0x08; // at 0x2E (left)
inline constexpr uint8_t LBL_TRK_R      = 0x08; // at 0x2A (right)
inline constexpr uint8_t LBL_IAS        = 0x80; // at 0x36
inline constexpr uint8_t LBL_MACH_L     = 0x80; // at 0x32
inline constexpr uint8_t LBL_MACH_R     = 0x80; // at 0x2E

// ---- API -------------------------------------------------------------

/// Clears the 32-byte payload to all zeros.
void clear(Payload& p);

/// Sets a single decimal digit (0..9) into a group using the provided flag.
/// The digit's seven segments are applied to that flag across the group's offsets.
/// Example: drawDigit(G0, p, SPD_HUNDREDS, 5);
void drawDigit(const GroupOffsets& g, Payload& p, uint8_t flag, int digit);

void drawLetterA(const GroupOffsets& g, Payload& p, uint8_t flag);

/// Utility to set/clear a single bit at an absolute offset.
void setFlag(Payload& p, uint8_t absOffset, uint8_t mask, bool enable);

} // namespace pap3::lcd::segments