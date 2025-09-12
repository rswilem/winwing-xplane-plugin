#include "zibo_profile.h"

#include "dataref.h"
#include "pap3_device.h"

#include <algorithm>
#include <cmath>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

namespace pap3::aircraft {

    ZiboPAP3Profile::ZiboPAP3Profile() {
        // Button bindings (press-only, unless noted). Offsets/masks per device map.
        auto B = [&](uint8_t off, uint8_t mask, const char *press, const char *release = nullptr) {
            _btns.push_back({off, mask, press, release});
        };

        // 0x01
        B(0x01, 0x01, "laminar/B738/autopilot/n1_press");
        B(0x01, 0x02, "laminar/B738/autopilot/speed_press");
        B(0x01, 0x04, "laminar/B738/autopilot/vnav_press");
        B(0x01, 0x08, "laminar/B738/autopilot/lvl_chg_press");
        B(0x01, 0x10, "laminar/B738/autopilot/hdg_sel_press");
        B(0x01, 0x20, "laminar/B738/autopilot/lnav_press");
        B(0x01, 0x40, "laminar/B738/autopilot/vorloc_press");
        B(0x01, 0x80, "laminar/B738/autopilot/app_press");

        // 0x02
        B(0x02, 0x01, "laminar/B738/autopilot/alt_hld_press");
        B(0x02, 0x02, "laminar/B738/autopilot/vs_press");
        B(0x02, 0x04, "laminar/B738/autopilot/cmd_a_press");
        B(0x02, 0x08, "laminar/B738/autopilot/cws_a_press");
        B(0x02, 0x10, "laminar/B738/autopilot/cmd_b_press");
        B(0x02, 0x20, "laminar/B738/autopilot/cws_b_press");
        B(0x02, 0x40, "laminar/B738/autopilot/change_over_press");
        B(0x02, 0x80, "laminar/B738/autopilot/spd_interv");

        // 0x03
        B(0x03, 0x01, "laminar/B738/autopilot/alt_interv");
        // The rest of 0x03/0x04/0x05 are handled as encoders or dedicated logic below if needed.

        // Encoder bindings
        auto E = [&](uint8_t posOff, const char *inc, const char *dec, int step = 1) {
            _encs.push_back({posOff, inc, dec, step});
        };

        // CRS CAPT
        E(0x15, "laminar/B738/autopilot/course_pilot_up", "laminar/B738/autopilot/course_pilot_dn");
        // SPD
        E(0x17, "sim/autopilot/airspeed_up", "sim/autopilot/airspeed_down");
        // HDG
        E(0x19, "laminar/B738/autopilot/heading_up", "laminar/B738/autopilot/heading_dn");
        // ALT
        E(0x1B, "laminar/B738/autopilot/altitude_up", "laminar/B738/autopilot/altitude_dn");
        // V/S
        E(0x1D, "sim/autopilot/vertical_speed_up", "sim/autopilot/vertical_speed_down");
        // CRS FO
        E(0x1F, "laminar/B738/autopilot/course_copilot_up", "laminar/B738/autopilot/course_copilot_dn");
    }

    ZiboPAP3Profile::~ZiboPAP3Profile() {
        stop();
    }

    bool ZiboPAP3Profile::isEligible() const {
        return Dataref::getInstance()->exists("laminar/B738/autopilot/mcp_speed_dial_kts_mach");
    }

    void ZiboPAP3Profile::start(StateCallback onChanged) {
        if (_running) {
            return;
        }
        _cb = std::move(onChanged);
        _running = true;
    }

    void ZiboPAP3Profile::stop() {
        if (!_running) {
            return;
        }
        _running = false;
    }

    pap3::aircraft::State ZiboPAP3Profile::current() const {
        return _state;
    }

    void ZiboPAP3Profile::tick() {
        if (!_running) {
            return;
        }
        poll();
    }

