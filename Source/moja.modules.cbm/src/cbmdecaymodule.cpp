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

			void CBMDecayModule::configure(const DynamicObject& config) {
				if (config.contains("extra_decay_removals")) {
					_extraDecayRemovals = config["extra_decay_removals"];
				}
			}

			void CBMDecayModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMDecayModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMDecayModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &CBMDecayModule::onTimingStep, *this);
			}

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

			bool CBMDecayModule::shouldRun() {
				// When moss module is spinning up, nothing to grow, turnover and decay.
				bool spinupMossOnly = _spinupMossOnly->value();
				bool isDecaying = _isDecaying->value();

				return !spinupMossOnly && isDecaying;
			}

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
