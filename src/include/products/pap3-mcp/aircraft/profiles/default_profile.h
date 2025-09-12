#pragma once
#include "../pap3_aircraft.h"

#include <XPLMDataAccess.h>

namespace pap3::aircraft {

    // Profil générique: lit les datarefs sim/* standard
    class DefaultPAP3Profile final : public PAP3AircraftProfile {
        public:
            DefaultPAP3Profile();
            ~DefaultPAP3Profile() override;

            bool isEligible() const override;             // fallback si rien d'autre ne matche
            void start(StateCallback onChanged) override; // démarre un FlightLoop à ~25 Hz
            void stop() override;
            State current() const override;

        private:
            // Callback FlightLoop -> redirige vers poll()
            static float FlightLoopThunk(float elapsed, float, int, void *refcon);

            // Worker réel
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