#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/logging.h"

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
        _historicDMID = spinupParams[CBMSpinupSequencer::historicDMID];
        _lastDMID = spinupParams[CBMSpinupSequencer::lastDMID];
		_standDelay = spinupParams[CBMSpinupSequencer::delay];
		_spinupGrowthCurveID = spinupParams[CBMSpinupSequencer::growthCurveID];
                
        _miniumRotation = landUnitData.getVariable("minimum_rotation")->value();

        _age = landUnitData.getVariable("age");
        _aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
        _belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");		

        // Get the stand age of this land unit.
        _standAge = landUnitData.getVariable("initial_age")->value();
                
		// set and pass the delay information
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
			_landUnitData->getVariable("spu")->value().convert<int>(),
			_historicDMID,
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

        // Record total slow pool carbon at the end of previous spinup pass (every 125 steps).
        double _lastSlowPoolValue = 0;	
        bool slowPoolStable = false;

		notificationCenter.postNotification(moja::signals::TimingInit);

		notificationCenter.postNotification(moja::signals::TimingPostInit);

        // Loop up to the maximum number of rotations/passes.
        int currentRotation = 0;
        while (!poolCached && ++currentRotation <= _maxRotationValue) {
            // Fire spinup pass, each pass is up to the stand age return interval.
            _age->set_value(0);
            fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval);

            // Get the slow pool values at the end of age interval.
            double aboveGroundSlowSoil = _aboveGroundSlowSoil->value();
            double belowGroundSlowSoil = _belowGroundSlowSoil->value();
            double currentSlowPoolValue = aboveGroundSlowSoil + belowGroundSlowSoil;

            // Check if the slow pool is stable.
            slowPoolStable = isSlowPoolStable(_lastSlowPoolValue, currentSlowPoolValue);

            // Update previous toal slow pool value.
            _lastSlowPoolValue = currentSlowPoolValue;

            if (slowPoolStable && currentRotation > _miniumRotation) {
                // Slow pool is stable, and the minimum rotations are done.
                MOJA_LOG_DEBUG << "Slow pool is stable at rotation: " << currentRotation;
                break;
            }								

            if (currentRotation == _maxRotationValue) {
                if (!slowPoolStable) {
                    MOJA_LOG_ERROR << "Slow pool is not stable at maximum rotation: " << currentRotation;
                }

                // Whenever the max rotations are reached, stop even if the
                // slow pool is not stable.
                break;
            }

            // CBM spinup is not done, notify to simulate the historic disturbance.
			notificationCenter.postNotificationWithPostNotification(
                moja::signals::DisturbanceEvent,
                std::make_shared<flint::DisturbanceEventNotification>(
                    &luc,
                    DynamicObject({ { "disturbance", _historicDMID } })).get());
		}

		if (!poolCached) {
			std::vector<double> cacheValue;
			auto pools = _landUnitData->poolCollection();
			for (auto& pool : pools) {
				cacheValue.push_back(pool->value());
			}

			_cache[cacheKey] = cacheValue;
		}
		
		// CBM spinup is done, notify to simulate the last disturbance.
		notificationCenter.postNotificationWithPostNotification(
            moja::signals::DisturbanceEvent,
            std::make_shared<flint::DisturbanceEventNotification>(
                &luc,
                DynamicObject({ { "disturbance", _lastDMID } })).get());

        // Fire up the spinup sequencer to grow the stand to the original stand age.
        _age->set_value(0);
        fireSpinupSequenceEvent(notificationCenter, luc, _standAge);

        if (_standDelay > 0) {
            // Fire up the spinup sequencer to do turnover and delay only   
            _landUnitData->getVariable("run_delay")->set_value("true");
            fireSpinupSequenceEvent(notificationCenter, luc, _standDelay);
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
                                                     int maximumSteps) {
        auto curStepDate = startDate;
        auto endStepDate = startDate;
        const auto timing = _landUnitData->timing();
        for (int curStep = 0; curStep < maximumSteps; curStep++) {
            timing->set_startStepDate(curStepDate);
            timing->set_endStepDate(endStepDate);
            timing->set_curStartDate(curStepDate);
            timing->set_curEndDate(endStepDate);
            timing->set_stepLengthInYears(1);
            timing->set_step(curStep);
            timing->set_fractionOfStep(1);

            auto useStartDate = curStepDate;

			notificationCenter.postNotificationWithPostNotification(moja::signals::TimingStep);
			notificationCenter.postNotificationWithPostNotification(moja::signals::TimingPreEndStep);
			notificationCenter.postNotification(moja::signals::TimingEndStep);
			notificationCenter.postNotification(moja::signals::TimingPostStep);

            curStepDate.addYears(1);
            endStepDate = curStepDate;
            endStepDate.addYears(1);
        }
    }

}}} // namespace moja::modules::cbm
