// src/include/products/pap3-mcp/device/illumination.cpp
//
// PAP3 MCP - Illumination Engine
// -------------------------------
// This module converts configuration + live inputs (aircraft percents,
// light sensor, power state) into 8-bit channel outputs for:
//   • Backlight
//   • LCD
//   • LEDs
//
// Notes:
//   • clampPct(), IllumMode, ChannelConfig, IlluminationConfig, IlluminationInputs,
//     IlluminationOutputs are declared in illumination.h.
//   • percentToByte() rounds to nearest and maps 100% → 255 (full scale).

#include "illumination.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace pap3::device {

    // -----------------------------------------------------------------------------
    // Static helpers
    // -----------------------------------------------------------------------------

    /**
     * Convert a [0..100] percent to an 8-bit value [0..255], rounding to nearest.
     * Input is clamped to [0..100] using clampPct().
     * Guarantees: 0% → 0, 100% → 255.
     */
    uint8_t IlluminationEngine::percentToByte(int pct) {
        pct = clampPct(pct);

        // Use double for the ratio, then round-to-nearest.
        // Example: 100 * 255 / 100 = 255 ;  50% → ~127.5 → 128 after lround.
        const double scaled = (static_cast<double>(pct) * 255.0) / 100.0;
        const int v = static_cast<int>(std::lround(scaled));

        // Safety clamp to byte range (defensive, should already be within bounds).
        return static_cast<uint8_t>(std::max(0, std::min(255, v)));
    }

    /**
     * Pick the source percentage for a given channel according to the configured mode,
     * then convert to an 8-bit output using percentToByte().
     *
     * Modes:
     *   - Fixed:       use channel's fixedPercent
     *   - Aircraft:    use the aircraft-provided percent for this channel
     *   - LightSensor: use the (already percent-normalized) light sensor value
     *   - default:     fall back to 100% (full brightness)
     */
    uint8_t IlluminationEngine::pickSource(IllumMode mode,
                                           const ChannelConfig &cc,
                                           const IlluminationInputs &in,
                                           int aircraftPctForChannel) {
        int pct = 100; // default fallback: full brightness

        switch (mode) {
            case IllumMode::Fixed:
                pct = clampPct(cc.fixedPercent);
                break;

            case IllumMode::Aircraft:
                pct = clampPct(aircraftPctForChannel);
                break;

            case IllumMode::LightSensor:
                pct = clampPct(in.lightSensorPct);
                break;

            default:
                // Keep pct = 100
                break;
        }

        return percentToByte(pct);
    }

    // -----------------------------------------------------------------------------
    // Configuration & live inputs
    // -----------------------------------------------------------------------------

    /**
     * Set the illumination configuration (modes + per-channel fixed percents).
     * Does NOT reset last frame state: if the config change impacts outputs,
     * the next compute() will naturally report differences via 'changed'.
     */
    void IlluminationEngine::setConfig(const IlluminationConfig &cfg) {
        _cfg = cfg;
    }

    /**
     * Set aircraft-provided percents (already 0..100 domain). Values are clamped.
     * These feed the "Aircraft" mode for each channel.
     */
    void IlluminationEngine::setAircraftPercents(int backlightPct, int lcdPct, int ledsPct) {
        _in.aircraftBacklightPct = clampPct(backlightPct);
        _in.aircraftLcdPct = clampPct(lcdPct);
        _in.aircraftLedsPct = clampPct(ledsPct);
    }

    /**
     * Set the normalized light sensor percentage (0..100). Value is clamped.
     * Used when channel mode is "LightSensor".
     */
    void IlluminationEngine::setLightSensorPercent(int pct) {
        _in.lightSensorPct = clampPct(pct);
    }

    /**
     * Indicate whether power is available. When power is not available,
     * all channel raw outputs are forced to 0 (power gating).
     */
    void IlluminationEngine::setPowerAvailable(bool available) {
        _in.powerAvailable = available;
    }

    // -----------------------------------------------------------------------------
    // Main compute
    // -----------------------------------------------------------------------------

    /**
     * Compute raw 8-bit outputs for the three channels and report which ones changed
     * since the previous call.
     *
     * Steps:
     *   1) Resolve per-channel raw values from mode + inputs
     *   2) Power gating: if power is off, force all channels to 0
     *   3) Change detection: on first run, mark all changed; afterwards, compare
     *
     * Returns:
     *   IlluminationOutputs {
     *     std::array<uint8_t,3> raw;     // raw0, raw1, raw2
     *     std::array<bool,3>    changed; // changed flags per channel
     *   }
     */
    IlluminationOutputs IlluminationEngine::compute() {
        IlluminationOutputs out{};

        // 1) Resolve raw outputs per channel from mode + inputs
        uint8_t raw0 = pickSource(_cfg.backlight.mode, _cfg.backlight, _in, _in.aircraftBacklightPct);
        uint8_t raw1 = pickSource(_cfg.lcd.mode, _cfg.lcd, _in, _in.aircraftLcdPct);
        uint8_t raw2 = pickSource(_cfg.leds.mode, _cfg.leds, _in, _in.aircraftLedsPct);

        // 2) Power gating: if no power available, outputs are zeroed
        if (!_in.powerAvailable) {
            raw0 = 0;
            raw1 = 0;
            raw2 = 0;
        }

        out.raw = {raw0, raw1, raw2};

        // 3) Change detection
        if (!_hasLast) {
            // First call: consider all channels as changed
            out.changed = {true, true, true};
            _lastRaw = out.raw;
            _hasLast = true;
            return out;
        }

        out.changed[0] = (out.raw[0] != _lastRaw[0]);
        out.changed[1] = (out.raw[1] != _lastRaw[1]);
        out.changed[2] = (out.raw[2] != _lastRaw[2]);

        _lastRaw = out.raw;
        return out;
    }

} // namespace pap3::device