// src/include/products/pap3-mcp/device/inputs.cpp
//
// PAP3 MCP - HID input decoding
// ------------------------------
// This module decodes the raw 64-byte (or device-specific) HID input report into
// higher-level events for observers (raw frame, button bytes, switch edges,
// encoders deltas, and light sensor).
//
// Public API and behavior are preserved. The code is reorganized for clarity:
//  • Declarative mapping table for button names (offset + bitmask → label)
//  • Tight, documented decode() pipeline
//  • Small helpers with precise intent
//
// Notes:
//  • All constants/types (kInputBytes, kHeader, kBtnCount, kBtnStart, kEnc*
//    ButtonsFrame, EncodersFrame, delta8, etc.) are defined in inputs.h.
//

#include "inputs.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace pap3::device {

    // -----------------------------------------------------------------------------
    // Button name lookup
    // -----------------------------------------------------------------------------
    //
    // The device reports button states over a few consecutive bytes (offsets 0x01..0x06).
    // Each bit inside those bytes corresponds to a specific button. We keep the
    // original mapping but express it declaratively in a compact table.
    //
    // nameForButton(off, m) expects a single-bit mask `m` at a given byte offset `off`.
    // Returns a stable string literal or nullptr if (off, m) is not recognized.
    //

    namespace {
        struct ButtonMap {
                uint8_t off;  // byte offset in the input report
                uint8_t mask; // single-bit mask within that byte
                const char *name;
        };

        // Offsets 0x01..0x06 with their bit meanings,
        // exactly matching the original logic.
        constexpr ButtonMap kButtonMap[] = {
            // ---- 0x01
            {0x01, 0x01, "N1"},
            {0x01, 0x02, "SPEED"},
            {0x01, 0x04, "VNAV"},
            {0x01, 0x08, "LVL CHG"},
            {0x01, 0x10, "HDG SEL"},
            {0x01, 0x20, "LNAV"},
            {0x01, 0x40, "VOR LOC"},
            {0x01, 0x80, "APP"},

            // ---- 0x02
            {0x02, 0x01, "ALT HLD"},
            {0x02, 0x02, "V/S"},
            {0x02, 0x04, "CMD A"},
            {0x02, 0x08, "CWS A"},
            {0x02, 0x10, "CMD B"},
            {0x02, 0x20, "CWS B"},
            {0x02, 0x40, "C/O (IAS/MACH)"},
            {0x02, 0x80, "SPD INTV"},

            // ---- 0x03
            {0x03, 0x01, "ALT INTV"},
            {0x03, 0x02, "CAPT CRS DOWN"},
            {0x03, 0x04, "CAPT CRS UP"},
            {0x03, 0x08, "SPD DOWN"},
            {0x03, 0x10, "SPD UP"},
            {0x03, 0x20, "HDG DOWN"},
            {0x03, 0x40, "HDG UP"},
            {0x03, 0x80, "ALT DOWN"},

            // ---- 0x04
            {0x04, 0x01, "ALT UP"},
            {0x04, 0x02, "FO CRS DOWN"},
            {0x04, 0x04, "FO CRS UP"},
            {0x04, 0x08, "CAPT F/D OFF"},
            {0x04, 0x20, "FO F/D OFF"},
            {0x04, 0x80, "DISENGAGE SWITCH UP"},

            // ---- 0x05
            {0x05, 0x01, "DISENGAGE SWITCH DOWN"},
            {0x05, 0x02, "BANK 10"},
            {0x05, 0x04, "BANK 15"},
            {0x05, 0x08, "BANK 20"},
            {0x05, 0x10, "BANK 25"},
            {0x05, 0x20, "BANK 30"},
            {0x05, 0x40, "V/S DOWN"},
            {0x05, 0x80, "V/S UP"},

            // ---- 0x06
            {0x06, 0x01, "A/T ARMED"},
            {0x06, 0x02, "A/T DISARMED"},
        };
    } // anonymous namespace

    const char *nameForButton(uint8_t off, uint8_t m) {
        // Expect a single-bit mask. If callers pass multi-bit masks, this will just not match.
        for (const auto &e : kButtonMap) {
            if (e.off == off && e.mask == m) {
                return e.name;
            }
        }
        return nullptr;
    }

    // -----------------------------------------------------------------------------
    // Inputs::decode
    // -----------------------------------------------------------------------------
    //
    // Decoding pipeline (executed when a new report arrives):
    //  1) Validate header/length
    //  2) Copy to a local "now" buffer (guard against caller aliasing)
    //  3) Notify raw observers
    //  4) Extract and forward the 5-button-byte block (0x01..0x05 inclusive, plus 0x06)
    //  5) Edge detection for selected offsets (watched set)
    //  6) Encoders deltas (6 channels)
    //  7) Light sensor value (optional, configurable offset)
    //  8) Update last-frame cache for subsequent edge/delta computations
    //
    // Notes:
    //  • Edge detection computes both 0→1 (set) and 1→0 (clear) masks.
    //  • Encoders use delta8() to derive signed steps from rolling counters.
    //

    bool Inputs::decode(const uint8_t *report, int len) {
        // 1) Basic validation
        if (!report || len < kInputBytes) {
            return false;
        }
        if (report[0] != kHeader) {
            return false;
        }

        // 2) Snapshot current frame (protect against caller reusing 'report' after return)
        uint8_t now[kInputBytes] = {0};
        std::memcpy(now, report, kInputBytes);

        // 3) Raw frame callback first (full visibility)
        if (onRaw) {
            onRaw(now);
        }

        // 4) Buttons block (bytes starting at kBtnStart, length kBtnCount)
        if (onButtons) {
            ButtonsFrame bf{};
            for (uint8_t i = 0; i < kBtnCount; ++i) {
                bf.bytes[i] = now[kBtnStart + i];
            }
            onButtons(bf);
        }

        // 5) Switch edge detection, if we have a previous frame
        if (onSwitchEdges && _hasLast) {
            for (uint8_t off : _watched) {
                if (off >= kInputBytes) {
                    continue;
                }
                const uint8_t prev = _last[off];
                const uint8_t cur = now[off];
                const uint8_t diff = static_cast<uint8_t>(prev ^ cur);
                if (!diff) {
                    continue;
                }

                // Bits that changed to 1 (0→1)
                const uint8_t changedSet = static_cast<uint8_t>(~prev & cur);
                // Bits that changed to 0 (1→0)
                const uint8_t changedClr = static_cast<uint8_t>(prev & ~cur);

                onSwitchEdges(off, changedSet, changedClr);
            }
        }

        // 6) Encoders deltas (require a previous frame)
        if (onEncoders && _hasLast) {
            EncodersFrame ef{};
            ef.d_crsC = delta8(now[kEncCrsC], _last[kEncCrsC]);
            ef.d_spd = delta8(now[kEncSpd], _last[kEncSpd]);
            ef.d_hdg = delta8(now[kEncHdg], _last[kEncHdg]);
            ef.d_alt = delta8(now[kEncAlt], _last[kEncAlt]);
            ef.d_vvi = delta8(now[kEncVvi], _last[kEncVvi]);
            ef.d_crsF = delta8(now[kEncCrsF], _last[kEncCrsF]);
            if (ef.any()) {
                onEncoders(ef);
            }
        }

        // 7) Light sensor (single byte) — optional, offset set by user/config
        if (onLightSensor && _lsOffset != 0xFF && _lsOffset < kInputBytes) {
            const uint8_t lux = now[_lsOffset];
            onLightSensor(lux);
        }

        // 8) Update cache for next frame's edge/delta computations
        std::memcpy(_last, now, kInputBytes);
        _hasLast = true;

        return true;
    }

} // namespace pap3::device