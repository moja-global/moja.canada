#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ipool.h>
#include <moja/flint/variable.h>

#include <moja/logging.h>

namespace moja {
	namespace modules {
		namespace cbm {

			void PrintPools::printMossPools(std::string message, flint::ILandUnitDataWrapper& landUnitData) {
				auto pools = landUnitData.poolCollection();
				int ageValue = landUnitData.getVariable("age")->value();
				MOJA_LOG_INFO << message << ageValue - 1 << ", " <<
					landUnitData.getPool("FeatherMossLive")->value() << ", " <<
					landUnitData.getPool("SphagnumMossLive")->value() << ", " <<
					landUnitData.getPool("FeatherMossFast")->value() << ", " <<
					landUnitData.getPool("FeatherMossSlow")->value() << ", " <<
					landUnitData.getPool("SphagnumMossFast")->value() << ", " <<
					landUnitData.getPool("SphagnumMossSlow")->value() << ", " <<
					landUnitData.getPool("CO2")->value() << ", " <<
					landUnitData.getPool("CH4")->value() << ", " <<
					landUnitData.getPool("CO")->value();
			}

			void PrintPools::printPeatlandPools(std::string message, flint::ILandUnitDataWrapper& landUnitData) {
				auto pools = landUnitData.poolCollection();
				int ageValue = landUnitData.getVariable("peatland_shrub_age")->value();
				auto& peatland_class = landUnitData.getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (ageValue >= 0) {
					MOJA_LOG_INFO << "peatlandId: " << peatlandId << ", " << message << ageValue << ", " <<
						landUnitData.getPool("WoodyFoliageLive")->value() << ", " <<
						landUnitData.getPool("WoodyStemsBranchesLive")->value() << ", " <<
						landUnitData.getPool("WoodyRootsLive")->value() << ", " <<
						landUnitData.getPool("SedgeFoliageLive")->value() << ", " <<
						landUnitData.getPool("SedgeRootsLive")->value() << ", " <<
						landUnitData.getPool("SphagnumMossLive")->value() << ", " <<
						landUnitData.getPool("FeatherMossLive")->value() << ", " <<
						landUnitData.getPool("WoodyFoliageDead")->value() << ", " <<
						landUnitData.getPool("WoodyFineDead")->value() << ", " <<
						landUnitData.getPool("WoodyRootsDead")->value() << ", " <<
						landUnitData.getPool("SedgeFoliageDead")->value() << ", " <<
						landUnitData.getPool("SedgeRootsDead")->value() << ", " <<
						landUnitData.getPool("FeathermossDead")->value() << ", " <<
						landUnitData.getPool("Acrotelm_O")->value() << ", " <<
						landUnitData.getPool("Acrotelm_A")->value() << ", " <<
						landUnitData.getPool("Catotelm_A")->value() << ", " <<
						landUnitData.getPool("Catotelm_o")->value();
					/*
					landUnitData.getPool("CO2")->value() << ", " <<
					landUnitData.getPool("CH4")->value() << ", " <<
					landUnitData.getPool("CO")->value();


					MOJA_LOG_INFO << "Stand Age: " << ageValue << ", " <<
					pools.findPool("WoodyFoliageLive")->value() << ", " <<
					pools.findPool("WoodyStemsBranchesLive")->value() << ", " <<
					pools.findPool("WoodyRootsLive")->value() << ", " <<
					pools.findPool("SedgeFoliageLive")->value() << ", " <<
					pools.findPool("SedgeRootsLive")->value() << ", " <<
					pools.findPool("WoodyStemsBranchesDead")->value() << ", " <<
					pools.findPool("WoodyFoliageDead")->value() << ", " <<
					pools.findPool("WoodyRootsDead")->value() << ", " <<
					pools.findPool("SedgeFoliageDead")->value() << ", " <<
					pools.findPool("SedgeRootsDead")->value() << ", " <<
					pools.findPool("FeathermossDead")->value() << ", " <<
					pools.findPool("Acrotelm")->value() << ", " <<
					pools.findPool("Catotelm")->value() << ", " <<
					pools.findPool("CO2")->value() << ", " <<
					pools.findPool("CH4")->value() << ", " <<
					pools.findPool("CO")->value();
					*/
				}
			}


			void PrintPools::printForestPools(std::string message, double mat, flint::ILandUnitDataWrapper& landUnitData) {
				auto pools = landUnitData.poolCollection();
				int ageValue = landUnitData.getVariable("age")->value();
				MOJA_LOG_INFO << message << ageValue - 1 << ", " <<
					mat << ", " <<
					landUnitData.getPool("SoftwoodMerch")->value() << ", " <<
					landUnitData.getPool("SoftwoodFoliage")->value() << ", " <<
					landUnitData.getPool("SoftwoodOther")->value() << ", " <<
					landUnitData.getPool("SoftwoodCoarseRoots")->value() << ", " <<
					landUnitData.getPool("SoftwoodFineRoots")->value() << ", " <<
					landUnitData.getPool("HardwoodMerch")->value() << ", " <<
					landUnitData.getPool("HardwoodFoliage")->value() << ", " <<
					landUnitData.getPool("HardwoodOther")->value() << ", " <<
					landUnitData.getPool("HardwoodCoarseRoots")->value() << ", " <<
					landUnitData.getPool("HardwoodFineRoots")->value() << ", " <<
					landUnitData.getPool("AboveGroundVeryFastSoil")->value() << ", " <<
					landUnitData.getPool("BelowGroundVeryFastSoil")->value() << ", " <<
					landUnitData.getPool("AboveGroundFastSoil")->value() << ", " <<
					landUnitData.getPool("BelowGroundFastSoil")->value() << ", " <<
					landUnitData.getPool("MediumSoil")->value() << ", " <<
					landUnitData.getPool("AboveGroundSlowSoil")->value() << ", " <<
					landUnitData.getPool("BelowGroundSlowSoil")->value() << ", " <<
					landUnitData.getPool("SoftwoodStemSnag")->value() << ", " <<
					landUnitData.getPool("SoftwoodBranchSnag")->value() << ", " <<
					landUnitData.getPool("HardwoodStemSnag")->value() << ", " <<
					landUnitData.getPool("HardwoodBranchSnag")->value() << ", " <<
					landUnitData.getPool("CO2")->value() << ", " <<
					landUnitData.getPool("CH4")->value() << ", " <<
					landUnitData.getPool("CO")->value();
	}
}}}
