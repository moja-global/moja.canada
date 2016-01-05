#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace moja {
namespace modules {
namespace cbm {

    VolumeToBiomassCarbonGrowth::VolumeToBiomassCarbonGrowth(DynamicObject rootParameters) {
        _hardwoodRootParameterA = rootParameters["hw_a"];
        _softwoodRootParameterA = rootParameters["sw_a"];
        _hardwoodRootParameterB = rootParameters["hw_b"];
        _fineRootProportionParameterA = rootParameters["frp_a"];
        _fineRootProportionParameterB = rootParameters["frp_b"];
        _fineRootProportionParameterC = rootParameters["frp_c"];
    }

    bool VolumeToBiomassCarbonGrowth::isBiomassCarbonCurveAvailable(Int64 growthCurveID) {
        auto standBioCarbonCurve = getBiomassCarbonCurve(growthCurveID);
        return standBioCarbonCurve != nullptr;
    }

    void VolumeToBiomassCarbonGrowth::generateBiomassCarbonCurve(
        std::shared_ptr<StandGrowthCurve> standGrowthCurve) {

        // Converter to generate softwood component biomass carbon curve.
        std::shared_ptr<ComponentBiomassCarbonCurve> swCarbonCurve = nullptr;
        if (standGrowthCurve->hasYieldComponent(SpeciesType::Softwood)) {
            swCarbonCurve = _converter.generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Softwood);

            _converter.doSmoothing(*standGrowthCurve, swCarbonCurve.get(), SpeciesType::Softwood);
        }

        // Converter to generate hardwood component biomass carbon curve.
        std::shared_ptr<ComponentBiomassCarbonCurve> hwCarbonCurve = nullptr;
        if (standGrowthCurve->hasYieldComponent(SpeciesType::Hardwood)) {
            hwCarbonCurve = _converter.generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Hardwood);

            _converter.doSmoothing(*standGrowthCurve, hwCarbonCurve.get(), SpeciesType::Hardwood);
        }

        auto standCarbonCurve = std::make_shared<StandBiomassCarbonCurve>();
        standCarbonCurve->setSoftwoodComponent(swCarbonCurve);
        standCarbonCurve->setHardwoodComponent(hwCarbonCurve);

        _standBioCarbonGrowthCurves.insert(std::pair<Int64, std::shared_ptr<StandBiomassCarbonCurve>>(
            standGrowthCurve->standGrowthCurveID(), standCarbonCurve));
    }
    
    std::shared_ptr<AboveGroundBiomassCarbonIncrement> VolumeToBiomassCarbonGrowth::getAGBiomassCarbonIncrements(
        Int64 growthCurveID, int age) {

        std::shared_ptr<StandBiomassCarbonCurve> standBioCarbonCurve = nullptr;
        auto mapIt = _standBioCarbonGrowthCurves.find(growthCurveID);
        if (mapIt != _standBioCarbonGrowthCurves.end()) {
            standBioCarbonCurve = mapIt->second;
        }

        double softwoodMerch = 0;
        double softwoodFoliage = 0;
        double softwoodOther = 0;
        auto softwoodComponent = standBioCarbonCurve->softwoodCarbonCurve();
        if (softwoodComponent != nullptr) {
            softwoodMerch = softwoodComponent->getMerchCarbonIncrement(age);
            softwoodFoliage = softwoodComponent->getFoliageCarbonIncrement(age);
            softwoodOther = softwoodComponent->getOtherCarbonIncrement(age);
        }

        double hardwoodMerch = 0;
        double hardwoodFoliage = 0;
        double hardwoodOther = 0;
        auto hardwoodComponent = standBioCarbonCurve->hardwoodCarbonCurve();
        if (hardwoodComponent != nullptr) {
            hardwoodMerch = hardwoodComponent->getMerchCarbonIncrement(age);
            hardwoodFoliage = hardwoodComponent->getFoliageCarbonIncrement(age);
            hardwoodOther = hardwoodComponent->getOtherCarbonIncrement(age);
        }

        auto increment = std::make_shared<AboveGroundBiomassCarbonIncrement>(
            softwoodMerch, softwoodFoliage, softwoodOther,
            hardwoodMerch, hardwoodFoliage, hardwoodOther);
        
        return increment;
    }
    
    std::shared_ptr<RootBiomassCarbonIncrement> VolumeToBiomassCarbonGrowth::getBGBiomassCarbonIncrements(
        double totalSWAgCarbon, double standSWCoarseRootsCarbon, double standSWFineRootsCarbon,
        double totalHWAgCarbon, double standHWCoarseRootsCarbon, double standHWFineRootsCarbon) {
        
        // Get the root biomass.
        double softwoodRootBiomassTotal = _softwoodRootParameterA
            * (totalSWAgCarbon / _biomassToCarbonRatio);
        double hardwoodRootBiomassTotal = _hardwoodRootParameterA
            * std::pow(totalHWAgCarbon / _biomassToCarbonRatio, _hardwoodRootParameterB);

        // Get fine root proportion.
        double fineRootPortion = _fineRootProportionParameterA
            + _fineRootProportionParameterB * std::exp(
                _fineRootProportionParameterC
                * (softwoodRootBiomassTotal  + hardwoodRootBiomassTotal));

        // Get the root biomass carbon.
        double swcrCarbonChanges = softwoodRootBiomassTotal * (1 - fineRootPortion)
            * _biomassToCarbonRatio - standSWCoarseRootsCarbon;
        double swfrCarbonChanges = softwoodRootBiomassTotal * fineRootPortion
            * _biomassToCarbonRatio - standSWFineRootsCarbon;

        double hwcrCarbonChanges = hardwoodRootBiomassTotal * (1 - fineRootPortion)
            * _biomassToCarbonRatio - standHWCoarseRootsCarbon;
        double hwfrCarbonChanges = hardwoodRootBiomassTotal * fineRootPortion
            * _biomassToCarbonRatio - standHWFineRootsCarbon;
        
        // Set the root increment/changes in terms of carbon.
        auto increment = std::make_shared<RootBiomassCarbonIncrement>();
        increment->setSoftwoodFineRoots(swfrCarbonChanges);
        increment->setSoftwoodCoarseRoots(swcrCarbonChanges);
        increment->setHardwoodFineRoots(hwfrCarbonChanges);
        increment->setHardwoodCoarseRoots(hwcrCarbonChanges);

        return increment;
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
