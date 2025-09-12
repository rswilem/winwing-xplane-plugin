#pragma once
#include "../pap3_aircraft.h"

#include <memory>

namespace pap3::aircraft {

    class ProfileFactory {
        public:
            static std::unique_ptr<PAP3AircraftProfile> detect();
    };

}
