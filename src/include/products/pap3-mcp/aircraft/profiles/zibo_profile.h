#ifndef PAP3_ZIBO_PROFILE
#define PAP3_ZIBO_PROFILE

#include "pap3_aircraft.h"

#include <cstdint>
#include <string>
#include <vector>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

namespace pap3::aircraft {

    class ZiboPAP3Profile final : public PAP3AircraftProfile {
        public:
            using State = pap3::aircraft::State;

            ZiboPAP3Profile();
            ~ZiboPAP3Profile() override;

            bool isEligible() const override;
            void start(StateCallback onChanged) override;
            void stop() override;
            State current() const override;
            void tick() override;

            void syncSimToHardware() override;

            // Input hooks (must match PAP3AircraftProfile)
            void onButton(uint8_t off, uint8_t mask, bool pressed) override;
            void onEncoderDelta(uint8_t posOff, int8_t delta) override;

            void syncSimToHardwareFromRaw(const std::uint8_t *report, int len) override;

            uint8_t mcpPowerMask() const override;

            // Device power for illumination gating
            bool mcpHasPower() const override;

            // --- Bank angle (Zibo) ---
            int readBankIndex() const;         // 0..4
            void nudgeBankAngleTo(int target); // 0..4

        private:
            bool _haveHwSnapshot = false;
            void poll();

            void repeatCmd(const char *inc, const char *dec, int8_t delta, int stepPerTick = 1);
            void maybeToggle(const char *dataref, bool desiredOn, const char *toggleCmd);
            bool debounce(float &lastTs, float minDeltaSec = 0.05f) const;

            State _state{};
            StateCallback _cb;
            bool _running = false;

            // --- Last known hardware state inferred from PAP3 inputs ---
            bool _hwFDCaptOn = false;
            bool _hwFDFoOn = false;
            bool _hwATOn = false;
            bool _hwApDiscEngaged = false; // true = UP/ENGAGED, false = DOWN/DISENGAGED

            // --- Simple debounce for double-line switches (AT/AP DISC) ---
            float _lastAtToggleTime = 0.0f;
            float _lastApDiscToggleTime = 0.0f;

            // Button bindings (offset/mask -> press/release commands)
            struct BtnBinding {
                    uint8_t off;
                    uint8_t mask;
                    const char *press;
                    const char *release = nullptr; // optional
            };

            std::vector<BtnBinding> _btns;

            // Encoder bindings (posOff -> inc/dec commands)
            struct EncBinding {
                    uint8_t posOff;
                    const char *inc = nullptr;
                    const char *dec = nullptr;
                    int stepPerTick = 1;
            };

            std::vector<EncBinding> _encs;
    };

}

#endif
