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

			void PeatlandDecayModule::configure(const DynamicObject& config) { }

			void PeatlandDecayModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandDecayModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandDecayModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &PeatlandDecayModule::onTimingStep, *this);
			}

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
			}

			void PeatlandDecayModule::doTimingInit() {
				_runPeatland = false;
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (_peatlandId > 0) {
					_runPeatland = true;

					//get the mean anual temperture variable
					const auto& defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();
					const auto& matVal = _landUnitData->getVariable("mean_annual_temperature")->value();
					double meanAnnualTemperature = matVal.isEmpty() ? defaultMAT : matVal;

					// 1) get the data by variable "peatland_decay_parameters"
					const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();
					//create the PeaglandDecayParameters, set the value from the variable
					decayParas = std::make_shared<PeatlandDecayParameters>();
					decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

					//compute the applied parameters
					decayParas->updateAppliedDecayParameters(meanAnnualTemperature);

					// 2) get the data by variable "peatland_turnover_parameters"
					const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

					//create the PeaglandGrowthParameters, set the value from the variable
					turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
					if (!peatlandTurnoverParams.isEmpty()) {
						turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());
					}

					// 3) get the DC (drought code), and then compute the wtd parameter
					auto& peatlandWTDBaseParams = _landUnitData->getVariable("peatland_wtd_base_parameters")->value();
					auto& fch4MaxParams = _landUnitData->getVariable("peatland_fch4_max_parameters")->value();

					wtdFch4Paras = std::make_shared<PeatlandWTDBaseFCH4Parameters>();
					wtdFch4Paras->setValue(peatlandWTDBaseParams.extract<DynamicObject>());
					wtdFch4Paras->setFCH4Value(fch4MaxParams.extract<DynamicObject>());
				}
			}

			void PeatlandDecayModule::doTimingStep() {
				if (!_runPeatland) { return; }

				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) { return; }

				//get current annual water table depth
				double awtd = getCurrentYearWaterTable();

				//test degug output, time to print the pool values to check
				//PrintPools::printPeatlandPools("Year ", *_landUnitData);
				double deadPoolTurnoverRate = decayParas->Pt();

				doDeadPoolTurnover(deadPoolTurnoverRate);
				doPeatlandNewCH4ModelDecay(deadPoolTurnoverRate);
				allocateCh4CO2(awtd);

				//old CO2/CH4 model
				//doPeatlandDecay(deadPoolTurnoverRate, awtd);			
			}

			double PeatlandDecayModule::getCurrentYearWaterTable() {
				//get the default annual drought code
				auto& defaultAnnualDC = _landUnitData->getVariable("default_annual_drought_class")->value();

				//get the current annual drought code
				auto& annualDC = _landUnitData->getVariable("annual_drought_class")->value();
				double annualDroughtCode = annualDC.isEmpty() ? defaultAnnualDC.convert<double>()
					: annualDC.type() == typeid(TimeSeries) ? annualDC.extract<TimeSeries>().value()
					: annualDC.convert<double>();

				//compute the water table depth parameter to be used in current step
				double newCurrentYearWtd = computeWaterTableDepth(annualDroughtCode, _peatlandId);

				return newCurrentYearWtd;
			}

			double PeatlandDecayModule::computeWaterTableDepth(double dc, int peatlandID) {
				double retVal = 0.0;

				std::string peatlandIDStr = std::to_string(peatlandID);
				double wtdBaseValue = baseWTDParameters[peatlandIDStr];
				retVal = -0.045 * dc + wtdBaseValue;

				return retVal;
			}

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

			double PeatlandDecayModule::getToCH4Rate(double rate, double deadPoolTurnoverRate, double awtd) {
				double retVal = 0.0;
				retVal = rate * (1 - deadPoolTurnoverRate) * (awtd * decayParas->c() + decayParas->d());
				return retVal;
			}

			double PeatlandDecayModule::getToCO2Rate(double rate, double deadPoolTurnoverRate, double awtd) {
				double retVal = 0.0;
				retVal = rate * (1 - deadPoolTurnoverRate) * (1 - (awtd * decayParas->c() + decayParas->d()));
				return retVal;
			}

			/**
			ToAirTotal =
				(D_W_Foliage *(1-Pt)(Pfe*akwfe) + Pfn*akwfne)) +
				(D_W_StemsBranches *(1-Pt)*akwsb +
				(D_W_Roots *(1-Pt)*akwr +
				(D_S_Foliage *(1-Pt)*aksf +
				(D_S_Roots *(1-Pt)*aksr +
				(D_Feather_Moss *(1-Pt)*akfm +
				(Acrotelm *(1-Pt)*aka +
				(Catotelm *'akc)

			ToCH4 = ToAirTotal * ((c * wtd) + d)
			ToCO2 = ToAirTotal - ToCH4
			*/
		}
	}
} // namespace moja::modules::cbm
