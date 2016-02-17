#include "moja/modules/cbm/standbiomasscarboncurve.h"

namespace moja {
namespace modules {
namespace cbm {
    
    std::unordered_map<std::string, double> StandBiomassCarbonCurve::getIncrements() {
        double standRootBiomass = 0.0;
        for (const auto& component : _components) {
            standRootBiomass += component.calculateRootBiomass();
        }

        std::unordered_map<std::string, double> increments;
        for (const auto& component : _components) {
            const auto componentIncrements = component.getIncrements(standRootBiomass);
            increments.insert(componentIncrements.begin(), componentIncrements.end());
        }

        return increments;
    }

}}}