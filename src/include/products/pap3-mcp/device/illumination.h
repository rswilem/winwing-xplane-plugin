#pragma once
#include <array>
#include <cstdint>
#include <algorithm>

/**
 * PAP3 MCP — Illumination model (device layer)
 * --------------------------------------------
 * This module encapsulates the logic for the three illumination channels:
 *   - Backlight  (channel 0)
 *   - LCD        (channel 1)
 *   - LEDs       (channel 2)
 *
 * Each channel can run in one of three modes:
 *   1) Fixed value      (uses a user-chosen percentage)
 *   2) Aircraft driven  (uses percentages provided by the active aircraft profile)
 *   3) Light sensor     (uses a percentage provided by the device's ambient light sensor)
 *
 * Inputs are provided as percentages (0..100). Outputs are 8-bit raw values (0..255)
 * suitable to feed the USB transport (dimming frames).
 *
 * Responsibilities kept here on purpose:
 *   - Per-channel mode selection
 *   - Percent→byte mapping with clamping and rounding
 *   - Simple change detection (to avoid spamming the USB bus)
 *   - Optional power gating (when power is not available, all channels return 0)
 *
 * Responsibilities intentionally NOT handled here:
 *   - Persistence (INI)          → menu subsystem
 *   - USB I/O and frame formats  → transport subsystem
 *   - Reading aircraft datarefs  → aircraft/profile layer
 *   - Reading the light sensor   → inputs/decoding layer
 *
 * Integration pattern:
 *   - Set configuration with setConfig().
 *   - Feed dynamic inputs every frame or when they change:
 *       * setAircraftPercents(...)
 *       * setLightSensorPercent(...)
 *       * setPowerAvailable(...)
 *   - Call compute() to obtain the three channel raw bytes and "changed" flags.
 */

namespace pap3::device {

/// Illumination operating mode for a channel.
enum class IllumMode : uint8_t {
    Fixed        = 0,
    Aircraft     = 1,
    LightSensor  = 2
};

/// Per-channel configuration.
struct ChannelConfig {
    IllumMode mode = IllumMode::Aircraft;
    int fixedPercent = 100;  // 0..100, clamped on use
};

/// Whole-device configuration (three channels).
struct IlluminationConfig {
    ChannelConfig backlight; // channel 0
    ChannelConfig lcd;       // channel 1
    ChannelConfig leds;      // channel 2
};

/// Dynamic inputs coming from other subsystems.
struct IlluminationInputs {
    // Percentages provided by the aircraft/profile, per channel (0..100).
    // If a channel is not aircraft-driven, these values are ignored.
    int aircraftBacklightPct = 100;
    int aircraftLcdPct       = 100;
    int aircraftLedsPct      = 100;

    // Percentage provided by the device's ambient light sensor (0..100).
    int lightSensorPct       = 100;

    // Power gate. When false, outputs are forced to 0 regardless of modes.
    bool powerAvailable      = true;
};

/// Outputs returned by the engine in one step.
struct IlluminationOutputs {
    // Raw 0..255 values to send to dimming channels 0/1/2.
    std::array<uint8_t, 3> raw = {255, 255, 255};

    // Per-channel change indicators since the previous compute() call.
    // These can be used to avoid sending redundant USB dimming frames.
    std::array<bool, 3> changed = {true, true, true};
};

class IlluminationEngine {
public:
    IlluminationEngine() = default;

    // Configuration
    void setConfig(const IlluminationConfig& cfg);
    const IlluminationConfig& config() const { return _cfg; }

    // Dynamic inputs (idempotent setters; values are clamped on read)
    void setAircraftPercents(int backlightPct, int lcdPct, int ledsPct);
    void setLightSensorPercent(int pct);
    void setPowerAvailable(bool available);

    // Computes the three channel outputs. Change detection uses the last
    // returned raw values; the first call marks all channels as changed.
    IlluminationOutputs compute();

    // Utility mapping exposed for unit tests.
    static uint8_t percentToByte(int pct);

private:
    static int clampPct(int pct) {
        return std::max(0, std::min(100, pct));
    }

    static uint8_t pickSource(IllumMode mode,
                              const ChannelConfig& cc,
                              const IlluminationInputs& in,
                              int aircraftPctForChannel);

private:
    IlluminationConfig _cfg{};
    IlluminationInputs _in{};

    // Last raw values used for change detection.
    bool _hasLast = false;
    std::array<uint8_t,3> _lastRaw{ {0,0,0} };
};

} // namespace pap3::device