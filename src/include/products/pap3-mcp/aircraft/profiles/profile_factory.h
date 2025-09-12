#ifndef PAP3_PROFILE_FACTORY
#define PAP3_PROFILE_FACTORY

#include "pap3_aircraft.h"

#include <memory>

namespace pap3::aircraft {

    class ProfileFactory {
        public:
            static std::unique_ptr<PAP3AircraftProfile> detect();
    };

}

#endif