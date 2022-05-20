/**
 * @file
 * The CBMDecayModule module models the decay of dead organic matter into soil carbon pools and CO2. 
 * Users have the option to define extra transfers that divert some 
 * of the soil carbon decaying to CO2, into other pools. Decay does not occur if a pixel has 
 * no matching growth curve, or if the pixel has a non-forest current land class (i.e., CL) at 
 * the start of the simulation and its last pass disturbance event was not deforestation.
 * 
 * ******************/

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
            * Configuration function
			* 
			* Assign CBMDecayModule._extraDecayRemovals the value of variable "extra_decay_removals" if 
            * it is contained in parameter config
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
	        * Subscribe to the signals LocalDomainInit, TimingInit, and TimingStep
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
			* Transfer between source and destination pools (Overloaded function)
			* 
			* Initialise the double variables decayRate and proptoatmosphere from CBMDecayModule._decayParameters \n
			* Invoke addTransfer() method on parameter operation, using arguments poolSrc, CBMDecayModule._atmosphere, poolDest \n
			* and variables decayRate and propToAtmosphere.
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
			* Transfer between source and destination pools (Overloaded Operation)
			*
			* Initialise the double variables decayRate and proptoatmosphere from CBMDecayModule._decayParameters \n
			* Get the additional removals from the amount decayed to the atmosphere and add transfer to the operation \n
			* using the parameter(pool),dstPool,decayRate and dstProps.
			* Add transfer to operation parameter using the parameters(pool,_atmopshere), decayRate and
			* propToAtmosphere.
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
			*
			* Initialise CBMDecayModule._aboveGroundVeryFastSoil, CBMDecayModule._belowGroundVeryFastSoil, \n
			* CBMDecayModule._aboveGroundFastSoil, CBMDecayModule._belowGroundFastSoil, CBMDecayModule._mediumSoil, CBMDecayModule._aboveGroundSlowSoil, \n
			* CBMDecayModule._belowGroundSlowSoil, CBMDecayModule._softwoodStemSnag, CBMDecayModule._softwoodBranchSnag, 
			* CBMDecayModule._hardwoodStemSnag, CBMDecayModule._hardwoodBranchSnag, CBMDecayModule._atmosphere, \n
			* CBMDecayModule._spinupMossOnly and CBMDecayModule._isDecaying from _landUnitData
			* Initialise constant variable decayParameterTable and add the values to CBMDecayModule._decayParameters
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
			*
			* Assign CBMDecayModule._slowMixingRate value of variable "slow_ag_to_bg_mixing_rate" in _landUnitData \n
			* If CBMDecayModule._extraDecayRemovals is true, assign the proportion of transfer between the source and \n
			* destination pools in CBMDecayModule._decayRemovals \n
			* Invoke CBMDecayModule.initPeatland()

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
			* Determine if the module should be run.
			*
			* When moss module is spinning up, nothing to grow, turnover and decay.
			* Return true if CBMDecayModule._spinupMossOnly is false and CBMDecayModule._isDecaying is true
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
			*
			* If CBMDecayModule.shouldRun() is false or CBMDecayModule._skipforPeatland is true, return \n
			* Initialise  proportional operation variables domDecay, soilDecay and soilTurnover. \n
			* Add domDecay transfer for CBMDecayModule._aboveGroundVeryFastSoil, CBMDecayModule._belowGroundVeryFastSoil, \n
			* CBMDecayModule._aboveGroundFastSoil, CBMDecayModule._belowGroundFastSoil, CBMDecayModule._mediumSoil, \n
			* CBMDecayModule._softwoodStemSnag, CBMDecayModule._softwoodBranchSnag, CBMDecayModule._hardwoodStemSnag, CBMDecayModule._hardwoodBranchSnag. \n
			* Add soilDecay transfer for CBMDecayModule._aboveGroundSlowSoil and CBMDecayModule._belowGroundSlowSoil. \n
			* Add soilTurnover transfer using CBMDecayModule._aboveGroundSlowSoil, CBMDecayModule._belowGroundSlowSoil and \n 
			* CBMDecayModule._slowMixingRate values.
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
			*
			* Reset CBMDecayModule._skipForPeatland as false \n
			* If the variable "enable_peatland" exists in_landUnitData and is not null, \n 
			* if the peatlandId corresponding to the value of the variable "peatland_class" is not equal to \n
			* Peatlands::OPEN_PEATLAND_BOG, Peatlands::OPEN_PEATLAND_POORFEN or Peatlands::OPEN_PEATLAND_RICHFEN, \n
			* value of CBMDecayModule._skipForPeatland is set to true else false,
			* i.e skip decay when running peatland on any open peatland
			* 
			* skip decay when running peatland on any open peatland
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
