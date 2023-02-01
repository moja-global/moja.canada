/**
 * @file
 * Parameters for moss related computing
 **********************/
#include "moja/modules/cbm/mossdecaymodule.h"
#include "moja/modules/cbm/helper.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>


namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 *
			 * @param config const DynamicObject&
			 * @return void
			 * *********************/
			void MossDecayModule::configure(const DynamicObject& config) { }

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 *
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 * **************************/
			void MossDecayModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossDecayModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossDecayModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &MossDecayModule::onTimingStep, *this);
			}

			/**
			 * Initialise MossDecayModule._featherMossFast, MossDecayModule._sphagnumMossFast, MossDecayModule._featherMossSlow, MossDecayModule._sphagnumMossSlow,
			 * MossDecayModule._CO2 value of "FeatherMossFast", SphagnumMossFast", "FeatherMossSlow", "SphagnumMossSlow", "CO2" in _landUnitData
			 *
			 * Initialise MossDecayModule._mossParameters as variable "moss_parameters" in _landUnitData,  \n
			 * MossDecayModule.fastToSlowTurnoverRate, MossDecayModule.fastToAirDecayRate, MossDecayModule.kff, MossDecayModule.ksf,
			 * MossDecayModule.kfs, MossDecayModule.kss, MossDecayModule.q10, MossDecayModule.tref, MossDecayModule.m, MossDecayModule.n values of
			 * "fastToSlowTurnoverRate", "fastToAirDecayRate", "kff", "ksf", "kfs", "kss", "q10", "tref", "m", "n" in MossDecayModule._mossParameters
			 *
			 * @return void
			 * ***************************/
			void MossDecayModule::doLocalDomainInit() {
				_featherMossFast = _landUnitData->getPool("FeatherMossFast");
				_sphagnumMossFast = _landUnitData->getPool("SphagnumMossFast");
				_featherMossSlow = _landUnitData->getPool("FeatherMossSlow");
				_sphagnumMossSlow = _landUnitData->getPool("SphagnumMossSlow");
				_CO2 = _landUnitData->getPool("CO2");

				_mossParameters = _landUnitData->getVariable("moss_parameters");
				const auto& mossGrowthParameters = _mossParameters->value().extract<DynamicObject>();

				fastToSlowTurnoverRate = mossGrowthParameters["fastToSlowTurnoverRate"];
				fastToAirDecayRate = mossGrowthParameters["fastToAirDecayRate"];

				kff = mossGrowthParameters["kff"];
				ksf = mossGrowthParameters["ksf"];
				kfs = mossGrowthParameters["kfs"];
				kss = mossGrowthParameters["kss"];
				q10 = mossGrowthParameters["q10"];
				tref = mossGrowthParameters["tref"];

				m = mossGrowthParameters["m"];
				n = mossGrowthParameters["n"];
			};

			/**
			 * If variable "enable_moss" exists in _landUnitData and it has a value,
			 * assign MossDecayModule.meanAnnualTemperature the value of variable "default_mean_annual_temperature" in _landUnitData if value of variable "mean_annual_temperature" is empty. \n
			 * Invoke Helper.runMoss() with arguments as value of variables "growth_curve_id", "moss_leading_species" and "leading_species" in _landUnitData \n
			 * Assign MossDisturbanceModule.runMoss to true if variable "peatland_class" in _landUnitData is empty, variable "growth_curve_id" in _landUnitData
			 * is not empty, and Helper.runMoss() returns true
			 *
			 * @return void
			 * **************************/
			void MossDecayModule::doTimingInit() {
				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {

					double defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();

					auto matVal = _landUnitData->getVariable("mean_annual_temperature")->value();
					meanAnnualTemperature = matVal.isEmpty() ? defaultMAT
						: matVal.type() == typeid(TimeSeries) ? matVal.extract<TimeSeries>().value()
						: matVal.convert<double>();

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
					bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

					auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
					auto speciesName = _landUnitData->getVariable("leading_species")->value();

					runMoss = peatlandId < 0
						&& isGrowthCurveDefined
						&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
				}
			};

			/**
			 * If MossDecayModule.runMoss is true, and the value of variable "growth_curve_id" in _landUnitData > 0,
			 * Get the annual maximumVolume of the stand as, invoke StandGrowthCurve.getAnnualStandMaximumVolume() on the result of StandGrowthCurveFactory.getStandGrowthCurve()
			 * on MossDecayModule._gcFactory with argument as the value of variable "growth_curve_id" in _landUnitData \n
			 * Invoke MossDecayModule.updateMossAppliedDecayParameters() with arguments maximumVolume, MossDecayModule.meanAnnualTemperature, MossDecayModule.doMossFastPoolDecay(), MossDecayModule.doMossSlowPoolDecay()
			 *
			 * @return void
			 ***********************************/
			void MossDecayModule::doTimingStep() {
				if (runMoss) {
					currentStandGCId = _landUnitData->getVariable("growth_curve_id")->value();

					//get the mean anual temperture variable
					double defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();

					auto matVal = _landUnitData->getVariable("mean_annual_temperature")->value();
					meanAnnualTemperature = matVal.isEmpty() ? defaultMAT
						: matVal.type() == typeid(TimeSeries) ? matVal.extract<TimeSeries>().value()
						: matVal.convert<double>();

					//if negative growth curve, the stand is deforested.
					if (currentStandGCId < 0) return;

					auto maximumVolume = _gcFactory->getStandGrowthCurve(currentStandGCId)->getAnnualStandMaximumVolume();

					updateMossAppliedDecayParameters(maximumVolume, meanAnnualTemperature);
#if 0
					auto pools = _landUnitData->poolCollection();
					int ageValue = _landUnitData->getVariable("age")->value();
					MOJA_LOG_INFO << "Stand Age: " << ageValue - 1 << ", " <<
						pools.findPool("FeatherMossLive")->value() << ", " <<
						pools.findPool("SphagnumMossLive")->value() << ", " <<
						pools.findPool("FeatherMossFast")->value() << ", " <<
						pools.findPool("FeatherMossSlow")->value() << ", " <<
						pools.findPool("SphagnumMossFast")->value() << ", " <<
						pools.findPool("SphagnumMossSlow")->value();
#endif
					doMossFastPoolDecay();
					doMossSlowPoolDecay();
				}
			};

			//moss fast pool turnover and decay
			/**
			 * Moss fast pool turnover and decay
			 *
			 * Invoke createStockOperation() on _landUnitData \n
			 * Add feather fast to slow and to air transfers between source MossDecayModule._featherMossFast and sink MossDecayModule._featherMossSlow
			 * with transfer value MossDecayModule._featherMossFast * MossDecayModule.akff * MossDecayModule.fastToSlowTurnoverRate,
			 * source MossDecayModule._featherMossFast and sink MossDecayModule._CO2 with transfer value
			 * MossDecayModule._featherMossFast * MossDecayModule.akff * MossDecayModule.fastToAirDecayRate \n
			 * Add sphagnum fast to slow and to air transfers between source MossDecayModule._sphagnumMossFast and sink MossDecayModule._sphagnumMossSlow with transfer value
			 * MossDecayModule._sphagnumMossFast * MossDecayModule.aksf * MossDecayModule.fastToSlowTurnoverRate
			 * source MossDecayModule._sphagnumMossFast and sink MossDecayModule._CO2 with transfer value MossDecayModule._sphagnumMossFast * MossDecayModule.aksf * MossDecayModule.fastToAirDecayRate \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers
			 *
			 * @return void
			 * *****************************/
			void MossDecayModule::doMossFastPoolDecay() {
				auto mossFastDecay = _landUnitData->createStockOperation();

				// feather fast to slow and to air
				double featherMossFastLoss = _featherMossFast->value() * akff;
				double featherFastToSlowTurnoverAmount = featherMossFastLoss * fastToSlowTurnoverRate;
				double featherFastDecayToAirAmount = featherMossFastLoss * fastToAirDecayRate;

				mossFastDecay->addTransfer(_featherMossFast, _featherMossSlow, featherFastToSlowTurnoverAmount);
				mossFastDecay->addTransfer(_featherMossFast, _CO2, featherFastDecayToAirAmount);

				//sphagnum fast to slow and to air
				double sphagnumFastLoss = _sphagnumMossFast->value() * aksf;
				double sphagnumFastToSlowTurnoverAmount = sphagnumFastLoss * fastToSlowTurnoverRate;
				double sphagnumFastDecayToAirAmount = sphagnumFastLoss * fastToAirDecayRate;

				mossFastDecay->addTransfer(_sphagnumMossFast, _sphagnumMossSlow, sphagnumFastToSlowTurnoverAmount);
				mossFastDecay->addTransfer(_sphagnumMossFast, _CO2, sphagnumFastDecayToAirAmount);

				_landUnitData->submitOperation(mossFastDecay);

			}

			//moss slow pool decay only
			/**
			 * Moss slow pool decay only
			 * Invoke createProportionalOperation() on _landUnitData, add a transfer of MossDecayModule.akfs from source MossDecayModule._featherMossSlow to sink MossDecayModule._CO2,
			 * transfer of MossDecayModule.akss from source MossDecayModule._sphagnumMossSlow to sink MossDecayModule._CO2 \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers
			 *
			 * @return void
			 * *************************/
			void MossDecayModule::doMossSlowPoolDecay() {
				auto mossSlowDecay = _landUnitData->createProportionalOperation();

				mossSlowDecay->addTransfer(_featherMossSlow, _CO2, akfs);
				mossSlowDecay->addTransfer(_sphagnumMossSlow, _CO2, akss);

				_landUnitData->submitOperation(mossSlowDecay);
			}

			//Sphagnum slow pool base decay rate, kss = m*ln(maxVolume) + n
			/**
			 * Return the Sphagnum slow pool base decay rate, applied on pool MossDecayModule.kss, given as  m * ln(maxVolume) + n
			 *
			 * @param m double
			 * @param n double
			 * @param maxVolume double
			 * @return double
			 * *********************/
			double MossDecayModule::F6(double m, double n, double maxVolume) {
				double value = m * log(maxVolume) + n;
				return value;
			}

			//Applied decay rate to all moss pools: kff, kfs, ksf, kss (kff*(e^((MAT-10)*(ln(Q10)*0.1))
			//kff - feather fast decay rate 
			//kfs - feather slow decay rate
			//ksf - sphagnum fast decay rate
			//kss - sphagnum slow decay rate
			/**
			 * Applied decay rate to all moss pools :kff, kfs, ksf, kss, given as (baseDecayRate * (e ^ ((meanAnnualTemperature - 10) * (ln(q10) * 0.1))
			 *
			 * @param baseDecayRate double
			 * @param meanAnnualTemperature double
			 * @param q10 double
			 * @return double
			 * **************************/
			double MossDecayModule::F7(double baseDecayRate, double meanAnnualTemperature, double q10) {
				double expValue = exp((meanAnnualTemperature - tref) * log(q10) * 0.1);
				double retValue = baseDecayRate * expValue;

				return retValue;
			}

			/**
			 * Update moss pool base decay rate based on mean annual temperature and q10 value
			 *
			 * Assign MossDecayModule.kss, sphagnum slow decay rate, result of MossDecayModule.F6() with arguments MossDecayModule.m, MossDecayModule.n and parameter standMaximumVolume \n
			 * MossDecayModule.akff, applied feather moss fast pool applied decay rate, the result of MossDecayModule.F7() with arguments MossDecayModule.kff, parameter meanAnnualTemperature and MossDecayModule.q10 \n
			 * MossDecayModule.akfs, applied feather moss slow pool applied decay rate, the result of MossDecayModule.F7() with arguments MossDecayModule.kfs, parameter meanAnnualTemperature and MossDecayModule.q10 \n
			 * MossDecayModule.aksf, applied sphagnum fast pool applied decay rate, the result of MossDecayModule.F7() with arguments MossDecayModule.ksf, parameter meanAnnualTemperature and MossDecayModule.q10 \n
			 * MossDecayModule.akss, applied sphagnum slow pool applied decay rate, the result of MossDecayModule.F7() with arguments MossDecayModule.kss, parameter meanAnnualTemperature and MossDecayModule.q10
			 *
			 * @param standMaximumVolume double
			 * @param  meanAnnualTemperature double
			 * @return void
			 * ******************************/
			void MossDecayModule::updateMossAppliedDecayParameters(double standMaximumVolume, double meanAnnualTemperature) {
				kss = F6(m, n, standMaximumVolume);

				akff = F7(kff, meanAnnualTemperature, q10); //applied feather moss fast pool applied decay rate  
				akfs = F7(kfs, meanAnnualTemperature, q10); //applied feather moss slow pool applied decay rate  
				aksf = F7(ksf, meanAnnualTemperature, q10); //applied sphagnum fast pool applied decay rate      
				akss = F7(kss, meanAnnualTemperature, q10); //applied sphagnum slow pool applied decay rate  		
			}
		}
	}
}
