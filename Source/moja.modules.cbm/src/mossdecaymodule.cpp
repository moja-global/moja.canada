#include "moja/modules/cbm/mossdecaymodule.h"
#include "moja/modules/cbm/helper.h"

#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
	namespace modules {
		namespace cbm {

			void MossDecayModule::configure(const DynamicObject& config) { }

			void MossDecayModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossDecayModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossDecayModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &MossDecayModule::onTimingStep, *this);
			}

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

			void MossDecayModule::doTimingInit() {
				auto& defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();
				auto& matVal = _landUnitData->getVariable("mean_annual_temperature")->value();

				meanAnnualTemperature = matVal.isEmpty() ? defaultMAT : matVal;

				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
				bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

				auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
				auto speciesName = _landUnitData->getVariable("leading_species")->value();
							   
				runMoss = peatlandId < 0 
					&& isGrowthCurveDefined
					&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
			};

			void MossDecayModule::doTimingStep() {
				if (runMoss) {
					currentStandGCId = _landUnitData->getVariable("growth_curve_id")->value();

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
			void MossDecayModule::doMossSlowPoolDecay() {
				auto mossSlowDecay = _landUnitData->createProportionalOperation();

				mossSlowDecay->addTransfer(_featherMossSlow, _CO2, akfs);
				mossSlowDecay->addTransfer(_sphagnumMossSlow, _CO2, akss);

				_landUnitData->submitOperation(mossSlowDecay);
			}

			//Sphagnum slow pool base decay rate, kss = m*ln(maxVolume) + n
			double MossDecayModule::F6(double m, double n, double maxVolume) {
				double value = m * log(maxVolume) + n;
				return value;
			}

			//Applied decay rate to all moss pools: kff, kfs, ksf, kss (kff*(e^((MAT-10)*(ln(Q10)*0.1))
			//kff - feather fast decay rate 
			//kfs - feather slow decay rate
			//ksf - sphagnum fast decay rate
			//kss - sphagnum slow decay rate
			double MossDecayModule::F7(double baseDecayRate, double meanAnnualTemperature, double q10) {
				double expValue = exp((meanAnnualTemperature - tref) * log(q10) * 0.1);
				double retValue = baseDecayRate * expValue;

				return retValue;
			}

			//update moss pool base decay rate based on mean annual temperature and q10 value
			void MossDecayModule::updateMossAppliedDecayParameters(double standMaximumVolume, double meanAnnualTemperature) {
				kss = F6(m, n, standMaximumVolume);

				akff = F7(kff, meanAnnualTemperature, q10); //applied feather moss fast pool applied decay rate  
				akfs = F7(kfs, meanAnnualTemperature, q10); //applied feather moss slow pool applied decay rate  
				aksf = F7(ksf, meanAnnualTemperature, q10); //applied sphagnum fast pool applied decay rate      
				akss = F7(kss, meanAnnualTemperature, q10); //applied sphagnum slow pool applied decay rate  		
	}
}}}
