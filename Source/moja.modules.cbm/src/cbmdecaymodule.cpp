/**
* @file
* @brief 
* Performs annual decay and turnover on a set of dead organic matter pools \n 
*  Data requirements: \n
* 1: table named "decay_parameters" with 1 set of decay \n
*    parameters for each of the enumerated dom pools in the DomPool enum \n
*    Columns: \n
*       SoilPoolId: the integer of the DomPool, which corresponds with the enumeration \n
*       OrganicMatterDecayRate: the base decay rate \n
*       Q10: the Q10 \n
*       Tref: the reference temperature (degrees Celcius) \n
*       Max: the maximum decay rate for the dom pool \n
* 2: scalar "mean_annual_temperature" the mean annual temperature of the environment \n
* 3: scalar "SlowMixingRate" the amount turned over from slow ag to slow bg annually \n
*
* ******/
#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/modules/cbm/printpools.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/flint/ivariable.h>



namespace moja {
	namespace modules {
		namespace cbm {

			/**
            * @brief configuration function.
            *
            * This function gets the value of the extra decay removals if it's exist
            *
            * @param config DynamicObject&
            * @return void
            * ************************/
			void CBMDecayModule::configure(const DynamicObject& config) {
				if (config.contains("extra_decay_removals")) {
					_extraDecayRemovals = config["extra_decay_removals"];
				}
			}

			/**
	        * @brief subscribe to signal.
	        *
	        * This function subscribes the signal localDomainInit, TimingInit, and TimingStep
	        * using the function onLocalDomainInit,onTimingInit ,and onTimingStep respectively.
	        * The values are passed and assigned here
	        *
	        * @param notificationCenter NotificationCenter&
	        * @return void
	        * ************************/

			void CBMDecayModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMDecayModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMDecayModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &CBMDecayModule::onTimingStep, *this);
			}


			/**
			* @brief getTransfer
			* 
			* This is an overloaded function that gets the value of the decay rate,
			* prop to atmosphere from the decay parameters variable and add transfer 
			* using the values passed here.
			* 
			* @param operation shared_ptr<Ioperation>
			* @param meanAnnualTemperature double
			* @param domPool string&
			* @param poolSrc IPool*
			* @param poolDest IPool*
			* @return void
			* ************************/

			void CBMDecayModule::getTransfer(std::shared_ptr<flint::IOperation> operation,
				double meanAnnualTemperature,
				const std::string& domPool,
				const flint::IPool* poolSrc,
				const flint::IPool* poolDest) {
				double decayRate = _decayParameters[domPool].getDecayRate(meanAnnualTemperature);
				double propToAtmosphere = _decayParameters[domPool].pAtm;
				operation->addTransfer(poolSrc, poolDest, decayRate * (1 - propToAtmosphere))
					->addTransfer(poolSrc, _atmosphere, decayRate * propToAtmosphere);
			}

			/**
			* @brief getTransfer
			*
			* This is an overloaded function that gets the value of the decay rate,
			* prop to atmosphere from the decay parameters variable and add transfer by removing the additional removals
			* from the amount decayed to the atmosphere.
			*
			* @param operation shared_ptr<Ioperation>
			* @param meanAnnualTemperature double
			* @param domPool string&
			* @param pool IPool*
			* @return void
			* ************************/

			void CBMDecayModule::getTransfer(std::shared_ptr<flint::IOperation> operation,
				double meanAnnualTemperature,
				const std::string& domPool,
				const flint::IPool* pool) {
				double decayRate = _decayParameters[domPool].getDecayRate(meanAnnualTemperature);
				double propToAtmosphere = _decayParameters[domPool].pAtm;

				// Decay a proportion of a pool to the atmosphere as well as any additional
				// removals (dissolved organic carbon, etc.) - additional removals are subtracted
				// from the amount decayed to the atmosphere.
				double propRemovals = 0.0;
				const auto removals = _decayRemovals.find(domPool);
				if (removals != _decayRemovals.end()) {
					for (const auto removal : (*removals).second) {
						const auto dstPool = _landUnitData->getPool(removal.first);
						const auto dstProp = removal.second;
						propRemovals += dstProp;
						operation->addTransfer(pool, dstPool, decayRate * dstProp);
					}
				}

				operation->addTransfer(pool, _atmosphere, decayRate * (propToAtmosphere - propRemovals));
			}

			/**
			* @brief Initiate Local domain
			*
			* This function gets the value of the decay paramters from the land unit data variable 
			* and add it to the decay parameter variable.
			* it also gets the pool value of the above ground very fast soil, below ground very fast soil,
			* above ground fast soil, below ground fast soil, medium soil,above ground slow soil,below ground slow soil,
			* softwood stem snag,softwood branch snag, hardwood stem snag, hardwood branch snag, co2, 
			* spinup moss only and is decaying from the land unit data.
			*
			* @return void
			* ************************/

