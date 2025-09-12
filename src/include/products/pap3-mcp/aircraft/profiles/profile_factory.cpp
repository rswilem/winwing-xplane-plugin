#include "profile_factory.h"

#include "default_profile.h"
#include "zibo_profile.h"

namespace pap3::aircraft {

    std::unique_ptr<PAP3AircraftProfile> ProfileFactory::detect() {
        // Essayer Zibo d’abord
        {
            auto zibo = std::make_unique<ZiboPAP3Profile>();
            if (zibo->isEligible()) {
                // conversion dérivée -> base (héritage visible ici)
                return std::unique_ptr<PAP3AircraftProfile>(std::move(zibo));
            }
        }
        // Sinon fallback
        return std::make_unique<DefaultPAP3Profile>();
    }

} // namespace pap3::aircraft