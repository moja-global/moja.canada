#include "moja/modules/cbm/standbiomasscarboncurve.h"

#include <moja/logging.h>
#include <algorithm>
#include <fstream>

namespace moja {
namespace modules {
namespace cbm {
    
    std::unordered_map<std::string, double> StandBiomassCarbonCurve::getIncrements(flint::ILandUnitDataWrapper* landUnitData) {
        double standRootBiomass = 0.0;
        for (const auto& component : _components) {
            standRootBiomass += component.calculateRootBiomass(landUnitData);
        }

        std::unordered_map<std::string, double> increments;
        for (const auto& component : _components) {
            const auto componentIncrements = component.getIncrements(landUnitData, standRootBiomass);
            increments.insert(componentIncrements.begin(), componentIncrements.end());
        }

        return increments;
    }

    std::vector<double> StandBiomassCarbonCurve::getAboveGroundCarbonCurve() {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getAboveGroundCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    std::vector<double> StandBiomassCarbonCurve::getMerchCarbonCurve() {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getMerchCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    std::vector<double> StandBiomassCarbonCurve::getFoliageCarbonCurve()
    {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getFoliageCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    std::vector<double> StandBiomassCarbonCurve::getOtherCarbonCurve() {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getOtherCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    void StandBiomassCarbonCurve::writeDebuggingInfo(const std::string& path) {
        std::ofstream file(path);

        file << "Merch,";
        for (auto& value : getMerchCarbonCurve()) {
            file << value << ",";
        }

        file << "\nFoliage,";
        for (auto& value : getFoliageCarbonCurve()) {
            file << value << ",";
        }

        file << "\nOther,";
        for (auto& value : getOtherCarbonCurve()) {
            file << value << ",";
        }

        file.close();
    }

}}}