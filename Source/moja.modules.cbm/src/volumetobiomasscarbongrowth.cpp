#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace moja {
namespace modules {
namespace cbm {

    bool VolumeToBiomassCarbonGrowth::isBiomassCarbonCurveAvailable(Int64 growthCurveID, Int64 spuID) {
        auto standBioCarbonCurve = getBiomassCarbonCurve(growthCurveID, spuID);
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
            const auto forestTypeConfig = standGrowthCurve->getForestTypeConfiguration(SpeciesType::Softwood);
            standCarbonCurve->addComponent(StandComponent(
                "Softwood", forestTypeConfig.rootBiomassEquation, carbonCurve
            ));
        }

        // Converter to generate hardwood component biomass carbon curve.
        if (standGrowthCurve->hasYieldComponent(SpeciesType::Hardwood)) {
            auto carbonCurve = _converter.generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Hardwood);

            _converter.doSmoothing(*standGrowthCurve, carbonCurve.get(), SpeciesType::Hardwood);
            const auto forestTypeConfig = standGrowthCurve->getForestTypeConfiguration(SpeciesType::Hardwood);
            standCarbonCurve->addComponent(StandComponent(
                "Hardwood", forestTypeConfig.rootBiomassEquation, carbonCurve
            ));
        }

        // Lock for updating in case this is a multithreaded run.
        auto key = std::make_tuple(standGrowthCurve->standGrowthCurveID(), standGrowthCurve->spuID());
        if (!_standBioCarbonGrowthCurves->has(key)) {
            _standBioCarbonGrowthCurves->add(key, standCarbonCurve);
        }
    }
    
    std::unordered_map<std::string, double> VolumeToBiomassCarbonGrowth::getBiomassCarbonIncrements(
        flint::ILandUnitDataWrapper* landUnitData, Int64 growthCurveID, Int64 spuID) {

        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = *_standBioCarbonGrowthCurves->get(key);
        return standBioCarbonCurve->getIncrements(landUnitData);
    }
    
    std::vector<double> VolumeToBiomassCarbonGrowth::getAboveGroundCarbonCurve(Int64 growthCurveID, Int64 spuID) {
        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = *_standBioCarbonGrowthCurves->get(key);
        return standBioCarbonCurve->getAboveGroundCarbonCurve();
    }

    std::vector<double> VolumeToBiomassCarbonGrowth::getFoliageCarbonCurve(Int64 growthCurveID, Int64 spuID) {
        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = *_standBioCarbonGrowthCurves->get(key);
        return standBioCarbonCurve->getFoliageCarbonCurve();
    }

    std::shared_ptr<StandBiomassCarbonCurve> VolumeToBiomassCarbonGrowth::getBiomassCarbonCurve(
        Int64 growthCurveID, Int64 spuID) {

        auto key = std::make_tuple(growthCurveID, spuID);
        std::shared_ptr<StandBiomassCarbonCurve> standBioCarbonCurve = nullptr;
        auto cached = _standBioCarbonGrowthCurves->get(key);
        if (!cached.isNull()) {
            standBioCarbonCurve = *cached;
        }

        return standBioCarbonCurve;
    }

}}}