    void ZiboPAP3Profile::poll() {
        _state.spd = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/mcp_speed_dial_kts_mach");
        _state.hdg = Dataref::getInstance()->getCached<int>("laminar/B738/autopilot/mcp_hdg_dial");
        _state.alt = Dataref::getInstance()->getCached<int>("laminar/B738/autopilot/mcp_alt_dial");
        _state.vvi = Dataref::getInstance()->getCached<float>("sim/cockpit2/autopilot/vvi_dial_fpm");
        _state.vviVisible = Dataref::getInstance()->getCached<float>("sim/cockpit2/autopilot/vvi_dial_fpmShow") > 0.5f;
        _state.crsCapt = Dataref::getInstance()->getCached<int>("laminar/B738/autopilot/course_pilot");
        _state.crsFo = Dataref::getInstance()->getCached<int>("laminar/B738/autopilot/course_copilot");

        if (Dataref::getInstance()->exists("sim/cockpit2/electrical/instrument_brightness_ratio_manual")) {
            std::vector<float> brightness = Dataref::getInstance()->get<std::vector<float>>("sim/cockpit2/electrical/instrument_brightness_ratio_manual");
            _state.mcpBrightness = std::clamp(brightness[15], 0.0f, 1.0f);
        }

        if (Dataref::getInstance()->exists("laminar/B738/electric/panel_brightness")) {
            std::vector<float> brightness = Dataref::getInstance()->get<std::vector<float>>("laminar/B738/electric/panel_brightness");
            _state.cockpitLights = std::clamp(brightness[0], 0.0f, 1.0f);
            _state.ledsBrightness = std::max(brightness[0], 0.60f); // minimum 60% for LEDs
        }

        // LCD special digits (A / 8)
        _state.digitA = Dataref::getInstance()->get<float>("laminar/B738/mcp/digit_A") > 0.5f;
        _state.digitB = Dataref::getInstance()->get<float>("laminar/B738/mcp/digit_8") > 0.5f;

        _state.led.N1 = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/n1_status1") > 0.5f;
        _state.led.SPEED = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/speed_status1") > 0.5f;
        _state.led.VNAV = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/vnav_status1") > 0.5f;
        _state.led.LVL_CHG = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/lvl_chg_status") > 0.5f;
        _state.led.HDG_SEL = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/hdg_sel_status") > 0.5f;
        _state.led.LNAV = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/lnav_status") > 0.5f;
        _state.led.VORLOC = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/vorloc_status") > 0.5f;
        _state.led.APP = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/app_status") > 0.5f;
        _state.led.ALT_HLD = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/alt_hld_status") > 0.5f;
        _state.led.V_S = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/vs_status") > 0.5f;
        _state.led.CMD_A = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/cmd_a_status") > 0.5f;
        _state.led.CWS_A = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/cws_a_status") > 0.5f;
        _state.led.CMD_B = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/cmd_b_status") > 0.5f;
        _state.led.CWS_B = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/cws_b_status") > 0.5f;
        _state.led.AT_ARM = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/autothrottle_status1") > 0.5f;
        _state.led.MA_CAPT = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/master_capt_status") > 0.5f;
        _state.led.MA_FO = Dataref::getInstance()->getCached<float>("laminar/B738/autopilot/master_fo_status") > 0.5f;

        if (_cb) {
            _cb(_state);
        }

#if PAP3_PRIORITY_LOCK
        // Enforce maintained switches to follow hardware when priority is enabled.
        // Only toggle when sim position differs from desired hardware position.

        if (_haveHwSnapshot) {
            // FD CAPT
            maybeToggle("laminar/B738/autopilot/flight_director_pos", _hwFDCaptOn, "laminar/B738/autopilot/flight_director_toggle");

            // FD FO
            maybeToggle("laminar/B738/autopilot/flight_director_fo_pos", _hwFDFoOn, "laminar/B738/autopilot/flight_director_fo_toggle");

            // A/T ARM
            // Debounce is not strictly required here because we only toggle on mismatch.
            // If you still want to limit rate, you can reuse debounce(_lastAtToggleTime).
            // maybeToggle("laminar/B738/autopilot/autothrottle_arm_pos", _hwATOn, "laminar/B738/autopilot/autothrottle_arm_toggle");
            if ("laminar/B738/autopilot/autothrottle_arm_pos") {
                const bool simAT = Dataref::getInstance()->getCached<bool>("laminar/B738/autopilot/autothrottle_arm_pos"); // 0 = DISARMED, 1 = ARMED

                // If simulator shows ARMED and hardware is ARMED at that moment,
                // pulse the solenoid (non-blocking OFF->ON after ~50ms).
                // Debounce avoids pulsing every tick.
                if (!simAT && _hwATOn) {
                    _device->pulseATSolenoid(100);
                } else {
                    // Otherwise, enforce hardware-priority on the sim switch.
                    maybeToggle("laminar/B738/autopilot/autothrottle_arm_pos", _hwATOn, "laminar/B738/autopilot/autothrottle_arm_toggle");
                }
            }

            // A/P DISCONNECT latch (ENGAGED vs DISENGAGED as captured in _hwApDiscEngaged)
            maybeToggle("laminar/B738/autopilot/disconnect_pos", _hwApDiscEngaged, "laminar/B738/autopilot/disconnect_toggle");
        }
#endif
    }

