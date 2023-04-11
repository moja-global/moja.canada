#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/timeseries.h"
#include "moja/modules/cbm/peatlands.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ilandunitcontroller.h>

#include <moja/exception.h>
#include <moja/signals.h>
#include <moja/logging.h>

#include <boost/algorithm/string.hpp> 
#include <boost/exception/all.hpp>
#include <boost/format.hpp>

#include <algorithm>
using namespace moja::flint;

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			* Initialise constant variable spinup as variable "spinup_parameters" in landUnitData. \n
			* If spinup is empty, it will return false. \n
			* Assign CBMSpinupSequencer._ageReturnInterval as "return_interval" in spinup, \n
			* CBMSpinupSequencer._maxRotationValue as "max_rotations" in spinup, \n
			* CBMSpinupSequencer._historicDistType as "historic_disturbance_type" in spinup, \n
			* CBMSpinupSequencer._lastPassDistType as "last_pass_disturbance_type" in spinup, \n
			* CBMSpinupSequencer._standDelay as "inventory_delay" in spinup, if empty assign as "delay" in spinup.
			*
			* If variable "growth_curve_id" in landUnitData is empty, assign CBMSpinupSequencer._spinupGrowthCurveID as -1, \n
			* else assign CBMSpinupSequencer._spinupGrowthCurveID as variable "growth_curve_id" in landUnitData. \n
			* If variable "minimum_rotation" in landUnitData is not empty, assign CBMSpinupSequencer._minimumRotation as variable "minimum_rotation" in landUnitData, \n
			* else return false. \n
			*
			* Assign CBMSpinupSequencer._age as variable "age" in landUnitData, \n
			* CBMSpinupSequencer._mat as "mean_annual_temperature" in landUnitData, \n
			* CBMSpinupSequencer._spu as "spatial_unit_id" in landUnitData, \n
			* CBMSpinupSequencer._isDecaying as "is_decaying" in landUnitData and \n
			* CBMSpinupSequencer._spinupMossOnly as "spinup_moss_only" in landUnitData.
			* if _landUnitData has variable "enable_peatland" and variable "enable_peatland" value, \n
			* Assign CBMSpinupSequencer._shrubAge as variable "peatland_shrub_age" in landUnitData and \n
			* CBMSpinupSequencer._smallTreeAge as variable "peatland_smalltree_age" in landUnitData. \n
			*
			* if variable "initial_age" in landUnitData is not empty, \n
			* assign CBMSpinupSequencer._standAge as variable "initial_age" in landUnitData. \n
			* else assign CBMSpinupSequencer._standAge as 0 and \n
			* if _landUnitData does not have variable "enable_peatland" and variable "enable_peatland" value, \n
			* return false. \n
			*
			* Set "delay" in landUnitData as CBMSpinupSequencer._standDelay. \n
			* Assign CBMSpinupSequencer._aboveGroundSlowSoil as pool variable "AboveGroundSlowSoil" in landUnitData and \n
			* CBMSpinupSequencer._belowGroundSlowSoil as pool variable "BelowGroundSlowSoil" in landUnitData. \n
			* if landUnitData has variable "last_pass_disturbance_timeseries", assign CBMSpinupSequencer._lastPassDisturbanceTimeseries as variable "last_pass_disturbance_timeseries" in landUnitData. \n
			*
			* Initialise integer Variable order as 1. \n
			* if landUnitData has variable "user_disturbance_order", \n
			* for every orderedDistType in variable "user_disturbance_order" in landUnitData, \n
			* iterate order by 1 and store the value in orderedDistType of CBMSpinupSequencer._disturbanceOrder. \n

			* if landUnitData has variable "default_disturbance_order",
			* for every orderedDistType in "default_disturbance_order" in landUnitData,
			* if orderedDistType of CBMSpinupSequencer._disturbanceOrder is equal to the last element of CBMSpinupSequencer._disturbanceOrder, \n
			* iterate order by 1 and store the value in orderedDistType of CBMSpinupSequencer._disturbanceOrder.

			* return true.
			*
			* @param landUnitData flint::ILandUnitDataWrapper&
			* @return bool
			* *****************/
			bool CBMSpinupSequencer::getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData) {
				const auto& spinup = landUnitData.getVariable("spinup_parameters")->value();
				if (spinup.isEmpty()) {
					return false;
				}

				const auto& spinupParams = spinup.extract<DynamicObject>();
				_ageReturnInterval = spinupParams[CBMSpinupSequencer::returnInverval];
				_maxRotationValue = spinupParams[CBMSpinupSequencer::maxRotation];
				_historicDistType = spinupParams[CBMSpinupSequencer::historicDistType].convert<std::string>();
				_lastPassDistType = spinupParams[CBMSpinupSequencer::lastDistType].convert<std::string>();

				auto delayParamName = spinupParams.contains(CBMSpinupSequencer::inventoryDelay)
					? CBMSpinupSequencer::inventoryDelay
					: CBMSpinupSequencer::delay;

				_standDelay = spinupParams[delayParamName].isEmpty() ? 0 : spinupParams[delayParamName].convert<int>();

				const auto& gcId = landUnitData.getVariable("growth_curve_id")->value();
				if (gcId.isEmpty()) {
					_spinupGrowthCurveID = -1;
				}
				else {
					_spinupGrowthCurveID = gcId;
				}

				const auto& minRotation = landUnitData.getVariable("minimum_rotation")->value();
				if (minRotation.isEmpty()) {
					return false;
				}
				_minimumRotation = minRotation;

				_age = landUnitData.getVariable("age");
				_mat = landUnitData.getVariable("mean_annual_temperature");
				_spu = landUnitData.getVariable("spatial_unit_id");
				_isDecaying = landUnitData.getVariable("is_decaying");
				_spinupMossOnly = landUnitData.getVariable("spinup_moss_only");
				_regenDelay = landUnitData.getVariable("regen_delay");

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {
					_shrubAge = landUnitData.getVariable("peatland_shrub_age");
					_smallTreeAge = landUnitData.getVariable("peatland_smalltree_age");
				}

				// Get the stand age of this land unit.
				const auto& initialAge = landUnitData.getVariable("initial_age")->value();
				if (initialAge.isEmpty()) {
					_standAge = 0;
					if (!(_landUnitData->hasVariable("enable_peatland") &&
						_landUnitData->getVariable("enable_peatland")->value())) {

						return false;
					}
				}
				else {
					int age = initialAge;
					_standAge = age < 0 ? 0 : age;
					_standRegenDelay = age < 0 ? age * -1 : 0;
				}

				// Set and pass the delay information.
				_delay = landUnitData.getVariable("delay");
				_delay->set_value(_standDelay);

				_aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
				_belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");

				if (landUnitData.hasVariable("last_pass_disturbance_timeseries")) {
					_lastPassDisturbanceTimeseries = landUnitData.getVariable("last_pass_disturbance_timeseries");
				}

				// Disturbance ordering for last pass timeseries.
				int order = 1;
				if (landUnitData.hasVariable("user_disturbance_order")) {
					const auto& userOrder = landUnitData.getVariable("user_disturbance_order")->value().extract<std::vector<DynamicVar>>();
					for (const auto& orderedDistType : userOrder) {
						_disturbanceOrder[orderedDistType] = order++;
					}
				}

				if (landUnitData.hasVariable("default_disturbance_order")) {
					const auto& defaultOrder = landUnitData.getVariable("default_disturbance_order")->value().extract<std::vector<DynamicVar>>();
					for (const auto& orderedDistType : defaultOrder) {
						if (_disturbanceOrder.find(orderedDistType) == _disturbanceOrder.end()) {
							_disturbanceOrder[orderedDistType] = order++;
						}
					}
				}

				return true;
			}

			/**
			* If the spinup parameters for _landUnitData is false return false. \n
			* Get variable "run_delay" in _landUnitData and assing the value as "false" \n
			* Check if to run peatland module and assign the value to a boolean Variable runPeatland. \n
			* Check if to run moss module and assign the value to a boolean Variable runMoss. \n
			* If runMoss is true, assign CBMSpinupSequencer._featherMossSlow as pool "FeatherMossSlow" in _landUnitData and \n
			* CBMSpinupSequencer._sphagnumMossSlow as pool "SphagnumMossSlow" in _landUnitData. \n
			* Get the timing in _landUnitData and assign the value to constant variable timing \n
			* Set timing stepping to TimeStepping::Annual. \n
			* If CBMSpinupSequencer._rampStartDate is not null, \n
			* Set Start date as CBMSpinupSequencer._rampStartDate,
			* End date as start date in parameter luc,
			* Start step date, end step date,cure start date and cur end date as timing start date in timing. \n
			* Post notification of signals TimingInit and TimingPostInit. \n
			* Set CBMSpinupSequencer._isDecaying as true. \n
			* if runPeatland is true, Invoke runPeatlandSpinup() using notificationCenter and luc as parameters. \n
			* else if variables "softwood_yield_table" and "hardwood_yield_table" in _landUnitData are empty, \n
			* set CBMSpinupSequencer._age as 0,
			* else invoke runRegularSpinup() using notificationCenter, luc and runMoss as parameters. \n
			* If any pool has a user-provided value, override the spinup value with it. \n
			* return true.
			*
			* @param notificationCenter NotificationCenter&
			* @param luc ILandUnitController&
			* @exception std::exception: Handles error
			* @exception VariableNotFoundException: Handles error when variable is not found
			* @exception VariableEmptyWhenValueExpectedException: Handles error when variable is empty
			* @exception SimulationError: Handles error during the simulation
			* @return bool
			* *****************/
			bool CBMSpinupSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
				// Get spinup parameters for this land unit.
				try {
					if (!getSpinupParameters(*_landUnitData)) {
						return false;
					}
				}
				catch (const VariableNotFoundException & e) {
					MOJA_LOG_FATAL << "Variable not found: " << *boost::get_error_info<VariableName>(e);
					MOJA_LOG_DEBUG << boost::diagnostic_information(e, true);
					throw;
				}
				catch (const boost::exception & e) {
					MOJA_LOG_FATAL << boost::diagnostic_information(e, true);
					throw;
				}
				catch (const Exception & e) {
					MOJA_LOG_FATAL << boost::diagnostic_information(e, true);
					throw;
				}
				catch (const std::exception & e) {
					MOJA_LOG_FATAL << e.what();
					throw;
				}

				try {
					_landUnitData->getVariable("run_delay")->set_value("false");
					_landUnitData->getVariable("regen_delay")->set_value(0);

					// Check and set run peatland flag.
					bool runPeatland = isPeatlandApplicable();

					// Check and set run moss flag.
					bool runMoss = isMossApplicable(runPeatland);

					if (runMoss) {
						_featherMossSlow = _landUnitData->getPool("FeatherMossSlow");
						_sphagnumMossSlow = _landUnitData->getPool("SphagnumMossSlow");
					}

					const auto timing = _landUnitData->timing();
					timing->setStepping(TimeStepping::Annual);
					if (!_rampStartDate.isNull()) {
						timing->setStartDate(_rampStartDate);
						timing->setEndDate(DateTime(luc.timing().startDate()));
						timing->setStartStepDate(timing->startDate());
						timing->setEndStepDate(timing->startDate());
						timing->setCurStartDate(timing->startDate());
						timing->setCurEndDate(timing->startDate());
					}

					notificationCenter.postNotification(moja::signals::TimingInit);
					notificationCenter.postNotification(moja::signals::TimingPostInit);

					_isDecaying->set_value(true);

					if (runPeatland) {
						runPeatlandSpinup(notificationCenter, luc);
					}
					else {
						// Skip spinup for pixels which have a non-forest (no increments) growth curve.
						const auto& swTable = _landUnitData->getVariable("softwood_yield_table")->value();
						const auto& hwTable = _landUnitData->getVariable("hardwood_yield_table")->value();
						if (swTable.isEmpty() && hwTable.isEmpty()) {
							_age->set_value(0);
						}
						else {
							runRegularSpinup(notificationCenter, luc, runMoss);
						}
					}

					// If any pool has a user-provided value, override the spinup value with it.
					for (const auto pool : _landUnitData->poolCollection()) {
						if (pool->initValue() > 0.0) {
							pool->init();
						}
					}

					_landUnitData->getVariable("regen_delay")->set_value(_standRegenDelay);

					return true;
				}
				catch (SimulationError & e) {
					std::string details = *(boost::get_error_info<Details>(e));
					std::string libraryName = *(boost::get_error_info<LibraryName>(e));
					std::string moduleName = *(boost::get_error_info<ModuleName>(e));
					std::string str = ((boost::format("%1%/%2%: %3%") % libraryName % moduleName % details).str());
					MOJA_LOG_FATAL << str;
					throw;
				}
				catch (const VariableEmptyWhenValueExpectedException & e) {
					MOJA_LOG_FATAL << "Variable empty when building query: " << *boost::get_error_info<VariableName>(e);
					MOJA_LOG_DEBUG << boost::diagnostic_information(e, true);
					throw;
				}
				catch (const VariableNotFoundException & e) {
					MOJA_LOG_FATAL << "Variable not found: " << *boost::get_error_info<VariableName>(e);
					MOJA_LOG_DEBUG << boost::diagnostic_information(e, true);
					throw;
				}
				catch (const std::exception & e) {
					MOJA_LOG_FATAL << e.what();
					BOOST_THROW_EXCEPTION(SimulationError()
						<< Details(e.what())
						<< LibraryName("moja.modules.cbm")
						<< ModuleName("unknown")
						<< ErrorCode(0));
				}
				catch (...) {
					MOJA_LOG_FATAL << "Unknown error during spinup";
					BOOST_THROW_EXCEPTION(SimulationError()
						<< Details("Unknown error during spinup")
						<< LibraryName("moja.modules.cbm")
						<< ModuleName("unknown")
						<< ErrorCode(0));
				}
			}

			/**
			 * Run Peatland Spinup
			 *
			 * Create a variable poolCached, set it to False
			 *
			 * Assign the value of variable "default_mean_annual_temperature" in _landUnitData to variable meanAnnualTemperature if CBMSpinupSequencer._mat is empty \n
			 *
			 * Set variable lastFireYearValue to the value of variable "default_last_fire_year" in _landUnitData if
			 * variable "last_fire_year" in _landUnitData is empty, \n
			 *
			 * Set variable maxReturnInterval to the maximum of value of variable "maximum_fire_return_interval" in _landUnitData and
			 * variable "default_fire_return_interval" in _landUnitData, if variable "fire_return_interval" in _landUnitData is empty,
			 * else set maxReturnInterval to the maximum of value of variable "maximum_fire_return_interval" in _landUnitData and
			 * variable "fire_return_interval" in _landUnitData
			 *
			 * If the cache object consisting of { CBMSpinupSequencer._spu, CBMSpinupSequencer._historicDistType, peatlandId, variable fireReturnIntervalValue and variable meanAnnualTemperature },
			 * is present in CBMSpinupSequencer._cache, set value of variable "peat_pool_cached" in _landUnitData to true and set poolCached to true \n
			 *
			 * Reset the ages CBMSpinupSequencer._shrubAge, CBMSpinupSequencer._smallTreeAge, CBMSpinupSequencer._age to zero before the spinup procedure
			 *
			 * If the variable poolCached is false, in production/removal mode, only run one rotation is performed and live biomass value at minimum spinup time steps is 200.
			 * set the value of variable "peatland_spinup_rotation" in _landUnitData to 0 for spinup output to record the rotation  \n
			 * Invoke CBMSpinupSequencer.fireSpinupSequenceEvent(), set 200 years for spinupnext to use the removal values at this age \n
			 * Post a special pre-disturbance signal to trigger peatland spinup next call \n
			 * one rotation of spinup is done, invoke CBMSpinupSequencer.fireSpinupSequenceEvent() to simulate the historic fire disturbance.
			 * Reset the ages CBMSpinupSequencer._shrubAge, CBMSpinupSequencer._smallTreeAge, CBMSpinupSequencer._age to zero
			 *
			 * If value of variable "peatland_fire_regrow" in _landUnitData is true, regrow to minimum peatland woody age.
			 * If CBMSpinupSequencer._standAge > 0, for forest peatland, just regrow to initial stand age  and invoke CBMSpinupSequencer.fireSpinupSequenceEvent()
			 *
			 * @param notificationCenter NotificationCenter&
			 * @param luc ILandUnitController&
			 * @return void
			 */
			void CBMSpinupSequencer::runPeatlandSpinup(NotificationCenter& notificationCenter, ILandUnitController& luc) {
				bool poolCached = false;
				const auto timing = _landUnitData->timing();
				int curStartYear = _landUnitData->timing()->curStartDate().year();
				double defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();

				auto mat = _mat->value();
				double meanAnnualTemperature = mat.isEmpty() ? defaultMAT
					: mat.type() == typeid(TimeSeries) ? mat.extract<TimeSeries>().value()
					: mat.convert<double>();

				auto& defalutLFY = _landUnitData->getVariable("default_last_fire_year")->value();
				auto& lastFireYear = _landUnitData->getVariable("last_fire_year")->value();
				int lastFireYearValue = lastFireYear.isEmpty() ? defalutLFY : lastFireYear;

				auto& defaultFRI = _landUnitData->getVariable("default_fire_return_interval")->value();
				auto& fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
				auto& maxReturnInterval = _landUnitData->getVariable("maximum_fire_return_interval")->value();
				int fireReturnIntervalValue = fireReturnInterval.isEmpty() ? defaultFRI : fireReturnInterval;
				if (fireReturnIntervalValue > maxReturnInterval) {
					fireReturnIntervalValue = maxReturnInterval;
				}

				auto& minimumPeatlandSpinupYears = _landUnitData->getVariable("minimum_peatland_spinup_years")->value();
				int minimumPeatlandSpinupYearsValue = minimumPeatlandSpinupYears.isEmpty() ? 100 : minimumPeatlandSpinupYears.convert<int>();

				auto peatlandFireRegrow = _landUnitData->getVariable("peatland_fire_regrow")->value();
				bool peatlandFireRegrowValue = peatlandFireRegrow.isEmpty() ? false : peatlandFireRegrow.convert<bool>();

				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				CacheKey cacheKey{
					_spu->value().convert<int>(),
					_historicDistType,
					peatlandId,
					fireReturnIntervalValue,
					meanAnnualTemperature
				};

				auto it = _cache.find(cacheKey);
				if (it != _cache.end()) {
					auto cachedResult = (*it).second;
					auto pools = _landUnitData->poolCollection();
					for (auto& pool : pools) {
						pool->set_value(cachedResult[pool->idx()]);
					}
					poolCached = true;
					_landUnitData->getVariable("peat_pool_cached")->set_value(poolCached);
				}

				int currentRotation = 0;
				int peatlandMaxRotationValue = minimumPeatlandSpinupYearsValue / fireReturnIntervalValue;
				peatlandMaxRotationValue = peatlandMaxRotationValue > 0 ? peatlandMaxRotationValue : 1;
				int peatlandSpinupStepsPerRotation = minimumPeatlandSpinupYearsValue > fireReturnIntervalValue ? fireReturnIntervalValue : minimumPeatlandSpinupYearsValue;

				// Reset the ages to ZERO before the spinup procedure
				_shrubAge->reset_value();
				_smallTreeAge->reset_value();
				_age->reset_value();

				// in production/removal mode, only run one rotation, 
				// and use live biomass value at minimum spinup time steps(200)
				peatlandMaxRotationValue = 1;
				while (!poolCached && currentRotation++ < peatlandMaxRotationValue) {
					//to record peatland spinup output
					_landUnitData->getVariable("peat_pool_cached")->set_value(false);

					//for spinup output to record the rotation 
					_landUnitData->getVariable("peatland_spinup_rotation")->set_value(currentRotation);

					//set 200 years for spinupnext to use the removal values at this age
					fireSpinupSequenceEvent(notificationCenter, luc, minimumPeatlandSpinupYearsValue, false);

					//post a special pre-disturbance signal to trigger peatland spinup next call
					notificationCenter.postNotification(moja::signals::PrePostDisturbanceEvent);

					//one rotation of spinup is done, simulate the historic fire disturbance.
					fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);

					//reset all ages to ZERO after fire event
					_shrubAge->reset_value();
					_smallTreeAge->reset_value();
					_age->reset_value();
				}

				int startYear = timing->startDate().year(); // Simulation start year.
				int minimumPeatlandWoodyAge = fireReturnIntervalValue; // Set the default regrow year.

				if (lastFireYearValue < 0) { // No last fire year record.
					minimumPeatlandWoodyAge = fireReturnIntervalValue;
				}
				else if (startYear - lastFireYearValue < 0) { // Fire occurred after simulation.
					minimumPeatlandWoodyAge = startYear - lastFireYearValue + fireReturnIntervalValue;
				}
				else { // Fire occurred before simulation.
					minimumPeatlandWoodyAge = startYear - lastFireYearValue;
				}

				if (!poolCached) {
					std::vector<double> cacheValue;
					auto pools = _landUnitData->poolCollection();
					for (auto& pool : pools) {
						cacheValue.push_back(pool->value());
					}
					_cache[cacheKey] = cacheValue;
				}

				// Regrow to minimum peatland woody age.
				if (peatlandFireRegrowValue) {
					if (_standAge > 0) {
						//for forest peatland, just regrow to initial stand age
						minimumPeatlandWoodyAge = _standAge;
					}
					fireSpinupSequenceEvent(notificationCenter, luc, minimumPeatlandWoodyAge, false);
				}
			}


			/**
			 * Perform Regular Spinup
			 *
			 *
			 * @param notificationCenter NotificationCenter&
			 * @param luc ILandUnitController&
			 * @param runMoss int
			 * @return void
			 */
			void CBMSpinupSequencer::runRegularSpinup(NotificationCenter& notificationCenter, ILandUnitController& luc, bool runMoss) {
				bool poolCached = false;
				_age->set_value(0);
				const auto timing = _landUnitData->timing();
				auto mat = _mat->value();
				auto meanAnnualTemperature = mat.isEmpty() ? 0
					: mat.type() == typeid(TimeSeries) ? mat.extract<TimeSeries>().value()
					: mat.convert<double>();

				CacheKey cacheKey{
					_spu->value().convert<int>(),
					_historicDistType,
					_spinupGrowthCurveID,
					_ageReturnInterval,
					meanAnnualTemperature
				};

				auto it = _cache.find(cacheKey);
				if (it != _cache.end()) {
					auto cachedResult = (*it).second;
					auto pools = _landUnitData->poolCollection();
					for (auto& pool : pools) {
						pool->set_value(cachedResult[pool->idx()]);
					}

					poolCached = true;
				}

				bool mossSlowPoolStable = false;
				double lastSlowPoolValue = 0;
				double lastMossSlowPoolValue = 0;

				// Loop up to the maximum number of rotations/passes.
				int currentRotation = 0;
				while (!poolCached && ++currentRotation <= _maxRotationValue) {
					// Fire spinup pass, each pass is up to the stand age return interval.
					// Reset forest stand and peatland age anyway for each pass.
					_age->set_value(0);
					fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, false);

					// Get the slow pool values at the end of age interval.			
					double currentSlowPoolValue = _aboveGroundSlowSoil->value() + _belowGroundSlowSoil->value();

					// Check if the slow pool is stable.
					bool slowPoolStable = isSlowPoolStable(lastSlowPoolValue, currentSlowPoolValue);
					if (runMoss) {
						double currentMossSlowPoolValue = _featherMossSlow->value() + _sphagnumMossSlow->value();
						mossSlowPoolStable = isSlowPoolStable(lastMossSlowPoolValue, currentMossSlowPoolValue);
						lastMossSlowPoolValue = currentMossSlowPoolValue;
					}

					// Update previous total slow pool value.
					lastSlowPoolValue = currentSlowPoolValue;
					if (slowPoolStable && currentRotation > _minimumRotation) {
						// Slow pool is stable, and the minimum rotations are done.
						break;
					}

					if (currentRotation == _maxRotationValue) {
						if (!slowPoolStable) {
							MOJA_LOG_INFO << "Slow pool is not stable at maximum rotation: " << currentRotation;
						}
						// Whenever the max rotations are reached, stop even if the slow pool is not stable.
						break;
					}

					// CBM spinup is not done, notify to simulate the historic disturbance.
					fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);

					// Growth curves assume a starting condition of zero biomass. If we use the post-disturbance
					// starting condition, biomass values could potentially be greater than zero and our
					// biomass/age class curves are shifted to the left.
					auto pools = _landUnitData->poolCollection();
					for (auto& pool : pools) {
						if (_biomassPools.find(pool->name()) != _biomassPools.end()) {
							pool->set_value(0);
						}
					}
				}

				while (!poolCached && runMoss && !mossSlowPoolStable) {
					// Do moss spinup only.
					_spinupMossOnly->set_value(true);

					_age->set_value(0);
					fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, false);

					double currentMossSlowPoolValue = _featherMossSlow->value() + _sphagnumMossSlow->value();
					mossSlowPoolStable = isSlowPoolStable(lastMossSlowPoolValue, currentMossSlowPoolValue);
					lastMossSlowPoolValue = currentMossSlowPoolValue;

					if (mossSlowPoolStable) {
						// Now moss slow pool is stable, turn off the moss spinup flag.
						_spinupMossOnly->set_value(false);
						break;
					}

					// Moss spinup is not done, notify to simulate the historic disturbance - wild fire.
					fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);
				}

				// Perform the optional ramp-up from spinup to regular simulation values: user specifies
				// ramp start year and provides one or more timeseries spinup variables; these use the
				// first value in the timeseries for regular spinup rotations, then the ramp advances them
				// through to the end of the timeseries.

				// Calculate the number of years in the ramp period.
				int simStartYear = luc.timing().startDate().year();
				int rampLength = _rampStartDate.isNull() ? 0 : simStartYear - _rampStartDate.value().year();

				// Last pass disturbance can potentially be specified as a timeseries.
				std::map<int, std::vector<std::string>> lastPassDisturbanceTimeseries;
				if (_lastPassDisturbanceTimeseries != nullptr) {
					const auto& lastPassTimeseries = _lastPassDisturbanceTimeseries->value();
					if (!lastPassTimeseries.isEmpty()) {
						for (const auto& event : lastPassTimeseries.extract<const std::vector<DynamicObject>>()) {
							int year = event["year"];
							std::string disturbanceType = event["disturbance_type"].extract<std::string>();
							lastPassDisturbanceTimeseries[year].push_back(disturbanceType);
						}
					}

					for (auto& eventYear : lastPassDisturbanceTimeseries) {
						std::stable_sort(
							eventYear.second.begin(), eventYear.second.end(),
							[this](const std::string& first, const std::string& second
								) {
									if (_disturbanceOrder.find(first) == _disturbanceOrder.end()) {
										return false;
									}

									return _disturbanceOrder[first] < _disturbanceOrder[second];
							});
					}
				}

				// If the provided last pass timeseries ends too soon and would result in a stand age
				// greater than the inventory specifies, add an extra event onto the end so that the stand
				// grows to the correct age. If the timeseries ends too late and would result in a stand
				// age less than the inventory specifies, ignore the inventory age.
				if (!lastPassDisturbanceTimeseries.empty()) {
					int lastPassTimeseriesEndYear = lastPassDisturbanceTimeseries.rbegin()->first;
					if (lastPassTimeseriesEndYear > simStartYear) {
						MOJA_LOG_FATAL << "Last pass disturbance timeseries cannot end after simulation start year.";
					}

					int ageFromTimeseries = simStartYear - lastPassTimeseriesEndYear;
					if (ageFromTimeseries < _standAge + _standDelay) {
						_standAge = _standAge == 0 ? 0 : ageFromTimeseries;
						_standDelay = _standDelay == 0 ? 0 : ageFromTimeseries;
					}
				}

				int finalLastPassYear = simStartYear - _standAge - _standDelay - (_standDelay > 0 ? 1 : 0);
				if (lastPassDisturbanceTimeseries.find(finalLastPassYear) == lastPassDisturbanceTimeseries.end()) {
					lastPassDisturbanceTimeseries[finalLastPassYear].push_back(_lastPassDistType);
				}

				int lastPassTimeseriesLength = lastPassDisturbanceTimeseries.rbegin()->first
					- lastPassDisturbanceTimeseries.begin()->first;

				// Calculate the total number of extra years needed in order to simulate the entire
				// ramp, divided into extra whole rotations, with the remainder of years added on to
				// the final rotation.
				int extraYears = std::max(0, rampLength - lastPassTimeseriesLength - _standAge - _standDelay);
				int extraRotations = extraYears / _ageReturnInterval;
				int finalRotationLength = extraYears % _ageReturnInterval;

				// Determine the number of years in the last pass disturbance timeseries that occur before
				// and after the start of the ramp period.
				int preRampLastPassYears = std::max(0, _standAge + _standDelay + lastPassTimeseriesLength - std::max(_standAge + _standDelay, rampLength));
				int rampLastPassYears = std::max(0, lastPassTimeseriesLength - preRampLastPassYears);

				// Determine the number of years between the final last pass disturbance in the timeseries
				// and the start of the ramp period: these timesteps advance the stand toward its final age,
				// but use regular (pre-ramp) spinup values.
				int preRampAgeGrowthYears = std::max(0, _standAge - rampLength);

				// Calculate the number of years of growth within the ramp period after the final last pass
				// disturbance: these timesteps advance the stand to its final age.
				int rampAgeGrowthYears = _standAge - preRampAgeGrowthYears;

				// Determine the number of pre- and post-ramp stand delay years to simulate, i.e. for stands with
				// a delay value instead of an age when their last pass disturbance is deforestation.
				int preRampDelayYears = std::max(0, _standDelay - rampLength);
				int rampDelayYears = _standDelay - preRampDelayYears;

				for (int i = 0; i < extraRotations; i++) {
					_age->set_value(0);
					fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, true);
					fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);
				}

				fireSpinupSequenceEvent(notificationCenter, luc, finalRotationLength, true);

				if (!poolCached) {
					std::vector<double> cacheValue;
					auto pools = _landUnitData->poolCollection();
					for (auto& pool : pools) {
						cacheValue.push_back(pool->value());
					}

					_cache[cacheKey] = cacheValue;
				}

				// Run the growth and disturbances in the last pass timeseries. The event at the beginning
				// fires immediately, otherwise there is a growth step before each disturbance.
				int lastPassYear = lastPassDisturbanceTimeseries.begin()->first;
				for (const auto& lastPassDisturbance : lastPassDisturbanceTimeseries[lastPassYear]) {
					fireHistoricalLastDisturbanceEvent(notificationCenter, luc, lastPassDisturbance);
				}

				for (int i = 0; i < lastPassTimeseriesLength; i++) {
					lastPassYear++;
					bool inRamp = i >= preRampLastPassYears;
					fireSpinupSequenceEvent(notificationCenter, luc, 1, inRamp);
					if (lastPassDisturbanceTimeseries.find(lastPassYear) != lastPassDisturbanceTimeseries.end()) {
						for (const auto& lastPassDisturbance : lastPassDisturbanceTimeseries[lastPassYear]) {
							fireHistoricalLastDisturbanceEvent(notificationCenter, luc, lastPassDisturbance);
						}
					}
				}

				// Fire up the spinup sequencer to grow the stand to the original stand age.
				_age->set_value(0);
				fireSpinupSequenceEvent(notificationCenter, luc, preRampAgeGrowthYears, false);
				fireSpinupSequenceEvent(notificationCenter, luc, rampAgeGrowthYears, true);

				if (_standDelay > 0) {
					// If there is stand delay due to deforestation disturbance,
					// fire up the stand delay to do turnover and decay only.
					_landUnitData->getVariable("run_delay")->set_value("true");
					fireSpinupSequenceEvent(notificationCenter, luc, preRampDelayYears, false);
					fireSpinupSequenceEvent(notificationCenter, luc, rampDelayYears, true);
					_landUnitData->getVariable("run_delay")->set_value("false");
				}
			}

			/**
			 * If parameter lastSlowPoolValue != 0, returns if the the ratio currentSlowPoolValue / lastSlowPoolValue
			 * is greater than 0.999 and less than 1.001, else returns True.
			 *
			 * @param lastSlowPoolValue double
			 * @param currentSlowPoolValue double
			 * @return bool
			 */
			bool CBMSpinupSequencer::isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue) {
				double changeRatio = 0;
				if (lastSlowPoolValue != 0) {
					changeRatio = currentSlowPoolValue / lastSlowPoolValue;
				}

				return changeRatio > 0.999 && changeRatio < 1.001;
			}

			/**
			 * For each step in the range 0 to parameter maximumSteps,
			 * if parameter incrementStep is true, increment the timing step, start step date, end step date,
			 * current start and end date by 1 (one year) \n
			 * Post notifications TimingStep, TimingPreEndStep, TimingEndStep and TimingPostStep \n
			 * Invoke applyOperations() to apply the operations in the current step and clearAllOperationResults() to clear the results
			 * on _landUnitData
			 *
			 * @param maximumSteps int
			 * @param incrementStep bool
			 * @param notificationCenter NotificationCenter&
			 * @param luc ILandUnitController&
			 * @return void
			 */
			void CBMSpinupSequencer::fireSpinupSequenceEvent(NotificationCenter& notificationCenter,
				flint::ILandUnitController& luc,
				int maximumSteps,
				bool incrementStep) {
				for (int i = 0; i < maximumSteps; i++) {
					if (incrementStep) {
						const auto timing = _landUnitData->timing();
						timing->setStep(timing->step() + 1);
						timing->setStartStepDate(timing->startStepDate().addYears(1));
						timing->setEndStepDate(timing->endStepDate().addYears(1));
						timing->setCurStartDate(timing->curStartDate().addYears(1));
						timing->setCurEndDate(timing->curEndDate().addYears(1));
					}

					notificationCenter.postNotificationWithPostNotification(moja::signals::TimingStep);
					notificationCenter.postNotificationWithPostNotification(moja::signals::TimingPreEndStep);
					notificationCenter.postNotification(moja::signals::TimingEndStep);
					notificationCenter.postNotification(moja::signals::TimingPostStep);
					_landUnitData->applyOperations();
					_landUnitData->clearAllOperationResults();
				}
			}

			/**
			 * Create a placeholder vector transfer to keep the event pool transfers and fire the disturbance with the transfers vector to be filled in by
			 *  any modules that build the disturbance matrix \n
			 * Post the notification DisturbanceEvent with the data about the disturbance and the transfers
			 *
			 * @param notificationCenter NotificationCenter&
			 * @param luc ILandUnitController&
			 * @param disturbance Disturbance&
			 * @return void
			 */
			void CBMSpinupSequencer::fireHistoricalLastDisturbanceEvent(NotificationCenter& notificationCenter,
				ILandUnitController& luc,
				std::string disturbanceName) {
				// Create a placeholder vector to keep the event pool transfers.
				auto transfer = std::make_shared<std::vector<CBMDistEventTransfer>>();

				// Fire the disturbance with the transfers vector to be filled in by
				// any modules that build the disturbance matrix.
				DynamicVar data = DynamicObject({
					{ "disturbance", disturbanceName },
					{ "transfers", transfer }
					});

				notificationCenter.postNotificationWithPostNotification(
					moja::signals::DisturbanceEvent, data);
			}

			/**
			 * Determine whether peatland has to be simulated
			 *
			 * If _landUnitData does not have the variables "peatland_class" and "enable_peatland", return false \n
			 * Else, if the value of variable "enable_peatland" in _landUnitData is not null,
			 * if value of variable "inventory_over_peatland" in _landUnitData > 0, value of variable
			 * "peatland" in _landUnitData > 0 and CBMSpinupSequencer._spinupGrowthCurveID > 0, determine if any species in
			 * variable "forest_peatland_leading_species" of _landUnitData is contained in variable "leading_species" of _landUnitData \n
			 * If peatlandId is not Peatlands::FOREST_PEATLAND_BOG, Peatlands::FOREST_PEATLAND_POORFEN, Peatlands::FOREST_PEATLAND_RICHFEN or
			 * Peatlands::FOREST_PEATLAND_SWAMP return true, indicating that the peatland is to be simulated \n
			 * In any other case, return false, indicating that the peatland is not to be simulated
			 *
			 * @return bool
			 */
			bool CBMSpinupSequencer::isPeatlandApplicable() {
				bool toSimulatePeatland = false;
				int peatlandId = -1;
				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					auto inventoryOverPeatland = _landUnitData->getVariable("inventory_over_peatland")->value();
					bool inventory_win = inventoryOverPeatland.convert<bool>();

					//rename peatland profile map as peatland
					//rename peatland_class profile map as peatland (variable)
					auto& peatland = _landUnitData->getVariable("peatland")->value();
					peatlandId = peatland.isEmpty() ? -1 : peatland.convert<int>();

					//check following for peatland pixel with valid forest growth curve
					if (inventory_win && peatlandId > 0 && _spinupGrowthCurveID > 0) {
						std::string speciesName = _landUnitData->getVariable("leading_species")->value();
						boost::algorithm::to_lower(speciesName);

						auto forestPeatlandLeadingSpecies = _landUnitData->getVariable("forest_peatland_leading_species")->value();

						bool hasForestPeatlandLeadingSpecies = false;

						for (std::string item : forestPeatlandLeadingSpecies) {
							boost::algorithm::to_lower(item);
							hasForestPeatlandLeadingSpecies = boost::contains(speciesName, item);

							if (hasForestPeatlandLeadingSpecies) {
								//one matched leading species found
								break;
							}
						}
						if (!hasForestPeatlandLeadingSpecies || //no matching leading species found					
							!((peatlandId == (int)Peatlands::FOREST_PEATLAND_BOG) ||
							(peatlandId == (int)Peatlands::FOREST_PEATLAND_POORFEN) ||
								(peatlandId == (int)Peatlands::FOREST_PEATLAND_RICHFEN) ||
								(peatlandId == (int)Peatlands::FOREST_PEATLAND_SWAMP))) {
							//with valid growth curve, matched leading species, but non-forest peatland

							peatlandId = -1; //reset peatland_id = -1 to skip running peatland module								
						}
					}

					toSimulatePeatland = peatlandId > 0;

					//set variable "peatland_class", peatland run is simulated only if peatland_class > 0
					_landUnitData->getVariable("peatland_class")->set_value(peatlandId);
				}

				return toSimulatePeatland;
			}


			/**
			 * Determine whether the moss needs to be simulated
			 *
			 * If _landUnitData does not have the variable "enable_moss", the moss will not be simulated and false is returned \n
			 * If _landUnitData has the variable "enable_moss" and the value is not null,
			 * if the value of variable "growth_curve_id" in _landUnitData is not empty,
			 * return true if parameter runPeatland is false and the value of variable "leading_species" in _landUnitData contains
			 * the value of variable "moss_leading_species" in _landUnitData. \n
			 * Else return false
			 *
			 * @param runPeatland bool
			 * @return bool
			 */
			bool CBMSpinupSequencer::isMossApplicable(bool runPeatland) {
				bool toSimulateMoss = false;

				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {

					// check this because moss growth is function of yield curve's merchantable volume.
					const auto& gcid = _landUnitData->getVariable("growth_curve_id")->value();
					bool isGrowthCurveDefined = !gcid.isEmpty() && gcid != -1;

					// moss growth is based on leading species' growth.
					if (isGrowthCurveDefined) {
						std::string mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
						std::string speciesName = _landUnitData->getVariable("leading_species")->value();

						// Can also get species from a spatial layer:
						// std::string speciesName2 = _landUnitData->getVariable("species")->value();
						boost::algorithm::to_lower(mossLeadingSpecies);
						boost::algorithm::to_lower(speciesName);

						toSimulateMoss = !runPeatland && boost::contains(speciesName, mossLeadingSpecies);
					}
				}

				return toSimulateMoss;
			}

		}
	}
} // namespace moja::modules::cbm
