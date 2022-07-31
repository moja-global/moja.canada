/**
 * @file
 * Prepare initial variables to simulate a peatland landunit (pixel)
 */ 
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

			/**
			 * Set the value of the pools "Atmosphere", "WoodyFoilageLive", "WoodyStemsBranchesLive", "WoodyRootsLive", 
			 * "SedgeFoliageLive", "SedgeRootsLive", "SphagnumMossLive", "FeatherMossLive", "WoodyFoliageDead", 
			 * "WoodyFineDead", "WoodyCoarseDead", "WoodyRootsDead", "SedgeFoliageDead", "SedgeRootsDead", "FeathermossDead", 
			 * "Acrotelm_O", "Catotelm_A", "Acrotelm_A", "Catotelm_O" from _landUnitData to PeatlandSpinupTurnOverModule._atmosphere, PeatlandSpinupTurnOverModule._woodyFoliageLive, PeatlandSpinupTurnOverModule._woodyStemsBranchesLive,
			 * PeatlandSpinupTurnOverModule._sedgeFoliageLive, PeatlandSpinupTurnOverModule._sedgeRootsLive, PeatlandSpinupTurnOverModule._sphagnumMossLive, PeatlandSpinupTurnOverModule._featherMossLive, PeatlandSpinupTurnOverModule._woodyFoliageDead, PeatlandSpinupTurnOverModule._woodyFineDead,
			 * PeatlandSpinupTurnOverModule._woodyCoarseDead, PeatlandSpinupTurnOverModule._woodyRootsDead, PeatlandSpinupTurnOverModule._sedgeFoliageDead, PeatlandSpinupTurnOverModule._sedgeRootsDead, 
			 * PeatlandSpinupTurnOverModule._feathermossDead, PeatlandSpinupTurnOverModule._acrotelm_O, PeatlandSpinupTurnOverModule._catotelm_A,
			 * PeatlandSpinupTurnOverModule._acrotelm_A, PeatlandSpinupTurnOverModule._catotelm_O
			 * 
			 * Set value of variables "spinup_moss_only", "regen_delay", "base_wtd_parameters", "applied_annual_wtd" from _landUnitData
			 * to PeatlandSpinupTurnOverModule._spinupMossOnly, PeatlandSpinupTurnOverModule._regenDelay, PeatlandSpinupTurnOverModule.baseWTDParameters, PeatlandSpinupTurnOverModule._appliedAnnualWTD
			 * 
			 * @return void
			 */ 
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
				_appliedAnnualWTD = _landUnitData->getVariable("applied_annual_wtd");
			}

			/**
			 * Set PeatlandSpinupTurnOverModule._runPeatland to false, PeatlandSpinupTurnOverModule._appliedAnnualWTD is only valid in forward run, reset it for spinup \n
			 * If value of variable "load_peatpool_initials" in _landUnitData is not null, invoke PeatlandSpinupTurnOverModule.loadPeatlandInitialPoolValues() \n
			 * If the value of "peatland_class" in _landUnitData is not empty, set PeatlandSpinupTurnOverModule._runPeatland to true \n
			 * assign turnoverParas a shared pointer of PeatlandGrowthParameters and set it to "peatland_turnover_parameters" in _landUnitData \n 
			 * assign growthParas a shared pointer of PeatlandGrowthParameters ans set it to "peatland_growth_parameters" in _landUnitData \n
			 * Set the result of PeatlandSpinupTurnOverModule.computeWaterTableDepth() to the three water table depths PeatlandSpinupTurnOverModule._spinup_longterm_wtd, 
			 * PeatlandSpinupTurnOverModule._spinup_previous_annual_wtd and PeatlandSpinupTurnOverModule._spinup_current_annual_wtd in spinup phase \n
			 * In spinup run, always set PeatlandSpinupTurnOverModule._appliedAnnualWTD same as spinup long term WTD, the result of PeatlandSpinupTurnOverModule.computeWaterTableDepth()
			 * 
			 * @return void
			 */ 
			void PeatlandSpinupTurnOverModule::doTimingInit() {
				_runPeatland = false;

				//applied_annual_wtd is only valid in forward run, reset it for spinup
				_appliedAnnualWTD->reset_value();

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

					//create the PeatlandGrowthParameters, set the value from the variable
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

					//In spinup run, always set applied annual wtd same as spinup long term WTD
					_appliedAnnualWTD->set_value(lwtd);
				}
			}

			/**
			 * If PeatlandSpinupTurnOverModule.spinupMossOnly is true, return \n
			 * Else, if PeatlandSpinupTurnOverModule._runPeatland is true, if value of PeatlandSpinupTurnOverModule._regenDelay > 0, there is no growth in delay
			 * period, invoke PeatlandSpinupTurnOverModule.doWaterTableFlux(), else, invoke PeatlandSpinupTurnOverModule.updatePeatlandLivePool() to update the current
			 * pool value, PeatlandSpinupTurnOverModule.doLiveTurnover() to turnover on live pools, 
			 * and PeatlandSpinupTurnOverModule.doWaterTableFlux() to update the  flux between catotelm and acrotelm due to water table changes
			 *
			 * @return void
			 */
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

			/**
			 * Add transfers between the pools PeatlandSpinupTurnOverModule._catotelm_o, PeatlandSpinupTurnOverModule._catotelm_a, 
			 * PeatlandSpinupTurnOverModule._acrotelm_o, PeatlandSpinupTurnOverModule._acrotelm_a based on the values of 
			 * current annual water table depth PeatlandSpinupTurnOverModule._spinup_current_annual_wtd, 
			 * previous annual water table depth PeatlandSpinupTurnOverModule._spinup_previous_annual_wtd
			 * and long term annual water table depth PeatlandSpinupTurnOverModule._spinup_longterm_wtd \n
			 * Invoke createStockOperation() on _landUnitData, compute the flux using PeatlandSpinupTurnOverModule.computeCarbonTransfers() \n
			 * Submit the stock operation to the LandUnitData and apply the stock operation to the _landUnitData \n
			 * 
			 * @return void
			 */
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

			/**
			 * Invoke createStockOperation() on _landUnitData \n
			 * Add transfers from source PeatlandSpinupTurnOverModule._atmosphere to sink PeatlandSpinupTurnOverModule._acrotelm_o, 
			 * transfer amount is value "acrotelm" in parameter data, source PeatlandSpinupTurnOverModule._atmosphere to sink PeatlandSpinupTurnOverModule._catotelm_o, 
			 * transfer amount is value "catotelm" in parameter data \n
			 * Submit the operation to _landUnitData, invoke submitOperation() and applyOperations() on _landUnitData
			 * 			 
			 * @param data const DynamicObject&
			 * @return void
			 */ 
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