    void ZiboPAP3Profile::repeatCmd(const char *inc, const char *dec, int8_t delta, int stepPerTick) {
        if (delta == 0) {
            return;
        }

        const bool up = (delta > 0);
        const int reps = std::max(1, static_cast<int>(std::abs(static_cast<int>(delta))) * std::max(1, stepPerTick));

        for (int i = 0; i < reps; ++i) {
            Dataref::getInstance()->executeCommand(up ? inc : dec);
        }
    }

    bool ZiboPAP3Profile::debounce(float &lastTs, float minDeltaSec) const {
        const float now = XPLMGetElapsedTime();
        if (now - lastTs < minDeltaSec) {
            return false;
        }
        lastTs = now;
        return true;
    }

    void ZiboPAP3Profile::maybeToggle(const char *dataref, bool desiredOn, const char *toggleCmd) {
        // Compare desired hardware state to current sim state and toggle only if needed.
        if (!dataref || !toggleCmd) {
            return;
        }
        const bool simOn = Dataref::getInstance()->get<bool>(dataref);
        if (simOn != desiredOn) {
            Dataref::getInstance()->executeCommand(toggleCmd);
        }
    }

    void ZiboPAP3Profile::onButton(uint8_t off, uint8_t mask, bool pressed) {
        // Maintained switches first

        // --- BANK ANGLE: 0x05 / {0x02,0x04,0x08,0x10,0x20} ---
        if (off == 0x05 && pressed) {
            int target = -1;
            switch (mask) {
                case 0x02:
                    target = 0;
                    break; // 10°
                case 0x04:
                    target = 1;
                    break; // 15°
                case 0x08:
                    target = 2;
                    break; // 20°
                case 0x10:
                    target = 3;
                    break; // 25°
                case 0x20:
                    target = 4;
                    break; // 30°
                default:
                    break;
            }
            if (target >= 0) {
                nudgeBankAngleTo(target);
                return; // ne pas retomber dans le mapping boutons générique
            }
        }

        // FD CAPT: 0x04 / 0x08  ("FD CAPT OFF" line). Pressed == switch ON, Released == switch OFF.
        if (off == 0x04 && mask == 0x08) {
            _hwFDCaptOn = pressed;
            _haveHwSnapshot = true;
            maybeToggle("laminar/B738/autopilot/flight_director_pos", _hwFDCaptOn, "laminar/B738/autopilot/flight_director_toggle");
            return;
        }

        // FD FO: 0x04 / 0x20 ("FD FO OFF" line). Pressed == switch ON, Released == switch OFF.
        if (off == 0x04 && mask == 0x20) {
            _hwFDFoOn = pressed;
            _haveHwSnapshot = true;
            maybeToggle("laminar/B738/autopilot/flight_director_fo_pos", _hwFDFoOn, "laminar/B738/autopilot/flight_director_fo_toggle");
            return;
        }

        // A/T ARM: 0x06 / 0x01 (ARMED) and 0x06 / 0x02 (DISARMED).
        // ON state when ARMED=pressed or DISARMED=released.
        if (off == 0x06 && (mask == 0x01 || mask == 0x02)) {
            if (mask == 0x01) {    // ARMED line
                _hwATOn = pressed; // pressed -> ON, released -> OFF
                _haveHwSnapshot = true;
            } else if (mask == 0x02) { // DISARMED line
                _hwATOn = !pressed;    // pressed -> OFF, released -> ON
            }
            if (debounce(_lastAtToggleTime)) {
                maybeToggle("laminar/B738/autopilot/autothrottle_arm_pos", _hwATOn, "laminar/B738/autopilot/autothrottle_arm_toggle");
            }
            return;
        }

        // A/P DISENGAGE two lines:
        //  - 0x04 / 0x80 : UP (ENGAGED) line
        //  - 0x05 / 0x01 : DOWN (DISENGAGED) line
        // ENGAGED when UP=pressed or DOWN=released.
        if ((off == 0x04 && mask == 0x80) || (off == 0x05 && mask == 0x01)) {
            if (off == 0x04 && mask == 0x80) { // UP line
                _hwApDiscEngaged = !pressed;   // pressed -> DISENGAGED, released -> ENGAGED
            } else {                           // DOWN line
                _hwApDiscEngaged = pressed;    // pressed -> ENGAGED, released -> DISENGAGED
                _haveHwSnapshot = true;
            }
            if (debounce(_lastApDiscToggleTime)) {
                maybeToggle("laminar/B738/autopilot/disconnect_pos", _hwApDiscEngaged, "laminar/B738/autopilot/disconnect_toggle");
            }
            return;
        }

        // All other momentary buttons use the existing mapping table
        for (const auto &b : _btns) {
            if (b.off == off && b.mask == mask) {
                Dataref::getInstance()->executeCommand(pressed ? b.press : b.release);
                return;
            }
        }
    }

