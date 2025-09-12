#pragma once
#include <array>
#include <cstdint>
#include "segments.h"

/*
 * PAP3 MCP – LCD compose
 *
 * High-level builder that converts a logical snapshot (numbers + labels)
 * into the 32-byte LCD payload occupying absolute offsets 0x19..0x38.
 *
 * The snapshot struct is intentionally minimal and device-agnostic. It
 * represents what should be rendered (values and label toggles), while
 * the segment-layer knows how to place bits at the right offsets.
 */

namespace pap3::lcd::compose {

using Payload = pap3::lcd::segments::Payload;

/// Logical snapshot used to render the LCDs.
struct Snapshot {
    // Numeric fields (expected ranges; values are clamped internally)
    float spd = 0;        // 0..9999 (drawn as 4 digits: K,H,T,U)
    int hdg = 0;        // 0..359  (drawn as 3 digits: H,T,U)
    int alt = 0;        // 0..99999 (drawn as 5 digits: 10k,k,h,t,u)
    int vvi = 0;        // -9999..9999 (sign & 4 digits abs)
    bool showVvi = false;
    int crsCapt = 0;    // 0..359  (3 digits)
    int crsFo   = 0;    // 0..359  (3 digits)
    bool digitA  = false; // Special MCP "A" digit
    bool digitB  = false; // Special MCP "8" digit

    // Dots (decimal / final)
    bool dotSpd      = false; // offset 0x19, bit 0x04
    bool dotAlt      = false; // offset 0x1A, bit 0x01
    bool dotVvi      = false; // offset 0x1B, bit 0x04
    bool dotCrsCapt  = false; // offset 0x19, bit 0x20 (final dot)
    bool dotCrsFo    = false; // offset 0x1C, bit 0x10 (final dot)
    bool dotHdg      = false; // offset 0x26, bit 0x08 (final dot)

    // SPD presentation
    bool lblIAS      = false; // 0x36,0x80
    bool lblMACH     = false; // 0x32,0x80 and 0x2E,0x80
    bool spdBarTop   = false; // 0x22,0x80
    bool spdBarBot   = false; // 0x1E,0x80

    // HDG/TRK labels (mutually exclusive in typical usage)
    bool lblHDG      = false; // 0x36,0x08 + 0x32,0x08
    bool lblTRK      = false; // 0x2E,0x08 + 0x2A,0x08

    // VSPD label (V/S or FPA)
    bool lblVS       = false; // 0x38,0x80
    bool lblFPA      = false; // 0x34,0x80
};

/// Builds the 32-byte LCD payload (absolute 0x19..0x38).
/// The returned buffer is ready to be copied at [0x19..] in the 0x38 LCD frame.
Payload build(const Snapshot& s);

} // namespace pap3::lcd::compose