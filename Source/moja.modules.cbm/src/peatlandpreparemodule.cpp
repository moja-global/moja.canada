#include "moja/modules/cbm/peatlandpreparemodule.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
	namespace modules {
		namespace cbm {

			void PeatlandPrepareModule::configure(const DynamicObject& config) {}

			void PeatlandPrepareModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandPrepareModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandPrepareModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &PeatlandPrepareModule::onTimingStep, *this);
			}

			void PeatlandPrepareModule::doLocalDomainInit() {
				_atmosphere = _landUnitData->getPool("Atmosphere");
				_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
				_catotelm_a = _landUnitData->getPool("Catotelm_A");

				baseWTDParameters = _landUnitData->getVariable("base_wtd_parameters")->value().extract<DynamicObject>();
			}

			void PeatlandPrepareModule::doTimingInit() {
				//for each landunit pixel, always reset water table depth variables before simulation
				resetWaterTableDepthValue();

				//for forward run test and debug purpose
				//_landUnitData->getVariable("peatland_shrub_age")->set_value(0);
				_runPeatland = _landUnitData->getVariable("run_peatland")->value();

				// if the land unit is eligible to run as peatland		
				if (_runPeatland) {
					peatlandID = _landUnitData->getVariable("peatlandId")->value();
					checkTreedOrForestPeatland(peatlandID);

					//load initial peat pool values if it is enabled
					auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();
					if (loadInitialFlag) {
						const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_stocks")->value();
						loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());
					}

					//get the long term average DC (drought code), compute long term water table depth
					auto& lnMDroughtCode = _landUnitData->getVariable("forward_drought_class")->value();
					auto& defaultLMDC = _landUnitData->getVariable("default_forward_drought_class")->value();
					auto lnMeanDroughtCode = lnMDroughtCode.isEmpty() ? defaultLMDC : lnMDroughtCode;
					auto lwtd = computeWaterTableDepth(lnMeanDroughtCode, peatlandID);

					//set the long term water table depth variable value			
					_landUnitData->getVariable("peatland_longterm_wtd")->set_value(lwtd);

					//for each peatland pixel, set the initial previous annual wtd same as the lwtd
					_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(lwtd);
				}
			}

			void PeatlandPrepareModule::doTimingStep() {
				if (_runPeatland) {
					//get the default annual drought code
					auto& defaultAnnualDC = _landUnitData->getVariable("default_annual_drought_class")->value();

					//get the current annual drought code
					auto& annualDC = _landUnitData->getVariable("annual_drought_class")->value();
					auto annualDroughtCode = annualDC.isEmpty() ? defaultAnnualDC.convert<double>()
						: annualDC.type() == typeid(TimeSeries) ? annualDC.extract<TimeSeries>().value()
						: annualDC.convert<double>();

					//compute the water table depth parameter to be used in current step
					auto newCurrentYearWtd = computeWaterTableDepth(annualDroughtCode, peatlandID);

					//get the potential annual water table modifer
					if (_landUnitData->hasVariable("peatland_annual_wtd_modifiers")) {
						auto wtdModifier = _landUnitData->getVariable("peatland_annual_wtd_modifiers");
						std::string modifierStr = wtdModifier->value();
						bool modifyAnualWTD = modifierStr.size() > 1;

						if (modifyAnualWTD) {
							std::size_t firstPos = modifierStr.find_first_of(";");
							std::string currentModiferStr;
							std::string remainingModifiers;

							//actual water table modifier value to be applied
							int modifierValue;

							if (firstPos != std::string::npos) {
								//old concept, after the disturbance, in following years
								//multiple entries: year1_modifier;year2_modifier...
								//get the first entry to use
								std::string newModifier = modifierStr.substr(0, firstPos);

								currentModiferStr = newModifier.substr(newModifier.find_first_of("_") + 1);
								modifierValue = std::stoi(currentModiferStr);

								//one modifier was taken and applied, update the remainings
								remainingModifiers = modifierStr.substr(firstPos + 1);

								_landUnitData->getVariable("peatland_annual_wtd_modifiers")->set_value(remainingModifiers);
							}
							else {
								//new concept, after the disturbnace, apply the modifier up to years
								//only one entry: year_modifier
								//apply the modifier for number of year
								std::string yearStr = modifierStr.substr(0, modifierStr.find_first_of("_"));
								int years = std::stoi(yearStr);

								currentModiferStr = modifierStr.substr(modifierStr.find_first_of("_") + 1);
								modifierValue = std::stoi(currentModiferStr);

								//default WTD modifier is "0,0", years=0, modifierValue=0
								//apply and update only if years > 0
								if (years > 0) {
									years -= 1;
									remainingModifiers = std::to_string(years) + "_" + std::to_string(modifierValue);

									//newCurrentYearWtd += modifier; // old concept

									//use the modifier to replace the current WTD, new concept
									newCurrentYearWtd = modifierValue;
									_landUnitData->getVariable("peatland_annual_wtd_modifiers")->set_value(remainingModifiers);
								}
							}
						}
					}

					//get the current water table depth which was used in last step, but not yet updated for current step
					auto currentWtdVar = _landUnitData->getVariable("peatland_current_annual_wtd");
					double currentWtd = currentWtdVar->value();
					if (currentWtd != 0) {
						//set the previous annual wtd with the not updated annual water table depth value
						_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(currentWtd);
					}

					//update the current annual wtd 		
					_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(newCurrentYearWtd);
				}
			}

			void PeatlandPrepareModule::resetWaterTableDepthValue() {
				_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(0.0);
				_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(0.0);
				_landUnitData->getVariable("peatland_longterm_wtd")->set_value(0.0);

				//also reset water table modifiers 
				_landUnitData->getVariable("peatland_annual_wtd_modifiers")->set_value("");
			}

			void PeatlandPrepareModule::checkTreedOrForestPeatland(int peatlandId) {
				switch (peatlandId) {
				case 3:		// forested bog
				case 6:		// forested poor fen
				case 9:		// forested rich fen
				case 11:	// forested swamp
					_isForestPeatland = true;
					break;
				case 2:		// treed bog
				case 5:		// treed poor fen
				case 8:		// treed rich fen
				case 10:	// treed swamp
					_isTreedPeatland = true;
					break;
				default:
					_isForestPeatland = false;
					_isTreedPeatland = false;
				}
			}

			double PeatlandPrepareModule::computeWaterTableDepth(double dc, int peatlandID) {
				std::string peatlandIDStr = std::to_string(peatlandID);
				double wtdBaseValue = baseWTDParameters[peatlandIDStr];
				double retVal = -0.045 * dc + wtdBaseValue;
				return retVal;
			}

			void PeatlandPrepareModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {
				auto init = _landUnitData->createStockOperation();

				init->addTransfer(_atmosphere, _acrotelm_o, data["acrotelm"])
					->addTransfer(_atmosphere, _catotelm_a, data["catotelm"]);

				//MOJA_LOG_INFO << "Acrotelm: " << (double)data["acrotelm"] << " catotelm: " << (double)data["catotelm"];
				_landUnitData->submitOperation(init);
			}
		}
	}
}