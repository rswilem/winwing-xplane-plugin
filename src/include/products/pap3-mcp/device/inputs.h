#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <cstring>

namespace pap3::device {

    const char* nameForButton(uint8_t offset, uint8_t bitMask);


/**
 * PAP3 HID input decoder.
 *
 * The PAP3 sends fixed-size input reports. This decoder takes a raw HID OUT
 * report, validates the header, maintains a "last frame" cache, and emits:
 *   - Raw 0x20 bytes snapshot (for consumers that want full control)
 *   - Button blocks 0x01..0x06 as 6 bytes (bitmasks)
 *   - Switch bit edges (per-offset changed bits: set and cleared)
 *   - Encoder deltas (-128..+127) computed from wrap-around byte counters
 *   - Optional light sensor byte (offset can be configured)
 *
 * Nothing in this class touches X-Plane APIs nor device I/O; it is pure decode.
 * All notifications are delivered via std::function callbacks if provided.
 */
class Inputs {
public:
    // ===== Configuration constants (match current understanding of PAP3) =====
    static constexpr uint8_t  kHeader      = 0x01;  // report[0] == 0x01 => input block
    static constexpr int      kInputBytes  = 0x20;  // total bytes in the report that matter
    static constexpr uint8_t  kBtnStart    = 0x01;  // first button byte offset
    static constexpr uint8_t  kBtnCount    = 0x06;  // number of button bytes (0x01..0x06 inclusive)
    static constexpr uint8_t  kEncCrsC     = 0x15;  // CAPT CRS position byte
    static constexpr uint8_t  kEncSpd      = 0x17;  // SPD position byte
    static constexpr uint8_t  kEncHdg      = 0x19;  // HDG position byte
    static constexpr uint8_t  kEncAlt      = 0x1B;  // ALT position byte
    static constexpr uint8_t  kEncVvi      = 0x1D;  // V/S position byte
    static constexpr uint8_t  kEncCrsF     = 0x1F;  // FO CRS position byte

    /**
     * The light sensor raw value is a single byte. The exact offset is
     * hardware/firmware dependent. This decoder defaults to 0x14 (one byte
     * before the first encoder position), but it can be overridden at runtime.
     */
    static constexpr uint8_t  kDefaultLightSensorOffset = 0x14;

    // ===== Payloads passed to callbacks =====
    struct ButtonsFrame {
        // 6 consecutive bytes starting at offset 0x01. Bit=1 means "pressed/on".
        uint8_t bytes[kBtnCount] = {0,0,0,0,0,0};

        // Convenience: returns true if any bit is set across the 6 bytes.
        bool any() const {
            return bytes[0] | bytes[1] | bytes[2] | bytes[3] | bytes[4] | bytes[5];
        }
    };

    struct EncodersFrame {
        // Signed deltas computed from wrap-around 8-bit counters.
        int8_t d_crsC = 0; // captain CRS
        int8_t d_spd  = 0; // IAS/MACH selector
        int8_t d_hdg  = 0; // heading selector
        int8_t d_alt  = 0; // altitude selector
        int8_t d_vvi  = 0; // vertical speed selector
        int8_t d_crsF = 0; // FO CRS

        bool any() const {
            return d_crsC | d_spd | d_hdg | d_alt | d_vvi | d_crsF;
        }
    };

    // ===== Callback signatures (optional) =====

    // Entire raw 0x20 bytes snapshot, including header. Size is guaranteed >= kInputBytes.
    std::function<void(const uint8_t now[kInputBytes])> onRaw;

    // Button bytes (0x01..0x06).
    std::function<void(const ButtonsFrame&)> onButtons;

    /**
     * Switch-bit edges. Called once per decode() if any bit in any watched offset changed.
     * Parameters:
     *  - off         : byte offset in [0, kInputBytes)
     *  - changedSet  : bits that transitioned 0->1 at this offset
     *  - changedClr  : bits that transitioned 1->0 at this offset
     *
     * Typical usage: subscribe to offsets 0x04..0x06 to track FD OFF / AP DISC / AT ARM bits.
     * Watched offsets can be configured with setWatchedSwitchOffsets().
     */
    std::function<void(uint8_t off, uint8_t changedSet, uint8_t changedClr)> onSwitchEdges;

    // Encoder deltas (-128..+127).
    std::function<void(const EncodersFrame&)> onEncoders;

    // Light sensor raw value [0..255]. Emitted only if the offset is in-range for the current report.
    std::function<void(uint8_t lux)> onLightSensor;

public:
    Inputs() = default;

    /**
     * Decodes a single HID report.
     * The function ignores reports that do not start with kHeader or that are shorter than kInputBytes.
     * On success, it emits callbacks in the following order (if provided):
     *   1) onRaw
     *   2) onButtons
     *   3) onSwitchEdges (once per watched offset having changes)
     *   4) onEncoders
     *   5) onLightSensor
     *
     * Returns true if the report was accepted and processed, false otherwise.
     */
    bool decode(const uint8_t* report, int len);

    /**
     * Returns a pointer to the last cached 0x20 bytes (including header) if available, else nullptr.
     * The pointer remains valid until the next decode() call.
     */
    const uint8_t* last() const { return _hasLast ? _last : nullptr; }

    /**
     * Configures the set of byte offsets to watch for bit-edge notifications.
     * Offsets out of range [0, kInputBytes) are ignored on decode().
     * Calling this replaces the previous set.
     */
    void setWatchedSwitchOffsets(std::vector<uint8_t> offsets) {
        _watched = std::move(offsets);
    }

    /**
     * Sets/overrides the light sensor offset. Use 0xFF to disable light sensor emission.
     */
    void setLightSensorOffset(uint8_t off) { _lsOffset = off; }

private:
    // Minimal signed delta on a uint8_t ring (difference in range [-128, +127]).
    static inline int8_t delta8(uint8_t now, uint8_t prev) {
        return static_cast<int8_t>(now - prev);
    }

    bool _hasLast = false;
    uint8_t _last[kInputBytes] = {0};

    // Offsets that should trigger onSwitchEdges when bits change.
    std::vector<uint8_t> _watched = {0x04, 0x05, 0x06};

    // Light sensor offset; 0xFF means disabled.
    uint8_t _lsOffset = kDefaultLightSensorOffset;
};

} // namespace pap3::device