			void CBMDecayModule::doLocalDomainInit() {
				_aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
				_belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
				_aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
				_belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");
				_mediumSoil = _landUnitData->getPool("MediumSoil");
				_aboveGroundSlowSoil = _landUnitData->getPool("AboveGroundSlowSoil");
				_belowGroundSlowSoil = _landUnitData->getPool("BelowGroundSlowSoil");
				_softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
				_softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
				_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
				_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
				_atmosphere = _landUnitData->getPool("CO2");

				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
				_isDecaying = _landUnitData->getVariable("is_decaying");

				const auto decayParameterTable = _landUnitData->getVariable("decay_parameters")->value()
					.extract<const std::vector<DynamicObject>>();

				_decayParameters.clear();
				for (const auto row : decayParameterTable) {
					_decayParameters.emplace(row["pool"].convert<std::string>(),
						PoolDecayParameters(row));
				}
			}

			/**
			* @brief initiate timing
			*
			* Detailed description here
			* 
			* @return void
			* ************************/

			void CBMDecayModule::doTimingInit() {
				_slowMixingRate = _landUnitData->getVariable("slow_ag_to_bg_mixing_rate")->value();

				if (_extraDecayRemovals) {
					const auto decayRemovalsTable = _landUnitData->getVariable("decay_removals")->value()
						.extract<const std::vector<DynamicObject>>();

					_decayRemovals.clear();
					for (const auto row : decayRemovalsTable) {
						_decayRemovals[row["from_pool"]][row["to_pool"]] = row["proportion"];
					}
				}

				initPeatland();
			}

			/**
			* @brief shouldRun
			*
			* Detailed description here
			* 
			* @return bool
			* ************************/

			bool CBMDecayModule::shouldRun() {
				// When moss module is spinning up, nothing to grow, turnover and decay.
				bool spinupMossOnly = _spinupMossOnly->value();
				bool isDecaying = _isDecaying->value();

				return !spinupMossOnly && isDecaying;
			}

			/**
			* @brief doTimingStep
			*
			* This function gets the time series from value of the mean annual temperature in the land unit data variable and
			* add transfer for above ground very fast soil,below ground very fast soil, above ground fast soil,below ground fast soil,
			* medium soil, softwood stem snag, softwood branch snag,hardwood stem snag ,hardwood branch snag, above ground slow soil, and
			* below ground slow soil.
			* 
			* @return void
			* ************************/

			void CBMDecayModule::doTimingStep() {
				if (!shouldRun() || _skipForPeatland) {
					return;
				}

				auto mat = _landUnitData->getVariable("mean_annual_temperature")->value();
				auto t = mat.isEmpty() ? 0
					: mat.type() == typeid(TimeSeries) ? mat.extract<TimeSeries>().value()
					: mat.convert<double>();

				auto domDecay = _landUnitData->createProportionalOperation();
				getTransfer(domDecay, t, "AboveGroundVeryFastSoil", _aboveGroundVeryFastSoil, _aboveGroundSlowSoil);
				getTransfer(domDecay, t, "BelowGroundVeryFastSoil", _belowGroundVeryFastSoil, _belowGroundSlowSoil);
				getTransfer(domDecay, t, "AboveGroundFastSoil", _aboveGroundFastSoil, _aboveGroundSlowSoil);
				getTransfer(domDecay, t, "BelowGroundFastSoil", _belowGroundFastSoil, _belowGroundSlowSoil);
				getTransfer(domDecay, t, "MediumSoil", _mediumSoil, _aboveGroundSlowSoil);
				getTransfer(domDecay, t, "SoftwoodStemSnag", _softwoodStemSnag, _aboveGroundSlowSoil);
				getTransfer(domDecay, t, "SoftwoodBranchSnag", _softwoodBranchSnag, _aboveGroundSlowSoil);
				getTransfer(domDecay, t, "HardwoodStemSnag", _hardwoodStemSnag, _aboveGroundSlowSoil);
				getTransfer(domDecay, t, "HardwoodBranchSnag", _hardwoodBranchSnag, _aboveGroundSlowSoil);
				_landUnitData->submitOperation(domDecay);
				_landUnitData->applyOperations();

				auto soilDecay = _landUnitData->createProportionalOperation();
				getTransfer(soilDecay, t, "AboveGroundSlowSoil", _aboveGroundSlowSoil);
				getTransfer(soilDecay, t, "BelowGroundSlowSoil", _belowGroundSlowSoil);
				_landUnitData->submitOperation(soilDecay);
				_landUnitData->applyOperations();

				auto soilTurnover = _landUnitData->createProportionalOperation();
				soilTurnover->addTransfer(_aboveGroundSlowSoil, _belowGroundSlowSoil, _slowMixingRate);
				_landUnitData->submitOperation(soilTurnover);
				_landUnitData->applyOperations();
			}

			/**
			* @brief initiate peat land
			*
			* This function always reset the skip for peatland variable to false and
			* it skips decay when peatland on any open peatland.
			* 
			* @return void
			* ************************/

			void CBMDecayModule::initPeatland() {
				//always reset to false
				_skipForPeatland = false;

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					bool isOpenPeatland = (
						peatlandId == (int)Peatlands::OPEN_PEATLAND_BOG ||
						peatlandId == (int)Peatlands::OPEN_PEATLAND_POORFEN ||
						peatlandId == (int)Peatlands::OPEN_PEATLAND_RICHFEN);

					//skip decay when running peatland on any open peatland
					_skipForPeatland = isOpenPeatland;
		}
	}
}}} // namespace moja::modules::cbm
