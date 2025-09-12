#include "profile_factory.h"

#include "default_profile.h"
#include "zibo_profile.h"

namespace pap3::aircraft {

    std::unique_ptr<PAP3AircraftProfile> ProfileFactory::detect() {
        auto zibo = std::make_unique<ZiboPAP3Profile>();
        if (zibo->isEligible()) {
            return std::unique_ptr<PAP3AircraftProfile>(std::move(zibo));
        }

        return std::make_unique<DefaultPAP3Profile>();
    }

}