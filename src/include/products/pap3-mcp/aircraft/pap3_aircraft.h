// pap3_aircraft.h
#pragma once

#include <cstdint>
#include <functional>
#include <string>

#ifndef PAP3_PRIORITY_LOCK
#define PAP3_PRIORITY_LOCK 1
#endif

namespace pap3 {
    namespace device {
        class PAP3Device;
    } // namespace device
} // namespace pap3

namespace pap3::aircraft {

    /// Canonical MCP power bits exposed by profiles (for mcpPowerMask()).
    /// Bit positions match the documentation below.
    namespace PowerBits {
        constexpr std::uint8_t MCP_POWER = 0x01; // Bit 0: AP has power
        constexpr std::uint8_t DC_BUS_1 = 0x02;  // Bit 1: DC Bus 1
        constexpr std::uint8_t DC_BUS_2 = 0x04;  // Bit 2: DC Bus 2
    } // namespace PowerBits

    // -----------------------------------------------------------------------------
    // State structure: canonical values for PAP3
    // -----------------------------------------------------------------------------
    /**
     * @brief Canonical MCP state as understood by PAP3.
     *
     */
    struct State {
            // --- MCP values ----------------------------------------------------------
            float spd = 0.0f;       ///< IAS/Mach window value (device expects float)
            int hdg = 0;            ///< Heading (degrees, 0..359 or sim-native domain)
            int alt = 0;            ///< Selected altitude (feet)
            float vvi = 0.0f;       ///< Vertical speed (fpm)
            bool vviVisible = true; ///< Whether V/S window is currently visible/active
            int crsCapt = 0;        ///< Captain course (degrees)
            int crsFo = 0;          ///< First Officer course (degrees)
            bool digitA = false;
            bool digitB = false;
            float mcpBrightness = 0.0f;
            float cockpitLights = 0.0f;
            float ledsBrightness = 1.0f;

            // --- LEDs ---------------------------------------------------------------
            /**
             * @brief LED annunciators on the MCP.
             *
             * Each boolean represents the desired ON/OFF state for the corresponding LED.
             * Naming follows common Boeing MCP conventions to avoid ambiguity.
             */
            struct {
                    bool N1 = false;
                    bool SPEED = false;
                    bool VNAV = false;
                    bool LVL_CHG = false;
                    bool HDG_SEL = false;
                    bool LNAV = false;
                    bool VORLOC = false;
                    bool APP = false;
                    bool ALT_HLD = false;
                    bool V_S = false;
                    bool CMD_A = false;
                    bool CWS_A = false;
                    bool CMD_B = false;
                    bool CWS_B = false;
                    bool AT_ARM = false;  ///< A/T ARM annunciator
                    bool MA_CAPT = false; ///< Master (A/P) Captain side
                    bool MA_FO = false;   ///< Master (A/P) First Officer side
            } led;
    };

    // -----------------------------------------------------------------------------
    // Interface for aircraft profiles
    // -----------------------------------------------------------------------------
    /**
     * @brief Abstract interface implemented by aircraft-specific profiles.
     *
     * A profile is responsible for:
     *  - Detecting whether it applies to the currently loaded aircraft (isEligible).
     *  - Subscribing to sim datarefs/vars and pushing updates (start/stop).
     *  - Exposing a synchronous snapshot (current).
     *  - Receiving device input hooks (buttons/encoders) and translating them
     *    into sim-side actions if desired.
     *
     * Lifecycle:
     *  - Construct concrete profile
     *  - Call isEligible(); if true, call start(callback)
     *  - Call stop() when no longer needed
     */

    class PAP3AircraftProfile {
        public:
            using StateCallback = std::function<void(const State &)>;

            virtual ~PAP3AircraftProfile() = default;

            /**
             * @brief Return true if this profile matches the loaded aircraft.
             * Typical implementation checks sim product name, datarefs presence, etc.
             */
            virtual bool isEligible() const = 0;

            /**
             * @brief Begin subscriptions to simulator data and push State updates.
             * The callback must be invoked whenever the canonical State changes.
             */
            virtual void start(StateCallback onChanged) = 0;

            /**
             * @brief Stop all simulator subscriptions / timers.
             * After stop(), no further callbacks should be issued.
             */
            virtual void stop() = 0;

            /**
             * @brief Return the current synchronous snapshot of State.
             * Should be cheap and thread-safe per profile's constraints.
             */
            virtual State current() const = 0;

            virtual void tick() {}

            // ----------------------
            // Input hooks (no-op by default)
            // ----------------------

            /**
             * @brief Notified once per bit edge detected on bytes 0x01..0x06.
             * @param off     Byte offset in the HID input report.
             * @param mask    Single-bit mask within that byte.
             * @param pressed True on 0→1 transitions, false on 1→0.
             *
             * Default: no-op.
             */
            virtual void onButton(std::uint8_t /*off*/, std::uint8_t /*mask*/, bool /*pressed*/) {}

            /**
             * @brief Notified for each encoder that reports a non-zero delta.
             * @param posOff Encoder "position" offset (e.g. 0x17 for SPD).
             * @param delta  Signed wrap-aware step in [-128, +127].
             *
             * Default: no-op.
             */
            virtual void onEncoderDelta(std::uint8_t /*posOff*/, std::int8_t /*delta*/) {}

            virtual void syncSimToHardwareFromRaw(const std::uint8_t * /*report*/, int /*len*/) {}

            /**
             * @brief Report MCP power-related bits as a mask.
             * Bits meanings:
             *   Bit 0 (0x01): AP has power
             *   Bit 1 (0x02): DC Bus 1
             *   Bit 2 (0x04): DC Bus 2
             *
             * Override if your aircraft provides finer power modeling.
             * Default: 0x01 (AP powered).
             */
            virtual std::uint8_t mcpPowerMask() const {
                return PowerBits::MCP_POWER;
            }

            /**
             * @brief Quick boolean shortcut for "MCP has power".
             * Device uses this to gate aircraft-driven illumination when available.
             * Default: true (powered).
             */
            virtual bool mcpHasPower() const {
                return true;
            }

            virtual void syncSimToHardware() {}

            void attachDevice(pap3::device::PAP3Device *dev) noexcept {
                _device = dev;
            }

        protected:
            // Back-pointer to the hardware device (may be null if not attached yet).
            pap3::device::PAP3Device *_device{nullptr};
    };

} // namespace pap3::aircraft
