#include "moja/modules/cbm/peatlandspinupturnovermodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include "moja/modules/cbm/timeseries.h"
#include <moja/logging.h>

namespace moja {
	namespace modules {
		namespace cbm {
			void PeatlandSpinupTurnOverModule::doLocalDomainInit() {
				_atmosphere = _landUnitData->getPool("Atmosphere");

				_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
				_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
				_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");

				_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
				_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");

				_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
				_featherMossLive = _landUnitData->getPool("FeatherMossLive");

				_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
				_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
				_woodyCoarseDead = _landUnitData->getPool("WoodyCoarseDead");
				_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");

				_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
				_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");

				_feathermossDead = _landUnitData->getPool("FeathermossDead");

				_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
				_catotelm_a = _landUnitData->getPool("Catotelm_A");
				_acrotelm_a = _landUnitData->getPool("Acrotelm_A");
				_catotelm_o = _landUnitData->getPool("Catotelm_O");

				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
				_regenDelay = _landUnitData->getVariable("regen_delay");

				baseWTDParameters = _landUnitData->getVariable("base_wtd_parameters")->value().extract<DynamicObject>();
			}

			void PeatlandSpinupTurnOverModule::doTimingInit() {
				_runPeatland = false;

				//load initial peat pool values if it is enabled
				auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();
				if (loadInitialFlag) {
					const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_stocks")->value();
					loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());
				}

				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (_peatlandId > 0) {
					_runPeatland = true;

					// get the data by variable "peatland_turnover_parameters"
					const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

					//create the PeaglandGrowthParameters, set the value from the variable
					turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
					turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());

					// get the data by variable "peatland_growth_parameters"
					const auto& peatlandGrowthParams = _landUnitData->getVariable("peatland_growth_parameters")->value();

					//create the PeatlandGrowthParameters, set the value from the variable
					growthParas = std::make_shared<PeatlandGrowthParameters>();
					if (!peatlandGrowthParams.isEmpty()) {
						growthParas->setValue(peatlandGrowthParams.extract<DynamicObject>());
					}

					auto& lnMDroughtCode = _landUnitData->getVariable("spinup_drought_class")->value();
					auto& defaultLMDC = _landUnitData->getVariable("default_spinup_drought_class")->value();
					auto lnMeanDroughtCode = lnMDroughtCode.isEmpty() ? defaultLMDC : lnMDroughtCode;
					auto lwtd = computeWaterTableDepth(lnMeanDroughtCode, _peatlandId);

					//set identical water table depth values for three water table variables in spinup phase			
					_spinup_longterm_wtd = lwtd;
					_spinup_previous_annual_wtd = lwtd;
					_spinup_current_annual_wtd = lwtd;
				}
			}
			void PeatlandSpinupTurnOverModule::doTimingStep() {
				//no need to update water table in spinup 
				//updateWaterTable();			

				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) { return; }

				if (_runPeatland) {
					int regenDelay = _regenDelay->value();
					if (regenDelay > 0) {
						//in delay period, no any growth
						//do flux between catotelm and acrotelm due to water table changes
						doWaterTableFlux();
					}
					else {
						//update the current pool value
						updatePeatlandLivePoolValue();

						//turnover on live pools
						doLivePoolTurnover();

						//flux between catotelm and acrotelm due to water table changes
						doWaterTableFlux();
					}
				}
			}

			void PeatlandSpinupTurnOverModule::doWaterTableFlux() {
				//get current annual water table depth
				double currentAwtd = _spinup_longterm_wtd;

				//get previous annual water table depth
				double previousAwtd = _spinup_previous_annual_wtd;

				//get long term annual water table depth
				double longtermWtd = _spinup_current_annual_wtd;

				//MOJA_LOG_INFO << "IN Flux- peatlandID: " << peatlandID << " currentAwtd: " << currentAwtd << " previousAwtd: " << previousAwtd << " longtermWtd: " << longtermWtd;

				currentAwtd = currentAwtd > 0.0 ? 0.0 : currentAwtd;
				previousAwtd = previousAwtd > 0.0 ? 0.0 : previousAwtd;
				longtermWtd = longtermWtd > 0.0 ? 0.0 : longtermWtd;

				double a = turnoverParas->a();
				double b = turnoverParas->b();

				auto peatlandWaterTableFlux = _landUnitData->createStockOperation();

				double coPoolValue = _catotelm_o->value();
				double caPoolValue = _catotelm_a->value();
				double aoPoolValue = _acrotelm_o->value();
				double aaPoolValue = _acrotelm_a->value();

				double fluxAmount = computeCarbonTransfers(previousAwtd, currentAwtd, a, b);

				if (currentAwtd < longtermWtd && previousAwtd < longtermWtd) {
					if (currentAwtd >= previousAwtd) {
						//Catotelm_O -> Catotelm_A 		
						if (fluxAmount > coPoolValue) fluxAmount = coPoolValue;
						peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, fluxAmount);
					}
					else if (currentAwtd <= previousAwtd) {
						//Catotelm_A -> Catotelm_O
						if (fluxAmount > caPoolValue) fluxAmount = caPoolValue;
						peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, fluxAmount);
					}
				}
				else if (currentAwtd > longtermWtd&& previousAwtd > longtermWtd) {
					if (currentAwtd >= previousAwtd) {
						//Acrotelm_O -> Acrotelm_A 				
						if (fluxAmount > aoPoolValue) fluxAmount = aoPoolValue;
						peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, fluxAmount);
					}
					else if (currentAwtd <= previousAwtd) {
						//Acrotelm_A -> Acrotelm_O 
						if (fluxAmount > aaPoolValue) fluxAmount = aaPoolValue;
						peatlandWaterTableFlux->addTransfer(_acrotelm_a, _acrotelm_o, fluxAmount);
					}
				}
				else if (currentAwtd >= longtermWtd && previousAwtd <= longtermWtd) {
					if (currentAwtd >= previousAwtd) {
						double ao2aa = computeCarbonTransfers(longtermWtd, currentAwtd, a, b);
						if (ao2aa > aoPoolValue) ao2aa = aoPoolValue;
						peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, ao2aa);

						double co2ca = computeCarbonTransfers(longtermWtd, previousAwtd, a, b);
						if (co2ca > coPoolValue) co2ca = coPoolValue;
						peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, co2ca);
					}
				}
				else if (currentAwtd <= longtermWtd && previousAwtd >= longtermWtd) {
					if (currentAwtd <= previousAwtd) {
						double aa2ao = computeCarbonTransfers(longtermWtd, previousAwtd, a, b);
						if (aa2ao > aaPoolValue) aa2ao = aaPoolValue;
						peatlandWaterTableFlux->addTransfer(_acrotelm_a, _acrotelm_o, aa2ao);

						double ca2co = computeCarbonTransfers(longtermWtd, currentAwtd, a, b);
						if (ca2co > caPoolValue) ca2co = caPoolValue;
						peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, ca2co);
					}
				}

				_landUnitData->submitOperation(peatlandWaterTableFlux);
				_landUnitData->applyOperations();
			}

			void PeatlandSpinupTurnOverModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {
				auto init = _landUnitData->createStockOperation();

				init->addTransfer(_atmosphere, _acrotelm_o, data["acrotelm"])
					->addTransfer(_atmosphere, _catotelm_a, data["catotelm"]);

				_landUnitData->submitOperation(init);
				_landUnitData->applyOperations();
			}
		}
	}
}