#include "moja/modules/cbm/standbiomasscarboncurve.h"

#include <moja/logging.h>
#include <algorithm>

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

    std::vector<double> StandBiomassCarbonCurve::getAboveGroundCarbonCurve()
    {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getAboveGroundCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < componentCurve.size()) {
                curve.resize(componentCurve.size(), 0.0);
            }

            for (int i = 0; i < componentCurve.size(); i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    double StandBiomassCarbonCurve::getMaturityAtAge(int age) {
        if (_maturityCurve.empty()) {
            std::vector<double> totalFoliageCurve;
            for (const auto& component : _components) {
                const auto& componentCurve = component.getAboveGroundCarbonCurve();
                auto maxComponentAge = componentCurve.size();

                if (totalFoliageCurve.size() < componentCurve.size()) {
                    totalFoliageCurve.resize(componentCurve.size(), 0.0);
                }

                for (int i = 0; i < componentCurve.size(); i++) {
                    totalFoliageCurve[i] += componentCurve[i];
                }
            }

            double maxFoliage = *std::max_element(totalFoliageCurve.begin(), totalFoliageCurve.end());
            for (auto foliage : totalFoliageCurve) {
                _maturityCurve.push_back(foliage / maxFoliage);
            }
        }

        return age >= _maturityCurve.size() ? _maturityCurve.back() : _maturityCurve[age];
    }

}}}