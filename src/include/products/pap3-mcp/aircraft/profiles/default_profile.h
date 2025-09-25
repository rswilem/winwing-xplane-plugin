#ifndef PAP3_DEFAULT_PROFILE
#define PAP3_DEFAULT_PROFILE

#include "pap3_aircraft.h"

#include <XPLMDataAccess.h>

namespace pap3::aircraft {

    class DefaultPAP3Profile final : public PAP3AircraftProfile {
        public:
            DefaultPAP3Profile();
            ~DefaultPAP3Profile() override;

            bool isEligible() const override;
            void start(StateCallback onChanged) override;
            void stop() override;
            State current() const override;

        private:
            static float FlightLoopThunk(float elapsed, float, int, void *refcon);

            void poll();

            State _state{};
            StateCallback _cb;
            bool _running{false};

            // Datarefs Laminar/Zibo
            XPLMDataRef _drSpd{nullptr};
            XPLMDataRef _drHdg{nullptr};
            XPLMDataRef _drAlt{nullptr};
            XPLMDataRef _drVvi{nullptr};
            XPLMDataRef _drCrsCapt{nullptr};
            XPLMDataRef _drCrsFo{nullptr};

            XPLMDataRef _drLedN1{nullptr};
            XPLMDataRef _drLedSpd{nullptr};
            XPLMDataRef _drLedVnav{nullptr};
            XPLMDataRef _drLedLvlChg{nullptr};
            XPLMDataRef _drLedHdgSel{nullptr};
            XPLMDataRef _drLedLnav{nullptr};
            XPLMDataRef _drLedVorLoc{nullptr};
            XPLMDataRef _drLedApp{nullptr};
            XPLMDataRef _drLedAltHld{nullptr};
            XPLMDataRef _drLedVs{nullptr};
            XPLMDataRef _drLedCmdA{nullptr};
            XPLMDataRef _drLedCwsA{nullptr};
            XPLMDataRef _drLedCmdB{nullptr};
            XPLMDataRef _drLedCwsB{nullptr};
            XPLMDataRef _drLedAtArm{nullptr};
            XPLMDataRef _drLedMaCapt{nullptr};
            XPLMDataRef _drLedMaFo{nullptr};
    };

}

#endif