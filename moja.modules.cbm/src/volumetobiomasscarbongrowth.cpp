#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace moja {
namespace modules {
namespace cbm {

    VolumeToBiomassCarbonGrowth::VolumeToBiomassCarbonGrowth(
        std::vector<ForestTypeConfiguration>& forestTypeConfigurations) {

        for (auto& forestType : forestTypeConfigurations) {
            _forestTypeConfigurations.insert(std::make_pair(
                forestType.forestType, forestType));
        }
    }

    bool VolumeToBiomassCarbonGrowth::isBiomassCarbonCurveAvailable(Int64 growthCurveID) {
        auto standBioCarbonCurve = getBiomassCarbonCurve(growthCurveID);
        return standBioCarbonCurve != nullptr;
    }

    void VolumeToBiomassCarbonGrowth::generateBiomassCarbonCurve(
        std::shared_ptr<StandGrowthCurve> standGrowthCurve) {

        auto standCarbonCurve = std::make_shared<StandBiomassCarbonCurve>();

        // Converter to generate softwood component biomass carbon curve.
        if (standGrowthCurve->hasYieldComponent(SpeciesType::Softwood)) {
            auto carbonCurve = _converter.generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Softwood);

            _converter.doSmoothing(*standGrowthCurve, carbonCurve.get(), SpeciesType::Softwood);
            const auto forestTypeConfig = _forestTypeConfigurations.find("Softwood")->second;
            standCarbonCurve->addComponent(StandComponent(
                "Softwood", forestTypeConfig.rootBiomassEquation, carbonCurve, forestTypeConfig.age,
                forestTypeConfig.merch, forestTypeConfig.foliage, forestTypeConfig.other,
                forestTypeConfig.fineRoots, forestTypeConfig.coarseRoots
            ));
        }

        // Converter to generate hardwood component biomass carbon curve.
        std::shared_ptr<ComponentBiomassCarbonCurve> hwCarbonCurve = nullptr;
        if (standGrowthCurve->hasYieldComponent(SpeciesType::Hardwood)) {
            auto carbonCurve = _converter.generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Hardwood);

            _converter.doSmoothing(*standGrowthCurve, hwCarbonCurve.get(), SpeciesType::Hardwood);
            const auto forestTypeConfig = _forestTypeConfigurations.find("Hardwood")->second;
            standCarbonCurve->addComponent(StandComponent(
                "Hardwood", forestTypeConfig.rootBiomassEquation, carbonCurve, forestTypeConfig.age,
                forestTypeConfig.merch, forestTypeConfig.foliage, forestTypeConfig.other,
                forestTypeConfig.fineRoots, forestTypeConfig.coarseRoots
            ));
        }

        _standBioCarbonGrowthCurves.insert(std::pair<Int64, std::shared_ptr<StandBiomassCarbonCurve>>(
            standGrowthCurve->standGrowthCurveID(), standCarbonCurve));
    }
    
    std::unordered_map<std::string, double> VolumeToBiomassCarbonGrowth::getBiomassCarbonIncrements(
        Int64 growthCurveID) {

        auto standBioCarbonCurve = _standBioCarbonGrowthCurves.find(growthCurveID)->second;
        return standBioCarbonCurve->getIncrements();
    }
    
    std::shared_ptr<StandBiomassCarbonCurve> VolumeToBiomassCarbonGrowth::getBiomassCarbonCurve(
        Int64 growthCurveID) {

        std::shared_ptr<StandBiomassCarbonCurve> standBioCarbonCurve = nullptr;
        auto mapIt = _standBioCarbonGrowthCurves.find(growthCurveID);
        if (mapIt != _standBioCarbonGrowthCurves.end()) {
            standBioCarbonCurve = mapIt->second;
        }

        return standBioCarbonCurve;
    }

}}}