    void ZiboPAP3Profile::onEncoderDelta(uint8_t posOff, int8_t delta) {
        for (const auto &e : _encs) {
            if (e.posOff == posOff) {
                repeatCmd(e.inc, e.dec, delta, e.stepPerTick);
                return;
            }
        }
    }

    uint8_t ZiboPAP3Profile::mcpPowerMask() const {
        uint8_t mask = 0;

        if (Dataref::getInstance()->get<bool>("sim/cockpit2/autopilot/autopilot_has_power")) {
            mask |= 0x01; // bit0 = autopilot has power
        }
        if (Dataref::getInstance()->get<bool>("laminar/B738/electric/dc_bus1_status")) {
            mask |= 0x02; // bit1 = DC bus 1 alive
        }
        if (Dataref::getInstance()->get<bool>("laminar/B738/electric/dc_bus2_status")) {
            mask |= 0x04; // bit2 = DC bus 2 alive
        }

        return mask;
    }

    bool ZiboPAP3Profile::mcpHasPower() const {
        return (mcpPowerMask() & 0x01) != 0;
    }

    void ZiboPAP3Profile::syncSimToHardware() {
        if (!_haveHwSnapshot) {
            return; // <--- garde-fou
        }

        maybeToggle("laminar/B738/autopilot/flight_director_pos", _hwFDCaptOn, "laminar/B738/autopilot/flight_director_toggle");
        maybeToggle("laminar/B738/autopilot/flight_director_fo_pos", _hwFDFoOn, "laminar/B738/autopilot/flight_director_fo_toggle");
        maybeToggle("laminar/B738/autopilot/disconnect_pos", _hwApDiscEngaged, "laminar/B738/autopilot/disconnect_toggle");

        const bool havePower = (Dataref::getInstance()->get<bool>("sim/cockpit2/autopilot/autopilot_has_power") && (Dataref::getInstance()->get<bool>("laminar/B738/electric/dc_bus1_status") || Dataref::getInstance()->get<bool>("laminar/B738/electric/dc_bus2_status")));
        if (havePower) {
            maybeToggle("laminar/B738/autopilot/autothrottle_arm_pos", _hwATOn, "laminar/B738/autopilot/autothrottle_arm_toggle");
        }
    }

    static inline bool bit(const uint8_t *r, int len, uint8_t off, uint8_t mask) {
        return (off < (uint8_t) len) && ((r[off] & mask) != 0);
    }

