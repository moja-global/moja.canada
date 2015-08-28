#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace moja {
namespace modules {
namespace CBM {

    VolumeToBiomassCarbonGrowth::VolumeToBiomassCarbonGrowth() {
        _converter = std::make_unique<VolumeToBiomassConverter>();
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
            swCarbonCurve = _converter->generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Softwood);

            _converter->DoSmoothing(*standGrowthCurve, swCarbonCurve.get(), SpeciesType::Softwood);
        }

        // Converter to generate hardwood component biomass carbon curve.
        std::shared_ptr<ComponentBiomassCarbonCurve> hwCarbonCurve = nullptr;
        if (standGrowthCurve->hasYieldComponent(SpeciesType::Hardwood)) {
            hwCarbonCurve = _converter->generateComponentBiomassCarbonCurve(
                standGrowthCurve, SpeciesType::Hardwood);

            _converter->DoSmoothing(*standGrowthCurve, hwCarbonCurve.get(), SpeciesType::Hardwood);
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

        auto softwoodComponent = standBioCarbonCurve->softwoodCarbonCurve();
        auto hardwoodComponent = standBioCarbonCurve->hardwoodCarbonCurve();

        double softwoodMerch = 0;
        double softwoodFoliage = 0;
        double softwoodOther = 0;
        double hardwoodMerch = 0;
        double hardwoodFoliage = 0;
        double hardwoodOther = 0;

        if (softwoodComponent != nullptr) {
            softwoodMerch = softwoodComponent->getMerchCarbonIncrement(age);
            softwoodFoliage = softwoodComponent->getFoliageCarbonIncrement(age);
            softwoodOther = softwoodComponent->getOtherCarbonIncrement(age);
        }

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
    
    // TODO: to fully implement later with current stand above and below ground biomass pool information.
    std::shared_ptr<RootBiomassCarbonIncrement> VolumeToBiomassCarbonGrowth::getBGBiomassCarbonIncrements(
        double totalSWAgCarbon, double standSWCoarseRootsCarbon, double standSWFineRootsCarbon,
        double totalHWAgCarbon, double standHWCoarseRootsCarbon, double standHWFineRootsCarbon) {
        
        // Get the root biomass.
        double softwoodRootBiomassTotal = softwoodRootParameterA
            * (totalSWAgCarbon / biomassToCarbonRation);
        double hardwoodRootBiomassTotal = hardwoodRootParameterA
            * std::pow(totalHWAgCarbon / biomassToCarbonRation, hardwoodRootParameterB);

        // Get fine root proportion.
        double fineRootPortion = fineRootProportionParameterA
            + fineRootProportionParameterB * std::exp(
                fineRootProportionParameterC
                * (softwoodRootBiomassTotal  + hardwoodRootBiomassTotal));

        // Get the root biomass carbon.
        double swcrCarbonChanges = softwoodRootBiomassTotal * (1 - fineRootPortion)
            * biomassToCarbonRation - standSWCoarseRootsCarbon;
        double swfrCarbonChanges = softwoodRootBiomassTotal * fineRootPortion
            * biomassToCarbonRation - standSWFineRootsCarbon;

        double hwcrCarbonChanges = hardwoodRootBiomassTotal * (1 - fineRootPortion)
            * biomassToCarbonRation - standHWCoarseRootsCarbon;
        double hwfrCarbonChanges = hardwoodRootBiomassTotal * fineRootPortion
            * biomassToCarbonRation - standHWFineRootsCarbon;
        
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
