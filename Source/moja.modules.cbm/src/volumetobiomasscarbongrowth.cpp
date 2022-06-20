#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace moja {
namespace modules {
namespace cbm {

    /**
     *  Return if VolumeToBiomassCarbonGrowth.getBiomassCarbonCurve() with arguments growthCurveID, spuID is not null 
     * 
     * @param growthCurveID Int64
     * @param spuID Int64
     * @return bool
     * *************************/
    bool VolumeToBiomassCarbonGrowth::isBiomassCarbonCurveAvailable(Int64 growthCurveID, Int64 spuID) {
        auto standBioCarbonCurve = getBiomassCarbonCurve(growthCurveID, spuID);
        return !standBioCarbonCurve.isNull();
    }

    /**
     * Generate Biomass carbon curves
     * 
     * If parameter standGrowthCurve has the yield component SpeciesType::Softwood or/and SpeciesType::Hardwood, 
     * generate the component biomass carbon curve and the root biomass equation corresponding to the forest configuration using moja::modules::CBM::StandGrowthCurve.getForestTypeConfiguration() \n
     * Add the components species type, root biomass equation and carbon curve to the parameter standCarbonCurve \n
     * Add the standCarbonCurve to the cache using VolumeToBiomassCarbonGrowth.getCache() for the tuple key (StandGrowthCurve.standGrowthCurveID(), StandGrowthCurve.spuID())
     * 
     * @param standGrowthCurve StandGrowthCurve& 
     * @return void
     * ********************/
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
    
    /**
     * If the tuple parameter growthCurveId, spuId exists in VolumeToBiomassCarbonGrowth.getCache(), get the StandBiomassCarbonCurve object from cache, 
     * and return the result of StandBiomassCarbonCurve.getIncrements() with argument as parameter landUnitData
     * 
     * @param landUnitData flint::ILandUnitDataWrapper*
     * @param growthCurveID Int64
     * @param spuID Int64
     * @return unordered_map<std::string, double>
     * *****************************/
    std::unordered_map<std::string, double> VolumeToBiomassCarbonGrowth::getBiomassCarbonIncrements(
        flint::ILandUnitDataWrapper* landUnitData, Int64 growthCurveID, Int64 spuID) {

        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = getCache().get(key);
        return standBioCarbonCurve->getIncrements(landUnitData);
    }
    
    /**
     * If the tuple parameter growthCurveId, spuId exists in VolumeToBiomassCarbonGrowth.getCache(), get the StandBiomassCarbonCurve object from cache, 
     * and return the result of VolumeToBiomassCarbonGrowth.getAboveGroundCarbonCurve()
     * 
     * @param growthCurveId Int64
     * @param spuId Int64
     * @return vector<double>
     *************************/
    std::vector<double> VolumeToBiomassCarbonGrowth::getAboveGroundCarbonCurve(Int64 growthCurveID, Int64 spuID) {
        auto key = std::make_tuple(growthCurveID, spuID);
        auto standBioCarbonCurve = getCache().get(key);
        return standBioCarbonCurve->getAboveGroundCarbonCurve();
    }

    /**
     * If the tuple parameter growthCurveId, spuId exists in VolumeToBiomassCarbonGrowth.getCache(), if the StandBiomassCarbonCurve object returned from cache is not null, 
     * return it, else return nullptr
     * 
     * @param growthCurveId Int64
     * @param spuId Int64
     * @return vector<double>
     *************************/
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
