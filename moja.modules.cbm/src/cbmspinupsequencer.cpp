#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/logging.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <boost/algorithm/string.hpp> 

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
		_spinupGrowthCurveID = landUnitData.getVariable("growth_curve_id")->value();
                
        _minimumRotation = landUnitData.getVariable("minimum_rotation")->value();

		_age = landUnitData.getVariable("age");
		_aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
		_belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");
		_featherMossSlow = _landUnitData->getPool("FeatherMossSlow");
		_sphagnumMossSlow = _landUnitData->getPool("SphagnumMossSlow");

        // Get the stand age of this land unit.
        _standAge = landUnitData.getVariable("initial_age")->value();
                
		// Set and pass the delay information.
		_delay = landUnitData.getVariable("delay");
		_delay->set_value(_standDelay);

        return true;
    }

    bool CBMSpinupSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
        // Get spinup parameters for this land unit
        if (!getSpinupParameters(*_landUnitData)) {
            return false;
        }

        bool poolCached = false;
        CacheKey cacheKey{
			_landUnitData->getVariable("spatial_unit_id")->value().convert<int>(),
			_historicDistType,
			_spinupGrowthCurveID,
            _ageReturnInterval
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

		_landUnitData->getVariable("run_delay")->set_value("false");

		// Check and set run moss flag.
		bool runMoss = isMossApplicable();
		_landUnitData->getVariable("run_moss")->set_value(runMoss);
		
		// Check and set run peatland flag.
		bool runPeatland = isPeatlandApplicable();
		_landUnitData->getVariable("run_peatland")->set_value(runPeatland);
		
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

        bool slowPoolStable = false;
		bool mossSlowPoolStable = false;

		double lastSlowPoolValue = 0;
		double currentSlowPoolValue = 0;

		double lastMossSlowPoolValue = 0;				
		double currentMossSlowPoolValue = 0;		

        // Loop up to the maximum number of rotations/passes.
		int currentRotation = 0;
		while (!poolCached && ++currentRotation <= _maxRotationValue) {
			// Fire spinup pass, each pass is up to the stand age return interval.
			_age->set_value(0);
			fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, false);

			// Get the slow pool values at the end of age interval.			
			currentSlowPoolValue = _aboveGroundSlowSoil->value() + _belowGroundSlowSoil->value();			
			currentMossSlowPoolValue = _featherMossSlow->value() + _sphagnumMossSlow->value();

			// Check if the slow pool is stable.
			slowPoolStable = isSlowPoolStable(lastSlowPoolValue, currentSlowPoolValue);
			if (runMoss) {
				mossSlowPoolStable = isSlowPoolStable(lastMossSlowPoolValue, currentMossSlowPoolValue);
			}

			// Update previous toal slow pool value.
			lastSlowPoolValue = currentSlowPoolValue;			
			lastMossSlowPoolValue = currentMossSlowPoolValue;

			if (slowPoolStable && currentRotation > _minimumRotation) {
				// Slow pool is stable, and the minimum rotations are done.
				break;
			}								

			if (currentRotation == _maxRotationValue) {
				if (!slowPoolStable) {
					MOJA_LOG_ERROR << "Slow pool is not stable at maximum rotation: " << currentRotation;
				}

				// Whenever the max rotations are reached, stop even if the slow pool is not stable.
				break;
			}

			// CBM spinup is not done, notify to simulate the historic disturbance.
			fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);			
		}

		while (runMoss && !mossSlowPoolStable) {				
			//do moss spinup only
			_landUnitData->getVariable("spinup_moss_only")->set_value(true);

			// moss spinup is not done, notify to simulate the historic disturbance - wild fire.
			fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _historicDistType);

			_age->set_value(0);
			fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, false);
				
			currentMossSlowPoolValue = _featherMossSlow->value() + _sphagnumMossSlow->value();
			mossSlowPoolStable = isSlowPoolStable(lastMossSlowPoolValue, currentMossSlowPoolValue);

			lastMossSlowPoolValue = currentMossSlowPoolValue;

			if (mossSlowPoolStable){
				// now moss slow pool is stable, turn off the moss spinup flag
				_landUnitData->getVariable("spinup_moss_only")->set_value(false);				
				break;
			}
		}

        // Perform the optional ramp-up from spinup to regular simulation values.
        int rampLength = _rampStartDate.isNull()
            ? 0
            : luc.timing().startDate().year() - _rampStartDate.value().year();

        int extraYears = rampLength - _standAge - _standDelay;
        int extraRotations = extraYears > 0 ? extraYears / _ageReturnInterval : 0;
        int finalRotationLength = extraYears > 0 ? extraYears % _ageReturnInterval : 0;

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
		
		// Spinup is done, notify to simulate the last pass disturbance.
		fireHistoricalLastDisturbanceEvent(notificationCenter, luc, _lastPassDistType);

        // Determine the number of years the final stages of the simulation need to
        // run without advancing the timestep into the ramp-up period; i.e. all spinup
        // timeseries variables are aligned to the end of spinup for each pixel.
        int yearsBeforeRamp = rampLength > (_standAge + _standDelay) ? 0
            : _standAge + _standDelay - rampLength;

        int preRampGrowthYears = yearsBeforeRamp > _standAge ? _standAge : yearsBeforeRamp;
        int rampGrowthYears = yearsBeforeRamp > _standAge ? 0 : _standAge - preRampGrowthYears;
        int preRampDelayYears = rampGrowthYears > 0 ? 0 : yearsBeforeRamp - _standAge;

		// Fire up the spinup sequencer to grow the stand to the original stand age.
		_age->set_value(0);
        fireSpinupSequenceEvent(notificationCenter, luc, preRampGrowthYears, false);
        fireSpinupSequenceEvent(notificationCenter, luc, rampGrowthYears, true);

		if (_standDelay > 0) {
			// if there is stand delay due to deforestation disturbance
			// Fire up the stand delay to do turnover and decay only   
			_landUnitData->getVariable("run_delay")->set_value("true");
            fireSpinupSequenceEvent(notificationCenter, luc, preRampDelayYears, false);
			fireSpinupSequenceEvent(notificationCenter, luc, _standDelay, true);
			_landUnitData->getVariable("run_delay")->set_value("false");
		}
   
		return true;
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
		// Create a place holder vector to keep the event pool transfers.
		auto transfer = std::make_shared<std::vector<CBMDistEventTransfer::Ptr>>();

		// Fire the disturbance with the transfers vector to be filled in by
        // any modules that build the disturbance matrix.
		Dynamic data = DynamicObject({ 
			{ "disturbance", disturbanceName },
			{ "transfers", transfer } 
		});

		notificationCenter.postNotificationWithPostNotification(
			moja::signals::DisturbanceEvent, data);
	}

	bool CBMSpinupSequencer::isMossApplicable() {
		bool toSimulateMoss = false;
		bool mossEnabled = _landUnitData->getVariable("enable_moss")->value();

		if (mossEnabled) {
			std::string mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
			std::string speciesName = _landUnitData->getVariable("species")->value();
			boost::algorithm::to_lower(speciesName);		

			if (mossEnabled && speciesName.compare(mossLeadingSpecies) == 0) {					
				toSimulateMoss = true;
			}
		}

		return toSimulateMoss;
	}

	bool CBMSpinupSequencer::isPeatlandApplicable() {
		// Todo: add logic to check if it is a peatland.
		// Temporarily use a variable - eventually get related information
        // from a spatial layer.
		bool toSimulatePeatland = _landUnitData->getVariable("enable_peatland")->value();

		return toSimulatePeatland;
	}

}}} // namespace moja::modules::cbm
