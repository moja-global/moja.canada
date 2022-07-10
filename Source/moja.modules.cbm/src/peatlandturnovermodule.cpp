#include "moja/modules/cbm/peatlandturnovermodule.h"
#include "moja/modules/cbm/printpools.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/logging.h>

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Set the value of the pools "Atmosphere", "WoodyFoilageLive", "WoodyStemsBranchesLive", "WoodyRootsLive", 
			 * "SedgeFoliageLive", "SedgeRootsLive", "SphagnumMossLive", "FeatherMossLive", "WoodyFoliageDead", 
			 * "WoodyFineDead", "WoodyCoarseDead", "WoodyRootsDead", "SedgeFoliageDead", "SedgeRootsDead", "FeathermossDead", 
			 * "Acrotelm_O", "Catotelm_A", "Acrotelm_A", "Catotelm_O" from _landUnitData to PeatlandTurnoverModule._atmosphere, PeatlandTurnoverModule._woodyFoliageLive, PeatlandTurnoverModule._woodyStemsBranchesLive,
			 * PeatlandTurnoverModule._sedgeFoliageLive, PeatlandTurnoverModule._sedgeRootsLive, PeatlandTurnoverModule._sphagnumMossLive, PeatlandTurnoverModule._featherMossLive, PeatlandTurnoverModule._woodyFoliageDead, PeatlandTurnoverModule._woodyFineDead,
			 * PeatlandTurnoverModule._woodyCoarseDead, PeatlandTurnoverModule._woodyRootsDead, PeatlandTurnoverModule._sedgeFoliageDead, PeatlandTurnoverModule._sedgeRootsDead, 
			 * PeatlandTurnoverModule._feathermossDead, PeatlandTurnoverModule._acrotelm_O, PeatlandTurnoverModule._catotelm_A,
			 * PeatlandTurnoverModule._acrotelm_A, PeatlandTurnoverModule._catotelm_O
			 * 
			 * Set value of variables "spinup_moss_only", "regen_delay", "base_wtd_parameters", "applied_annual_wtd" from _landUnitData
			 * to PeatlandTurnoverModule._spinupMossOnly, PeatlandTurnoverModule._regenDelay, PeatlandTurnoverModule.baseWTDParameters, PeatlandTurnoverModule._appliedAnnualWTD
			 * 
			 * @return void
			 */ 
			void PeatlandTurnoverModule::doLocalDomainInit() {
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
				_waterTableDepthModifier = _landUnitData->getVariable("peatland_annual_wtd_modifiers");
				_appliedAnnualWTD = _landUnitData->getVariable("applied_annual_wtd");
			}

			/**
			 * Set PeatlandTurnoverModule._runPeatland and  PeatlandTurnoverModule._modifiersFullyAppplied to false, PeatlandTurnoverModule._appliedAnnualWTD is reset \n
			 * If value of variable "load_peatpool_initials" in _landUnitData is not null, invoke PeatlandTurnoverModule.loadPeatlandInitialPoolValues() \n
			 * If the value of "peatland_class" in _landUnitData is not empty,
			 * assign turnoverParas a shared pointer of PeatlandGrowthParameters and set it to "peatland_turnover_parameters" in _landUnitData \n 
			 * assign growthParas a shared pointer of PeatlandGrowthParameters ans set it to "peatland_growth_parameters" in _landUnitData \n
			 * Set the result of PeatlandTurnoverModule.computeWaterTableDepth() to the three water table depths PeatlandTurnoverModule._forward_longterm_wtd, 
			 * PeatlandTurnoverModule._forward_previous_annual_wtd and PeatlandTurnoverModule._forward_current_annual_wtd 
			 * 
			 * @return void
			 */ 
			void PeatlandTurnoverModule::doTimingInit() {
				_runPeatland = false;
				_forward_wtd_modifier = "";
				_modifiersFullyAppplied = false;
				_appliedAnnualWTD->reset_value();

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

					auto& lnMDroughtCode = _landUnitData->getVariable("forward_drought_class")->value();
					auto& defaultLMDC = _landUnitData->getVariable("default_forward_drought_class")->value();
					auto lnMeanDroughtCode = lnMDroughtCode.isEmpty() ? defaultLMDC : lnMDroughtCode;
					auto lwtd = computeWaterTableDepth(lnMeanDroughtCode, _peatlandId);

					//set the long term water table depth variable value as initial status		
					_forward_longterm_wtd = lwtd;
					_forward_previous_annual_wtd = lwtd;
					_forward_current_annual_wtd = lwtd;
				}
			}

			/**
			 * If spinupMossOnly is true return \n
			 * If PeatlandTurnoverModule._runPeatland is true and value of PeatlandTurnoverModule._regenDelay > 0, indicating delay period, there is no growth, 
			 * invoke PeatlandTurnoverModule.doWaterTableFlux() to perform transfers between pools catotelm and acrotelm \n
			 * If PeatlandTurnoverModule._runPeatland is true and value of PeatlandTurnoverModule._regenDelay <= 0, invoke PeatlandTurnoverModule.updatePeatlandLivePoolValue() to update the current pool values, 
			 * PeatlandTurnoverModule.doLivePoolTurnover() to turnover on live pools and PeatlandTurnoverModule.doWaterTableFlux() to perform transfers between pools catotelm and acrotelm
			 * 
			 * @return void
			 */
			void PeatlandTurnoverModule::doTimingStep() {
				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) { return; }

				if (_runPeatland) {
					updateWaterTable();

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
			 * Extract the value of the current year from the argument modifierStr, if years > 0
			 * decrement the value of year by 1. After the decrement, if year is 0, set PeatlandTurnoverModule._forward_wtd_modifier to empty string and
			 * PeatlandTurnoverModule._modifiersFullyAppplied to true. Else if year != 0, then set PeatlandTurnoverModule._forward_wtd_modifier to the
			 * concatenation of the value of the current year and the value of the modifier from argument modifierStr
			 * 
			 * @param modifierStr string
			 * @return double
			 */
			double PeatlandTurnoverModule::getModifiedAnnualWTD(std::string modifierStr) {
				//set default current yeare WTD as forward long term WTD
				double newCurrentYearWtd = _forward_longterm_wtd;

				//new concept, after the disturbnace, apply the modifier(WTD) up to years
				std::string yearStr = modifierStr.substr(0, modifierStr.find_first_of("_"));
				int years = std::stoi(yearStr);

				std::string currentModiferStr = modifierStr.substr(modifierStr.find_first_of("_") + 1);
				int modifierValue = std::stoi(currentModiferStr);

				//default WTD modifier are "-1,0" or "0,0", years=0/-1, modifierValue=0
				//apply and update only if years > 0
				if (years > 0) {
					years -= 1;

					//use the modifier WTD to replace the current WTD, new concept
					newCurrentYearWtd = modifierValue;

					if (years == 0) {
						// years to modifier reached, no more modification further
						_forward_wtd_modifier = "";
						_modifiersFullyAppplied = true;
					}
					else {
						//update the forward modifier for remaining years
						_forward_wtd_modifier = std::to_string(years) + "_" + std::to_string(modifierValue);;
					}
				}
				return newCurrentYearWtd;
			}

			/**
			 * Update the water table
			 * 
			 * Set the value of variable "annual_drought_class" in _landUnitData to the current annual drought code 
			 * if it is not empty, else set it to the value of variable "default_annual_drought_class" in _landUnitData \n
			 * Invoke PeatlandTurnoverModule.computeWaterTableDepth() to compute the water table depth parameter to be used in current step with parameters as
			 * annual drought code and PeatlandTurnoverModule._peatlandId \n
			 * If PeatlandTurnoverModule._modifiersFullyAppplied is false, apply the water table depth modifier and update the current year water table depth \n
			 * if the local modifier, PeatlandTurnoverModule._forward_wtd_modifier, is empty, it means it is never set before, then check if there is a valid WTD modifer, 
			 * PeatlandTurnoverModule._waterTableDepthModifier, 
			 * trigged in the event.If there is a WTD modifier, get the valid WTD value for current step and update the modifiers accordingly, 
			 * set the new water table depth to the result of PeatlandTurnoverModule.getModifiedAnnualWTD() with parameter as PeatlandTurnoverModule._waterTableDepthModifier \n
			 * Else, if the forward WTD modifier, PeatlandTurnoverModule._forward_wtd_modifier, is already set, get the valid WTD value and update remaining modifiers accordingly, set the 
			 * result of PeatlandTurnoverModule.getModifiedAnnualWTD() with parameter as PeatlandTurnoverModule._forward_wtd_modifier to the new water table depth \n
			 * Set the previous annual WTD, PeatlandTurnoverModule._forward_previous_annual_wtd, with the old value (not updated) current water table value PeatlandTurnoverModule._forward_current_annual_wtd, 
			 * update the current WTD, PeatlandTurnoverModule._forward_current_annual_wtd with the newly computed WTD value, 
			 * post the updated PeatlandTurnoverModule._forward_current_annual_wtd as applied annual WTD by setting the value of PeatlandTurnoverModule._appliedAnnualWTD to 
			 * PeatlandTurnoverModule._forward_current_annual_wtd
			 * 
			 * @return void
			 */
			void PeatlandTurnoverModule::updateWaterTable() {
				//get the default annual drought code
				auto& defaultAnnualDC = _landUnitData->getVariable("default_annual_drought_class")->value();

				//get the current annual drought code
				auto& annualDC = _landUnitData->getVariable("annual_drought_class")->value();
				double annualDroughtCode = annualDC.isEmpty() ? defaultAnnualDC.convert<double>()
					: annualDC.type() == typeid(TimeSeries) ? annualDC.extract<TimeSeries>().value()
					: annualDC.convert<double>();

				//compute the water table depth parameter to be used in current step
				double newCurrentYearWtd = computeWaterTableDepth(annualDroughtCode, _peatlandId);

				//try to apply the WTD modifier and update the current year WTD
				if (!_modifiersFullyAppplied) {//modifier is neither applied nor used up
					//if the local modifier is empty, it means it is never set before
					if (_forward_wtd_modifier.empty()) {
						//then check if there is a valid WTD modifer trigged in event
						std::string waterTableModifier = _waterTableDepthModifier->value();
						if (!waterTableModifier.empty()) {
							//there is a WTD modifier, get the valid WTD value for current step
							//and updated the modifiers accordingly (remainging years to apply)
							newCurrentYearWtd = getModifiedAnnualWTD(waterTableModifier);
						}
					}
					else {
						//forward WTD modifier is already set, get the valid WTD value 
						//and update remaining modifiers accordingly
						newCurrentYearWtd = getModifiedAnnualWTD(_forward_wtd_modifier);
					}
				}// if modification years are used up, no more update

				//set the previous annual WTD with the not updated current WTD value
				_forward_previous_annual_wtd = _forward_current_annual_wtd;

				//update the current WTD with newly computed WTD value	
				_forward_current_annual_wtd = newCurrentYearWtd;

				//post the updated forward_current_annual_wtd as applied annual WTD
				_appliedAnnualWTD->set_value(_forward_current_annual_wtd);
			}

			/**
			 * Add transfers between the pools PeatlandTurnoverModule._catotelm_o, PeatlandTurnoverModule._catotelm_a, 
			 * PeatlandTurnoverModule._acrotelm_o, PeatlandTurnoverModule._acrotelm_a based on the values of 
			 * current annual water table depth PeatlandTurnoverModule._spinup_current_annual_wtd, 
			 * previous annual water table depth PeatlandTurnoverModule._spinup_previous_annual_wtd
			 * and long term annual water table depth PeatlandTurnoverModule._spinup_longterm_wtd \n
			 * Invoke createStockOperation() on _landUnitData, compute the flux using PeatlandTurnoverModule.computeCarbonTransfers() \n
			 * Submit the stock operation to the LandUnitData and apply the stock operation to the _landUnitData \n
			 * 
			 * @return void
			 */
			void PeatlandTurnoverModule::doWaterTableFlux() {
				//get current annual water table depth
				double currentAwtd = _forward_current_annual_wtd;

				//get previous annual water table depth
				double previousAwtd = _forward_previous_annual_wtd;

				//get long term annual water table depth
				double longtermWtd = _forward_longterm_wtd;

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
		}
	}
} // namespace moja::modules::cbm
