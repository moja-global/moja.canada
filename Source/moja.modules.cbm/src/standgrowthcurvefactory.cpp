#include "moja/flint/variable.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"

namespace moja {
namespace modules {
namespace cbm {
    
	StandGrowthCurveFactory::StandGrowthCurveFactory() {}  
		
	Poco::SharedPtr<StandGrowthCurve> StandGrowthCurveFactory::createStandGrowthCurve(
		Int64 standGrowthCurveID, Int64 spuID, flint::ILandUnitDataWrapper& landUnitData) {

        StandGrowthCurve standGrowthCurve(standGrowthCurveID, spuID);

		 // Get the table of softwood merchantable volumes associated to the stand growth curve.
        std::vector<DynamicObject> softwoodYieldTable;
        const auto& swTable = landUnitData.getVariable("softwood_yield_table")->value();
        if (!swTable.isEmpty()) {
            softwoodYieldTable = swTable.extract<const std::vector<DynamicObject>>();
        }

        TreeYieldTable swTreeYieldTable(softwoodYieldTable, SpeciesType::Softwood);
        standGrowthCurve.addYieldTable(swTreeYieldTable);

        // Get the table of hardwood merchantable volumes associated to the stand growth curve.
        std::vector<DynamicObject> hardwoodYieldTable;
		const auto& hwTable = landUnitData.getVariable("hardwood_yield_table")->value();
        if (!hwTable.isEmpty()) {
            hardwoodYieldTable = hwTable.extract<const std::vector<DynamicObject>>();
        }

        TreeYieldTable hwTreeYieldTable(hardwoodYieldTable, SpeciesType::Hardwood);
        standGrowthCurve.addYieldTable(hwTreeYieldTable);
        
        // Query for the appropriate PERD factor data.
        std::vector<DynamicObject> vol2bioParams;
		const auto& vol2bio = landUnitData.getVariable("volume_to_biomass_parameters")->value();
        if (!vol2bio.isEmpty()) {
            if (vol2bio.isVector()) {
                vol2bioParams = vol2bio.extract<std::vector<DynamicObject>>();
            } else {
                vol2bioParams.push_back(vol2bio.extract<DynamicObject>());
            }
        }

        for (const auto& row : vol2bioParams) {
            auto perdFactor = std::make_unique<PERDFactor>();
            perdFactor->setValue(row);

            std::string forestType = row["forest_type"].convert<std::string>();
            if (forestType == "Softwood") {
                standGrowthCurve.setPERDFactor(std::move(perdFactor), SpeciesType::Softwood);
                standGrowthCurve.setForestTypeConfiguration(ForestTypeConfiguration{
                    "Softwood",
                    std::make_shared<SoftwoodRootBiomassEquation>(
                        row["sw_a"], row["frp_a"], row["frp_b"], row["frp_c"])
                }, SpeciesType::Softwood);
            } else if (forestType == "Hardwood") {
                standGrowthCurve.setPERDFactor(std::move(perdFactor), SpeciesType::Hardwood);
                standGrowthCurve.setForestTypeConfiguration(ForestTypeConfiguration{
                    "Hardwood",
                    std::make_shared<HardwoodRootBiomassEquation>(
                        row["hw_a"], row["hw_b"], row["frp_a"], row["frp_b"], row["frp_c"])
                }, SpeciesType::Hardwood);
            }
        }

		// Pre-process the stand growth curve here.
		standGrowthCurve.processStandYieldTables();

		// Time to store the stand growth curve lookup for moss related modules		
		addStandGrowthCurve(standGrowthCurveID, standGrowthCurve);

        return getCache().get(standGrowthCurveID);	
	}  	
	
	
	Poco::SharedPtr<StandGrowthCurve> StandGrowthCurveFactory::getStandGrowthCurve(Int64 growthCurveID) {
        Poco::SharedPtr<StandGrowthCurve> standGrowthCurve = nullptr;
        auto cached = getCache().get(growthCurveID);
        if (!cached.isNull()) {
            standGrowthCurve = cached;
        }

        return standGrowthCurve;
    }

	void StandGrowthCurveFactory::addStandGrowthCurve(Int64 standGrowthCurveID, StandGrowthCurve& standGrowthCurve) {
		getCache().add(standGrowthCurve.standGrowthCurveID(), standGrowthCurve);
	}
}}}