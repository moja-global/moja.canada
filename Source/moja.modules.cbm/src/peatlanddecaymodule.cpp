#include "moja/modules/cbm/peatlanddecaymodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <moja/modules/cbm/peatlandwtdbasefch4parameters.h>
namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 * 
			 * @param config const DynamicObject&
			 * @return void
			 **/
			void PeatlandDecayModule::configure(const DynamicObject& config) { }

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and  TimingStep
			 * 
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 **/
			void PeatlandDecayModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandDecayModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandDecayModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &PeatlandDecayModule::onTimingStep, *this);
			}

			/**
			 * Initialise PeatlandDecayModule._woodyFoliageDead, PeatlandDecayModule._woodyFineDead, PeatlandDecayModule._woodyCoarseDead, 
			 * PeatlandDecayModule._woodyRootsDead, PeatlandDecayModule._sedgeFoliageDead, PeatlandDecayModule._sedgeRootsDead, 
			 * PeatlandDecayModule._feathermossDead, PeatlandDecayModule._acrotelm_o, PeatlandDecayModule._catotelm_a, PeatlandDecayModule._acrotelm_a, 
			 * PeatlandDecayModule._catotelm_o, PeatlandDecayModule._co2, PeatlandDecayModule._ch4, PeatlandDecayModule._tempCarbon
			 *  with the pools "WoodyFoliageDead", "WoodyFineDead", "WoodyCoarseDead", "WoodyRootsDead",
			 * "SedgeFoliageDead", "SedgeRootsDead", "FeathermossDead", "Acrotelm_o", "Catotelm_a", "Acrotelm_a", 
			 * "Catotelm_o", "CO2", "CH4", "TempCarbon" in _landUnitData \n
			 * Initialise PeatlandDecayModule._spinupMossOnly, PeatlandDecayModule.baseWTDParameters, PeatlandDecayModule._appliedAnnualWTD value of variables
			 * "spinup_moss_only", "base_wtd_parameters", "applied_annual_wtd" in _landUnitData
			 * 
			 * @return void
			 **/
			void PeatlandDecayModule::doLocalDomainInit() {
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

				_co2 = _landUnitData->getPool("CO2");
				_ch4 = _landUnitData->getPool("CH4");
				_tempCarbon = _landUnitData->getPool("TempPeatlandDecayCarbon");
				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
				baseWTDParameters = _landUnitData->getVariable("base_wtd_parameters")->value().extract<DynamicObject>();

				_appliedAnnualWTD = _landUnitData->getVariable("applied_annual_wtd");
			}

			/**
			 * Set the value of PeatlandDecayModule._runPeatland to false \n
			 * If the value of variable "peatland_class" in _landUnitData is > 0, set PeatlandDecayModule._runPeatland as true. \n
			 * Create a variable meanAnnualTemperature, set it to the value of variable "mean_annual_temperature" in _landUnitData 
			 * if not empty, else to the value of variable "default_mean_annual_temperature" in _landUnitData \n
			 * Assign value of variables "peatland_decay_parameters", "peatland_turnover_parameters" in _landUnitData to 
			 * PeatlandDecayModule.decayParas, a shared pointer of type PeatlandDecayParameters, PeatlandDecayModule.turnoverParas, 
			 * a shared pointer of type PeatlandTurnoverParameters \n
			 * Invoke PeatlandDecayParameters.updateAppliedDecayParameters() on PeatlandDecayModule.decayParas with argument 
			 * as variable meanAnnualTemperature \n
			 * Assign a shared pointer of type PeatlandWTDBaseFCH4Parameters to PeatlandDecayModule.wtdFch4Paras, 
			 * invoke PeatlandWTDBaseFCH4Parameters.setValue() with argument as value of variable 
			 * "peatland_wtd_base_parameters" in _landUnitData \n
			 * PeatlandWTDBaseFCH4Parameters.setFCH4Value() with argument as value of variable 
			 * "peatland_fch4_max_parameters" in _landUnitData 
			 * 
			 * @return void
			 */ 
			void PeatlandDecayModule::doTimingInit() {
				_runPeatland = false;

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					if (_peatlandId > 0) {
						_runPeatland = true;

						//get the mean anual temperture variable
						const auto& defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();
						const auto& matVal = _landUnitData->getVariable("mean_annual_temperature")->value();
						_meanAnnualTemperature = matVal.isEmpty() ? defaultMAT : matVal;

						//get all parameters
						updateParameters();

						//get and set water table depth related parameter
						auto& peatlandWTDBaseParams = _landUnitData->getVariable("peatland_wtd_base_parameters")->value();
						auto& fch4MaxParams = _landUnitData->getVariable("peatland_fch4_max_parameters")->value();

						wtdFch4Paras = std::make_shared<PeatlandWTDBaseFCH4Parameters>();
						wtdFch4Paras->setValue(peatlandWTDBaseParams.extract<DynamicObject>());
						wtdFch4Paras->setFCH4Value(fch4MaxParams.extract<DynamicObject>());
					}
				}
			}


			/**
			 * If PeatlandDecayModule._runPeatland is false or _spinupMossOnly is true, return \n
			 * Else, invoke PeatlandDecayModule.doDeadPoolTurnover(), PeatlandDecayModule.doPeatlandNewCH4ModelDecay() 
			 * with argument as the result of PeatlandDecayParameters.Pt() on 
			 * PeatlandDecayModule.decayParas \n, PeatlandDecayModule.allocateCh4CO2() with argument as the current value of the water table
			 * given by the value of PeatlandDecayModule._appliedAnnualWTD
			 * 
			 * @return void
			 */
			void PeatlandDecayModule::doTimingStep() {
				if (!_runPeatland) { return; }

				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) { return; }

				//check peatland at current step
				//peatland of this Pixel may be changed due to disturbance and transition
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				int peatlandIdAtCurrentStep = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (peatlandIdAtCurrentStep != _peatlandId) {
					_peatlandId = peatlandIdAtCurrentStep;
					updateParameters();
				}

				//get current applied annual water table depth
				double awtd = _appliedAnnualWTD->value();

				//test degug output, time to print the pool values to check
				//PrintPools::printPeatlandPools("Year ", *_landUnitData);
				double deadPoolTurnoverRate = decayParas->Pt();

				doDeadPoolTurnover(deadPoolTurnoverRate);
				doPeatlandNewCH4ModelDecay(deadPoolTurnoverRate);
				allocateCh4CO2(awtd);

				//old CO2/CH4 model
				//doPeatlandDecay(deadPoolTurnoverRate, awtd);			
			}

			/**
			 * Get the current water table depth
			 * 
			 * Assign a variable annualDroughtCode, value of variable "annual_drought_class" in _landUnitData else,
			 * value of variable "default_annual_drought_class" in _landUnitData \n
			 * Assign a variable newCurrentYearWtd, compute the water table depth to be used in current step, the result of PeatlandDecayModule.computeWaterTableDepth() with arguments as 
			 * annualDroughtCode, PeatlandDecayModule._peatlandId \n
			 * If there is a valid modified annual WTD for forward run only, if value of PeatlandDecayModule._appliedAnnualWTD < 0, assign it to newCurrentYearWtd \n
			 * Return newCurrentYearWtd
			 * 
			 * @return double
			 */
			double PeatlandDecayModule::getCurrentYearWaterTable() {
				//get the default annual drought code
				auto& defaultAnnualDC = _landUnitData->getVariable("default_annual_drought_class")->value();

				//get the current annual drought code
				auto& annualDC = _landUnitData->getVariable("annual_drought_class")->value();
				double annualDroughtCode = annualDC.isEmpty() ? defaultAnnualDC.convert<double>()
					: annualDC.type() == typeid(TimeSeries) ? annualDC.extract<TimeSeries>().value()
					: annualDC.convert<double>();

				//compute the water table depth to be used in current step
				double newCurrentYearWtd = computeWaterTableDepth(annualDroughtCode, _peatlandId);
				double modifiedAnnualWtd = _appliedAnnualWTD->value();

				if (modifiedAnnualWtd < 0.0) {
					//there is a valid modified annual WTD for forward run only
					newCurrentYearWtd = modifiedAnnualWtd;
				}

				return newCurrentYearWtd;
			}

			/**
			 * Get the water table depth corresponding to the peatlandID
			 * 
			 * Return -0.045 * dc + base water table depth in PeatlandDecayModule.baseWTDParameters for parameter peatlandID  
			 * 
			 * @param dc double
			 * @param peatlandId int
			 * @return double
			 ***/
			double PeatlandDecayModule::computeWaterTableDepth(double dc, int peatlandID) {
				double retVal = 0.0;

				std::string peatlandIDStr = std::to_string(peatlandID);
				double wtdBaseValue = baseWTDParameters[peatlandIDStr];
				retVal = -0.045 * dc + wtdBaseValue;

				return retVal;
			}

			/**
			 * Invoke createProportionalOperation() on _landUnitData \n
			 * Add all the <a href="https://github.com/moja-global/moja.canada/blob/9c9a65181700ceaf364ce01680de8dd610b95e16/Source/moja.modules.cbm/src/peatlanddecaymodule.cpp#L145">transfers</a> from source to sink pools  \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers, and applyOperations() to apply the transfers.
			 * 
			 * @param deadPoolTurnoverRate double
			 * @return void
			 **/
			void PeatlandDecayModule::doDeadPoolTurnover(double deadPoolTurnoverRate) {
				auto peatlandDeadPoolTurnover = _landUnitData->createProportionalOperation();
				peatlandDeadPoolTurnover
					->addTransfer(_woodyFoliageDead, _acrotelm_o, (turnoverParas->Pfe() * decayParas->akwfe() +
						turnoverParas->Pfn() * decayParas->akwfne()) * deadPoolTurnoverRate)
					->addTransfer(_woodyFineDead, _acrotelm_o, decayParas->akwsb() * deadPoolTurnoverRate)
					->addTransfer(_woodyCoarseDead, _acrotelm_o, decayParas->akwc() * deadPoolTurnoverRate)
					->addTransfer(_woodyRootsDead, _acrotelm_o, decayParas->akwr() * deadPoolTurnoverRate)
					->addTransfer(_sedgeFoliageDead, _acrotelm_o, decayParas->aksf() * deadPoolTurnoverRate)
					->addTransfer(_sedgeRootsDead, _acrotelm_o, decayParas->aksr() * deadPoolTurnoverRate)
					->addTransfer(_feathermossDead, _acrotelm_o, decayParas->akfm() * deadPoolTurnoverRate)
					->addTransfer(_acrotelm_o, _catotelm_a, decayParas->aka() * deadPoolTurnoverRate);
				_landUnitData->submitOperation(peatlandDeadPoolTurnover);
				_landUnitData->applyOperations();
			}

			/**
			 * Invoke createProportionalOperation() on _landUnitData \n
			 * Add all the <a href="https://github.com/moja-global/moja.canada/blob/9c9a65181700ceaf364ce01680de8dd610b95e16/Source/moja.modules.cbm/src/peatlanddecaymodule.cpp#L161">transfers</a> from source to sink pools  \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers, and applyOperations() to apply the transfers.
			 * 
			 * @param deadPoolTurnoverRate double
			 * @return void
			 **/
			void PeatlandDecayModule::doPeatlandNewCH4ModelDecay(double deadPoolTurnoverRate) {
				auto peatlandDeadPoolDecay = _landUnitData->createProportionalOperation();
				peatlandDeadPoolDecay
					->addTransfer(_woodyFoliageDead, _tempCarbon, (1 - deadPoolTurnoverRate) * (turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()))
					->addTransfer(_woodyFineDead, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akwsb())
					->addTransfer(_woodyCoarseDead, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akwc())
					->addTransfer(_woodyRootsDead, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akwr())
					->addTransfer(_sedgeFoliageDead, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->aksf())
					->addTransfer(_sedgeRootsDead, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->aksr())
					->addTransfer(_feathermossDead, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akfm())
					->addTransfer(_acrotelm_o, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->aka())
					->addTransfer(_catotelm_a, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akc())
					->addTransfer(_acrotelm_a, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akaa())
					->addTransfer(_catotelm_o, _tempCarbon, (1 - deadPoolTurnoverRate) * decayParas->akco());
				_landUnitData->submitOperation(peatlandDeadPoolDecay);
				_landUnitData->applyOperations();
			}

			/**
			 * Get the CO2 an CH4 portions and perform transfers from temporary carbon pool
			 * 
			 * Create a variable ch4Portion with initial value 0.0, if parameter awtd is greater than
			 * PeatlandWTDBaseFCH4Parameters.OptCH4WTD() on PeatlandDecayModule.wtdFch4Paras, set ch4Portion to FCH4max * pow(F10r, ((OptCH4WTD - awtd) / 10.0)), 
			 * else to FCH4max * pow(F10d, ((OptCH4WTD - awtd) / 10.0)), where 
			 * FCH4max is the result of PeatlandWTDBaseFCH4Parameters.FCH4_max(), F10d is the result of PeatlandWTDBaseFCH4Parameters.F10d(), F10r is the result of PeatlandWTDBaseFCH4Parameters.F10r(), OptCH4WTD is the result of PeatlandWTDBaseFCH4Parameters.OptCH4WTD() 
			 * on PeatlandDecayModule.wtdFch4Paras \n
			 * A variable co2Portion is assgined the difference of PeatlandDecayModule._tempCarbon and variable ch4Portion \n
			 * If variables ch4Portion, co2Portion and PeatlandDecayModule._tempCarbon are greater than 0.0, create a proportionalOperation() on _landUnitData
			 * and add a transfer of ch4Portion from source PeatlandDecayModule._tempCarbon to sink PeatlandDecayModule._ch4, \
			 * and a transfer of co2Portion from source PeatlandDecayModule._tempCarbon to sink PeatlandDecayModule._co2
			 * Submit the proportionalOperation() on _landUnitData and applyOperations() to apply the transfers.
			 * 
			 * @param awtd double
			 * @return void
			 ***/ 
			void PeatlandDecayModule::allocateCh4CO2(double awtd) {
				double OptCH4WTD = this->wtdFch4Paras->OptCH4WTD();
				double F10d = this->wtdFch4Paras->F10d();
				double F10r = this->wtdFch4Paras->F10r();
				double FCH4max = this->wtdFch4Paras->FCH4_max();
				double ch4Portion = 0.0;

				if (OptCH4WTD < awtd) {
					ch4Portion = FCH4max * pow(F10r, ((OptCH4WTD - awtd) / 10.0));
				}
				else {
					ch4Portion = FCH4max * pow(F10d, ((OptCH4WTD - awtd) / 10.0));
				}

				double tempCPoolValue = _tempCarbon->value();
				double co2Portion = tempCPoolValue - ch4Portion;
				if (tempCPoolValue > 0.0 && co2Portion > 0.0 && ch4Portion > 0.0)
				{
					auto peatlandDeadPoolDecay = _landUnitData->createStockOperation();
					peatlandDeadPoolDecay
						->addTransfer(_tempCarbon, _ch4, ch4Portion)
						->addTransfer(_tempCarbon, _co2, co2Portion);

					_landUnitData->submitOperation(peatlandDeadPoolDecay);
					_landUnitData->applyOperations();
				}
			}

			/**
			 * Invoke createProportionalOperation() on _landUnitData \n
			 * Add all the <a href="https://github.com/moja-global/moja.canada/blob/9c9a65181700ceaf364ce01680de8dd610b95e16/Source/moja.modules.cbm/src/peatlanddecaymodule.cpp#L211">transfers</a> from source to sink pools  \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers, and applyOperations() to apply the transfers.
			 * 
			 * @param deadPoolTurnoverRate double
			 * @param awtd double
			 * @return void
			 **/
			void PeatlandDecayModule::doPeatlandDecay(double deadPoolTurnoverRate, double awtd) {
				//special turnover rate for catotelm pool -> (Catotelm *'akc) and (Acrotelm *(1-Pt)*aka 
				//set zeroTurnoverRate to utilize the getToCO2Rate() and getToCH4Rate() function
				double zeroTurnoverRate = 0.0;

				auto peatlandDeadPoolDecay = _landUnitData->createProportionalOperation();
				peatlandDeadPoolDecay
					->addTransfer(_acrotelm_o, _co2, getToCO2Rate(decayParas->aka(), deadPoolTurnoverRate, awtd))
					->addTransfer(_acrotelm_o, _ch4, getToCH4Rate(decayParas->aka(), deadPoolTurnoverRate, awtd))
					->addTransfer(_acrotelm_a, _co2, getToCO2Rate(decayParas->akaa(), deadPoolTurnoverRate, awtd))
					->addTransfer(_acrotelm_a, _ch4, getToCH4Rate(decayParas->akaa(), deadPoolTurnoverRate, awtd))

					->addTransfer(_catotelm_a, _co2, getToCO2Rate(decayParas->akc(), zeroTurnoverRate, awtd))
					->addTransfer(_catotelm_a, _ch4, getToCH4Rate(decayParas->akc(), zeroTurnoverRate, awtd))
					->addTransfer(_catotelm_o, _co2, getToCO2Rate(decayParas->akco(), zeroTurnoverRate, awtd))
					->addTransfer(_catotelm_o, _ch4, getToCH4Rate(decayParas->akco(), zeroTurnoverRate, awtd))

					->addTransfer(_woodyFoliageDead, _co2, getToCO2Rate((turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()), deadPoolTurnoverRate, awtd))
					->addTransfer(_woodyFoliageDead, _ch4, getToCH4Rate((turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()), deadPoolTurnoverRate, awtd))

					->addTransfer(_woodyFineDead, _co2, getToCO2Rate(decayParas->akwsb(), deadPoolTurnoverRate, awtd))
					->addTransfer(_woodyFineDead, _ch4, getToCH4Rate(decayParas->akwsb(), deadPoolTurnoverRate, awtd))
					->addTransfer(_woodyCoarseDead, _co2, getToCO2Rate(decayParas->akwc(), deadPoolTurnoverRate, awtd))
					->addTransfer(_woodyCoarseDead, _ch4, getToCH4Rate(decayParas->akwc(), deadPoolTurnoverRate, awtd))

					->addTransfer(_woodyRootsDead, _co2, getToCO2Rate(decayParas->akwr(), deadPoolTurnoverRate, awtd))
					->addTransfer(_woodyRootsDead, _ch4, getToCH4Rate(decayParas->akwr(), deadPoolTurnoverRate, awtd))

					->addTransfer(_sedgeFoliageDead, _co2, getToCO2Rate(decayParas->aksf(), deadPoolTurnoverRate, awtd))
					->addTransfer(_sedgeFoliageDead, _ch4, getToCH4Rate(decayParas->aksf(), deadPoolTurnoverRate, awtd))

					->addTransfer(_sedgeRootsDead, _co2, getToCO2Rate(decayParas->aksr(), deadPoolTurnoverRate, awtd))
					->addTransfer(_sedgeRootsDead, _ch4, getToCH4Rate(decayParas->aksr(), deadPoolTurnoverRate, awtd))

					->addTransfer(_feathermossDead, _co2, getToCO2Rate(decayParas->akfm(), deadPoolTurnoverRate, awtd))
					->addTransfer(_feathermossDead, _ch4, getToCH4Rate(decayParas->akfm(), deadPoolTurnoverRate, awtd));
				_landUnitData->submitOperation(peatlandDeadPoolDecay);
				_landUnitData->applyOperations();
			}

			/**
			 * Get the total CH4 production rate for the given parameters.
			 * 
			 * Return, rate * (1 - deadPoolTurnoverRate) * (awtd * decayParas->c() + decayParas->d()), 
			 * where rate, deadPoolTurnoverRate, and awtd are parameters, PeatlandDecayParameters.c(), PeatlandDecayParameters.d() are 
			 * invoked on PeatlandDecayModule.decayParas
			 * 
			 * @param rate double
			 * @param deadPoolTurnoverRate double
			 * @param awtd double
			 * @return double
			 **/
			double PeatlandDecayModule::getToCH4Rate(double rate, double deadPoolTurnoverRate, double awtd) {
				double retVal = 0.0;
				retVal = rate * (1 - deadPoolTurnoverRate) * (awtd * decayParas->c() + decayParas->d());
				return retVal;
			}

			/**
			 * Get the total CO2 production rate for the given parameters.
			 * 
			 * Return, rate * (1 - deadPoolTurnoverRate) * (1 - (awtd * decayParas->c() + decayParas->d())), 
			 * where rate, deadPoolTurnoverRate, and awtd are parameters, PeatlandDecayParameters.c(), PeatlandDecayParameters.d() are 
			 * invoked on PeatlandDecayModule.decayParas
			 * 
			 * @param rate double
			 * @param deadPoolTurnoverRate double
			 * @param awtd double
			 * @return double
			 **/
			double PeatlandDecayModule::getToCO2Rate(double rate, double deadPoolTurnoverRate, double awtd) {
				double retVal = 0.0;
				retVal = rate * (1 - deadPoolTurnoverRate) * (1 - (awtd * decayParas->c() + decayParas->d()));
				return retVal;
			}

			void PeatlandDecayModule::updateParameters() {
				// 1) get the data by variable "peatland_decay_parameters"
				const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();
				//create the PeaglandDecayParameters, set the value from the variable
				decayParas = std::make_shared<PeatlandDecayParameters>();
				decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

				//compute the applied parameters
				decayParas->updateAppliedDecayParameters(_meanAnnualTemperature);

				// 2) get the data by variable "peatland_turnover_parameters"
				const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

				//create the PeatlandTurnoverParameters, set the value from the variable
				turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
				if (!peatlandTurnoverParams.isEmpty()) {
					turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());
				}
			}

		}
	}
} // namespace moja::modules::cbm
