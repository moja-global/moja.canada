#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ilandunitcontroller.h>

#include <moja/exception.h>
#include <moja/signals.h>
#include <moja/logging.h>

#include <boost/algorithm/string.hpp> 
#include <boost/exception/diagnostic_information.hpp>

#include <algorithm>

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {

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
		_standDelay = spinupParams[CBMSpinupSequencer::delay];

		const auto& gcId = landUnitData.getVariable("growth_curve_id")->value();
		if (gcId.isEmpty()) {
			return false;
		}
		_spinupGrowthCurveID = gcId;

		const auto& minRotation = landUnitData.getVariable("minimum_rotation")->value();
		if (minRotation.isEmpty()) {
			return false;
		}
        _minimumRotation = minRotation;

		_age = landUnitData.getVariable("age");
		_mat = landUnitData.getVariable("mean_annual_temperature");
		_spu = landUnitData.getVariable("spatial_unit_id");
        _isDecaying = landUnitData.getVariable("is_decaying");

        // Get the stand age of this land unit.
		const auto& initialAge = landUnitData.getVariable("initial_age")->value();
		if (initialAge.isEmpty()) {
			return false;
		}
		_standAge = initialAge;
                
		// Set and pass the delay information.
		_delay = landUnitData.getVariable("delay");
		_delay->set_value(_standDelay);

		_aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
		_belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");

        if (landUnitData.hasVariable("last_pass_disturbance_timeseries")) {
            _lastPassDisturbanceTimeseries = landUnitData.getVariable("last_pass_disturbance_timeseries");
        }

		return true;
    }

    bool CBMSpinupSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
        // Get spinup parameters for this land unit.
		try {
			if (!getSpinupParameters(*_landUnitData)) {
				return false;
			}
		} catch (const VariableNotFoundException& e) {
			MOJA_LOG_FATAL << boost::diagnostic_information(e, false);
			throw;
        } catch (const boost::exception& e) {
            MOJA_LOG_FATAL << boost::diagnostic_information(e);
            throw;
        } catch (const Exception& e) {
			MOJA_LOG_FATAL << boost::diagnostic_information(e);
			throw;
		} catch (const std::exception& e) {
			MOJA_LOG_FATAL << e.what();
			throw;
		}

        for (const auto pool : _landUnitData->poolCollection()) {
            if (pool->value() > 0.0) {
                // Skip spinup if any pool has explicitly been assigned an initial value.
                _age->set_value(_standAge);
                return true;
            }
        }

        try {
		    _landUnitData->getVariable("run_delay")->set_value("false");			

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
		    } else {
                runRegularSpinup(notificationCenter, luc, runMoss);
		    }

		    return true;
        } catch (SimulationError& e) {
            MOJA_LOG_FATAL << *boost::get_error_info<Details>(e);
            throw;
        } catch (const std::exception& e) {
            MOJA_LOG_FATAL << e.what();
            BOOST_THROW_EXCEPTION(SimulationError()
                << Details(e.what())
                << LibraryName("moja.modules.cbm")
                << ModuleName("unknown")
                << ErrorCode(0));
        }
    }

    void CBMSpinupSequencer::runPeatlandSpinup(NotificationCenter& notificationCenter, ILandUnitController& luc) {
        bool poolCached = false;
        const auto timing = _landUnitData->timing();
        auto mat = _mat->value();
        auto meanAnualTemperature = mat.isEmpty() ? 0
            : mat.type() == typeid(TimeSeries) ? mat.extract<TimeSeries>().value()
            : mat.convert<double>();

        auto lastFireYear = _landUnitData->getVariable("last_fire_year")->value();
        int lastFireYearValue = lastFireYear.isEmpty() ? -1 : lastFireYear.convert<int>();

        auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
        int fireReturnIntervalValue = fireReturnInterval.isEmpty() ? -1 : fireReturnInterval.convert<int>();

        auto minimumPeatlandSpinupYears = _landUnitData->getVariable("minimum_peatland_spinup_years")->value();
        int minimumPeatlandSpinupYearsValue = minimumPeatlandSpinupYears.isEmpty() ? 100 : minimumPeatlandSpinupYears.convert<int>();

        auto peatlandFireRegrow = _landUnitData->getVariable("peatland_fire_regrow")->value();
        bool peatlandFireRegrowValue = peatlandFireRegrow.isEmpty() ? false : peatlandFireRegrow.convert<bool>();

		int peatlandId = _landUnitData->getVariable("peatlandId")->value();

        CacheKey cacheKey{
            _spu->value().convert<int>(),
            _historicDistType,
			peatlandId,
			fireReturnInterval,
            meanAnualTemperature			
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

		int currentRotation = 0;
		int peatlandMaxRotationValue = minimumPeatlandSpinupYearsValue / fireReturnIntervalValue;
		int peatlandSpinupStepsPerRotation = minimumPeatlandSpinupYearsValue > fireReturnIntervalValue ? fireReturnIntervalValue : minimumPeatlandSpinupYearsValue;		

		while (!poolCached && currentRotation++ <= peatlandMaxRotationValue) {
			//for spinup output 
			_landUnitData->getVariable("peatland_spinup_rotation")->set_value(currentRotation);

			// Reset the ages to ZERO.
			_landUnitData->getVariable("peatland_smalltree_age")->set_value(0);
			_landUnitData->getVariable("peatland_shrub_age")->set_value(0);
			_age->set_value(0);

			//fire spinup steps for 
            fireSpinupSequenceEvent(notificationCenter, luc, peatlandSpinupStepsPerRotation, false);

			// Peatland spinup is done, notify to simulate the historic disturbance.
			fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);   
        }

        int startYear = timing->startDate().year(); // Simulation start year.
        int minimumPeatlandWoodyAge = fireReturnIntervalValue; // Set the default regrow year.

        if (lastFireYearValue < 0) { // No last fire year record.
            minimumPeatlandWoodyAge = fireReturnIntervalValue;
        } else if (startYear - lastFireYearValue < 0) { // Fire occurred after simulation.
            minimumPeatlandWoodyAge = startYear - lastFireYearValue + fireReturnIntervalValue;
        } else { // Fire occurred before simulation.
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
            fireSpinupSequenceEvent(notificationCenter, luc, minimumPeatlandWoodyAge, false);
        }
    }

    void CBMSpinupSequencer::runRegularSpinup(NotificationCenter& notificationCenter, ILandUnitController& luc, bool runMoss) {
        bool poolCached = false;
        const auto timing = _landUnitData->timing();
        auto mat = _mat->value();
        auto meanAnualTemperature = mat.isEmpty() ? 0
            : mat.type() == typeid(TimeSeries) ? mat.extract<TimeSeries>().value()
            : mat.convert<double>();

        CacheKey cacheKey{
            _spu->value().convert<int>(),
            _historicDistType,
            _spinupGrowthCurveID,
            _ageReturnInterval,
            meanAnualTemperature
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
        }

        while (!poolCached && runMoss && !mossSlowPoolStable) {
            // Do moss spinup only.
            _landUnitData->getVariable("spinup_moss_only")->set_value(true);

            _age->set_value(0);
            fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, false);

            double currentMossSlowPoolValue = _featherMossSlow->value() + _sphagnumMossSlow->value();
            mossSlowPoolStable = isSlowPoolStable(lastMossSlowPoolValue, currentMossSlowPoolValue);
            lastMossSlowPoolValue = currentMossSlowPoolValue;

            if (mossSlowPoolStable) {
                // Now moss slow pool is stable, turn off the moss spinup flag.
                _landUnitData->getVariable("spinup_moss_only")->set_value(false);
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
        std::map<int, std::string> lastPassDisturbanceTimeseries;
        if (_lastPassDisturbanceTimeseries != nullptr) {
            const auto& lastPassTimeseries = _lastPassDisturbanceTimeseries->value();
            if (!lastPassTimeseries.isEmpty()) {
                for (const auto& event : lastPassTimeseries.extract<const std::vector<DynamicObject>>()) {
                    int year = event["year"];
                    std::string disturbanceType = event["disturbance_type"].extract<std::string>();
                    lastPassDisturbanceTimeseries[year] = disturbanceType;
                }
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

            int ageFromTimeseries = simStartYear - lastPassTimeseriesEndYear - 1;
            if (ageFromTimeseries < _standAge + _standDelay) {
                _standAge = _standAge == 0 ? 0 : ageFromTimeseries;
                _standDelay = _standDelay == 0 ? 0 : ageFromTimeseries;
            }
        }

        int finalLastPassYear = simStartYear - 1 - _standAge - _standDelay;
        if (lastPassDisturbanceTimeseries.find(finalLastPassYear) == lastPassDisturbanceTimeseries.end()) {
            lastPassDisturbanceTimeseries[finalLastPassYear] = _lastPassDistType;
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
        int preRampAgeGrowthYears = std::max(0, _standAge + _standDelay - rampLength);

        // Calculate the number of years of growth within the ramp period after the final last pass
        // disturbance: these timesteps advance the stand to its final age.
        int rampAgeGrowthYears = _standAge - preRampAgeGrowthYears;

        // Determine the number of pre- and post-ramp stand delay years to simulate, i.e. for stands with
        // a delay value instead of an age when their last pass disturbance is deforestation.
        int preRampDelayYears = preRampAgeGrowthYears - _standAge;
        int rampDelayYears = _standDelay;

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
        auto firstLastPassDisturbanceType = lastPassDisturbanceTimeseries[lastPassYear];
        fireHistoricalLastDisturbanceEvent(notificationCenter, luc, firstLastPassDisturbanceType);

        for (int i = 0; i < lastPassTimeseriesLength; i++) {
            lastPassYear++;
            bool inRamp = i >= preRampLastPassYears;
            fireSpinupSequenceEvent(notificationCenter, luc, 1, inRamp);
            if (lastPassDisturbanceTimeseries.find(lastPassYear) != lastPassDisturbanceTimeseries.end()) {
                auto lastPassDisturbanceType = lastPassDisturbanceTimeseries[lastPassYear];
                fireHistoricalLastDisturbanceEvent(notificationCenter, luc, lastPassDisturbanceType);
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
            fireSpinupSequenceEvent(notificationCenter, luc, _standDelay, true);
            _landUnitData->getVariable("run_delay")->set_value("false");
        }
    }

    bool CBMSpinupSequencer::isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue) {
        double changeRatio = 0;
        if (lastSlowPoolValue != 0) {
            changeRatio =  currentSlowPoolValue / lastSlowPoolValue;
        }

        return changeRatio > 0.999 && changeRatio < 1.001;
    }

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
        }
    }

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
	
	bool CBMSpinupSequencer::isPeatlandApplicable() {
		if (!_landUnitData->hasVariable("enable_peatland")) {
			return false;
		}

		bool peatlandEnabled = _landUnitData->getVariable("enable_peatland")->value();
		if (!peatlandEnabled) {
			return false;
		}

		auto peatlandId = _landUnitData->getVariable("peatland_class")->value();
		int peatland_id = peatlandId.isEmpty() ? -1 : peatlandId.convert<int>();
		_landUnitData->getVariable("peatlandId")->set_value(peatland_id);

		bool toSimulatePeatland = (peatlandEnabled && (peatland_id > 0));
		_landUnitData->getVariable("run_peatland")->set_value(toSimulatePeatland);

		return toSimulatePeatland;
	}	

	bool CBMSpinupSequencer::isMossApplicable(bool runPeatland) {
		bool toSimulateMoss = false;
		if (!_landUnitData->hasVariable("enable_moss")) {
			return false;
		}

		bool mossEnabled = _landUnitData->getVariable("enable_moss")->value();
		if (!mossEnabled) {
			return false;
		}

		// Have to check this because moss growth is function of yield curve's merchantable volume.
		bool isGrowthCurveDefined = _landUnitData->getVariable("growth_curve_id")->value() > 0;

		// Moss growth is based on leading species' growth.
		if (mossEnabled && isGrowthCurveDefined) {
			std::string mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
			std::string speciesName = _landUnitData->getVariable("leading_species")->value();

			// Can also get species from a spatial layer:
			// std::string speciesName2 = _landUnitData->getVariable("species")->value();
			boost::algorithm::to_lower(mossLeadingSpecies);
			boost::algorithm::to_lower(speciesName);
			bool leadingSpeciesMatched = boost::contains(speciesName, mossLeadingSpecies);

			if (mossEnabled && leadingSpeciesMatched) {
				if (!runPeatland) {
					// Wherever peatland is run, moss run is disabled.
					toSimulateMoss = true;					
				}
			}
		}

		_landUnitData->getVariable("run_moss")->set_value(toSimulateMoss);
		return toSimulateMoss;
	}

}}} // namespace moja::modules::cbm
