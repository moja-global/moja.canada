#include "moja/flint/variable.h"

#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
namespace modules {
namespace cbm {
    
	StandGrowthCurveFactory::StandGrowthCurveFactory(){}  
		
	std::shared_ptr<StandGrowthCurve> StandGrowthCurveFactory::createStandGrowthCurve(Int64 standGrowthCurveID, int spuID, flint::ILandUnitDataWrapper& landUnitData){
		 auto standGrowthCurve = std::make_shared<StandGrowthCurve>(standGrowthCurveID, spuID);
		 
		 // Get the table of softwood merchantable volumes associated to the stand growth curve.
        std::vector<DynamicObject> softwoodYieldTable;
        const auto& swTable = landUnitData.getVariable("softwood_yield_table")->value();
        if (!swTable.isEmpty()) {
            softwoodYieldTable = swTable.extract<const std::vector<DynamicObject>>();
        }

        auto swTreeYieldTable = std::make_shared<TreeYieldTable>(softwoodYieldTable, SpeciesType::Softwood);
        standGrowthCurve->addYieldTable(swTreeYieldTable);

        // Get the table of hardwood merchantable volumes associated to the stand growth curve.
        std::vector<DynamicObject> hardwoodYieldTable;
		const auto& hwTable = landUnitData.getVariable("hardwood_yield_table")->value();
        if (!hwTable.isEmpty()) {
            hardwoodYieldTable = hwTable.extract<const std::vector<DynamicObject>>();
        }

        auto hwTreeYieldTable = std::make_shared<TreeYieldTable>(hardwoodYieldTable, SpeciesType::Hardwood);
        standGrowthCurve->addYieldTable(hwTreeYieldTable);
        
        // Query for the appropriate PERD factor data.
        std::vector<DynamicObject> vol2bioParams;
		const auto& vol2bio = landUnitData.getVariable("volume_to_biomass_parameters")->value();
        if (vol2bio.isVector()) {
            vol2bioParams = vol2bio.extract<std::vector<DynamicObject>>();
        } else if (!vol2bio.isEmpty()) {
            vol2bioParams.push_back(vol2bio.extract<DynamicObject>());
        }

        for (const auto& row : vol2bioParams) {
            auto perdFactor = std::make_unique<PERDFactor>();
            perdFactor->setValue(row);

            std::string forestType = row["forest_type"].convert<std::string>();
            if (forestType == "Softwood") {
                standGrowthCurve->setPERDFactor(std::move(perdFactor), SpeciesType::Softwood);
            } else if (forestType == "Hardwood") {
                standGrowthCurve->setPERDFactor(std::move(perdFactor), SpeciesType::Hardwood);
            }
        }

		// Pre-process the stand growth curve here.
		standGrowthCurve->processStandYieldTables();

		//time to store the stand growth curve lookup for moss related modules		
		addStandGrowthCurve(standGrowthCurveID, standGrowthCurve);

        return standGrowthCurve;	
	}  	
	
	
	std::shared_ptr<StandGrowthCurve> StandGrowthCurveFactory::getStandGrowthCurve(Int64 growthCurveID){
		std::shared_ptr<StandGrowthCurve> standGrowthCurve = nullptr;
			
        auto mapIt = _standGrowthCurves.find(growthCurveID);
        if (mapIt != _standGrowthCurves.end()) {
            standGrowthCurve = mapIt->second;
		}
		
        return standGrowthCurve;
	}

	void StandGrowthCurveFactory::addStandGrowthCurve(Int64 standGrowthCurveID, std::shared_ptr<StandGrowthCurve> standGrowthCurve){
		_standGrowthCurves.insert(std::pair<Int64, std::shared_ptr<StandGrowthCurve>>(
			standGrowthCurve->standGrowthCurveID(), standGrowthCurve));
	}
}}}