    void ZiboPAP3Profile::syncSimToHardwareFromRaw(const std::uint8_t *r, int len) {
        if (!r || len <= 0) {
            return;
        }

        // --- FD CAPT / FO ---
        // Lignes "OFF" (1 => OFF). ON <=> (OFF == 0)
        const bool hwFDCaptOn = !bit(r, len, 0x04, 0x08); // FD CAPT OFF
        const bool hwFDFoOn = !bit(r, len, 0x04, 0x20);   // FD FO   OFF
        maybeToggle("laminar/B738/autopilot/flight_director_pos", hwFDCaptOn, "laminar/B738/autopilot/flight_director_toggle");
        maybeToggle("laminar/B738/autopilot/flight_director_fo_pos", hwFDFoOn, "laminar/B738/autopilot/flight_director_fo_toggle");
        _hwFDCaptOn = hwFDCaptOn; // seed internes si tu les utilises
        _hwFDFoOn = hwFDFoOn;

        // --- A/T ARM ---
        // 0x06/0x01 = "ARMED"; 0x06/0x02 = "DISARMED"
        // ON <=> (ARMED == 1) || (DISARMED == 0)
        const bool atArmed = bit(r, len, 0x06, 0x01);
        const bool atDisarmed = bit(r, len, 0x06, 0x02);
        const bool hwATOn = atArmed || !atDisarmed;

        // ---  BANK ANGLE snapshot ---
        auto bit = [](const std::uint8_t *rr, int l, std::uint8_t off, std::uint8_t mask) {
            return (rr && l > off) && ((rr[off] & mask) != 0);
        };

        int targetBank = -1;
        if (bit(r, len, 0x05, 0x02)) {
            targetBank = 0; // 10°
        } else if (bit(r, len, 0x05, 0x04)) {
            targetBank = 1; // 15°
        } else if (bit(r, len, 0x05, 0x08)) {
            targetBank = 2; // 20°
        } else if (bit(r, len, 0x05, 0x10)) {
            targetBank = 3; // 25°
        } else if (bit(r, len, 0x05, 0x20)) {
            targetBank = 4; // 30°
        }

        if (targetBank >= 0) {
            nudgeBankAngleTo(targetBank);
        }

        const bool havePower = (Dataref::getInstance()->get<bool>("sim/cockpit2/autopilot/autopilot_has_power") &&
                                (Dataref::getInstance()->get<bool>("laminar/B738/electric/dc_bus1_status") || Dataref::getInstance()->get<bool>("laminar/B738/electric/dc_bus2_status")));
        if (havePower) {
            maybeToggle("laminar/B738/autopilot/autothrottle_arm_pos", hwATOn, "laminar/B738/autopilot/autothrottle_arm_toggle");
            if (Dataref::getInstance()->get<bool>("laminar/B738/autopilot/autothrottle_arm_pos") && hwATOn && _device) {
                _device->pulseATSolenoid(100);
            }
        }
        _hwATOn = hwATOn;

        // --- A/P DISENGAGE latch ---
        // 0x04/0x80 = UP(ENGAGED), 0x05/0x01 = DOWN(DISENGAGED)
        // ENGAGED <=> (UP==1) || (DOWN==0)
        const bool upEng = bit(r, len, 0x04, 0x80);
        const bool downDiseng = bit(r, len, 0x05, 0x01);
        const bool hwApEngaged = upEng || !downDiseng;
        maybeToggle("laminar/B738/autopilot/disconnect_pos", hwApEngaged, "laminar/B738/autopilot/disconnect_toggle");
        _hwApDiscEngaged = hwApEngaged;
    }

    int ZiboPAP3Profile::readBankIndex() const {
        if (Dataref::getInstance()->exists("laminar/B738/autopilot/bank_angle_pos")) {
            return -1;
        }

        return Dataref::getInstance()->get<int>("laminar/B738/autopilot/bank_angle_pos");
    }

    void ZiboPAP3Profile::nudgeBankAngleTo(int target) {
        if (target < 0 || target > 4) {
            return;
        }

        int cur = readBankIndex();
        if (cur < 0) {
            return;
        }

        // Chemin direct (sans wrap). Si un jour Zibo wrappe circulairement, on pourra optimiser.
        const int maxSteps = 10; // garde-fou anti-boucle
        int steps = 0;

        while (cur != target && steps++ < maxSteps) {
            const bool up = (target > cur);

            Dataref::getInstance()->executeCommand(up ? "laminar/B738/autopilot/bank_angle_up" : "laminar/B738/autopilot/bank_angle_dn");

            // Relis la position; Zibo met à jour très vite.
            int next = readBankIndex();
            if (next == cur) {
                Dataref::getInstance()->executeCommand(up ? "laminar/B738/autopilot/bank_angle_up" : "laminar/B738/autopilot/bank_angle_dn");
                next = readBankIndex();
            }
            cur = next;
        }
    }

}
