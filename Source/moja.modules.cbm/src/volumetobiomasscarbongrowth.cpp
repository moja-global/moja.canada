#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace moja {
namespace modules {
namespace cbm {

    bool VolumeToBiomassCarbonGrowth::isBiomassCarbonCurveAvailable(Int64 growthCurveID, Int64 spuID) {
        auto standBioCarbonCurve = getBiomassCarbonCurve(growthCurveID, spuID);
        return !standBioCarbonCurve.isNull();
    }

    void VolumeToBiomassCarbonGrowth::generateBiomassCarbonCurve(StandGrowthCurve& standGrowthCurve) {
        StandBiomassCarbonCurve standCarbonCurve;

        // Converter to generate softwood component biomass carbon curve.
        if (standGrowthCurve.hasYieldComponent(SpeciesType::Softwood)) {
            auto carbonCurve = getConverter().generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Softwood);

            getConverter().doSmoothing(standGrowthCurve, carbonCurve.get(), SpeciesType::Softwood);
            const auto forestTypeConfig = standGrowthCurve.getForestTypeConfiguration(SpeciesType::Softwood);
            standCarbonCurve.addComponent(StandComponent(
                "Softwood", forestTypeConfig.rootBiomassEquation, carbonCurve
            ));
        }

        // Converter to generate hardwood component biomass carbon curve.
        if (standGrowthCurve.hasYieldComponent(SpeciesType::Hardwood)) {
            auto carbonCurve = getConverter().generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Hardwood);

            getConverter().doSmoothing(standGrowthCurve, carbonCurve.get(), SpeciesType::Hardwood);
            const auto forestTypeConfig = standGrowthCurve.getForestTypeConfiguration(SpeciesType::Hardwood);
            standCarbonCurve.addComponent(StandComponent(
                "Hardwood", forestTypeConfig.rootBiomassEquation, carbonCurve
            ));
        }

        auto key = std::make_tuple(standGrowthCurve.standGrowthCurveID(), standGrowthCurve.spuID());
        getCache().add(key, standCarbonCurve);
    }
    
    std::unordered_map<std::string, double> VolumeToBiomassCarbonGrowth::getBiomassCarbonIncrements(
        flint::ILandUnitDataWrapper* landUnitData, Int64 growthCurveID, Int64 spuID) {

        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = getCache().get(key);
        return standBioCarbonCurve->getIncrements(landUnitData);
    }
    
    std::vector<double> VolumeToBiomassCarbonGrowth::getAboveGroundCarbonCurve(Int64 growthCurveID, Int64 spuID) {
        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = getCache().get(key);
        return standBioCarbonCurve->getAboveGroundCarbonCurve();
    }

    std::vector<double> VolumeToBiomassCarbonGrowth::getFoliageCarbonCurve(Int64 growthCurveID, Int64 spuID) {
        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = getCache().get(key);
        return standBioCarbonCurve->getFoliageCarbonCurve();
    }

    Poco::SharedPtr<StandBiomassCarbonCurve> VolumeToBiomassCarbonGrowth::getBiomassCarbonCurve(
        Int64 growthCurveID, Int64 spuID) {

        auto key = std::make_tuple(growthCurveID, spuID);
        Poco::SharedPtr<StandBiomassCarbonCurve> standBioCarbonCurve = nullptr;
        auto cached = getCache().get(key);
        if (!cached.isNull()) {
            standBioCarbonCurve = cached;
        }

        return standBioCarbonCurve;
    }

}}}
