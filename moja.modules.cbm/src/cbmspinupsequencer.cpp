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
        _historicDistTypeID = spinupParams[CBMSpinupSequencer::historicDistTypeID];
        _lastDistTypeID = spinupParams[CBMSpinupSequencer::lastDistTypeID];
		_standDelay = spinupParams[CBMSpinupSequencer::delay];
                
        _miniumRotation = landUnitData.getVariable("minimum_rotation")->value();

        _age = landUnitData.getVariable("age");
        _aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
        _belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");		

        // Get the stand age of this land unit.
        _standAge = landUnitData.getVariable("initial_age")->value();
        _age->set_value(_standAge);
                
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

		Int64 luid = _landUnitData->getVariable("LandUnitId")->value();

       CacheKey cacheKey{
            _landUnitData->getVariable("spu")->value().convert<int>(),
            _historicDistTypeID,
            _lastDistTypeID,
            _landUnitData->getVariable("growth_curve_id")->value().convert<int>()
        };
        auto it = _cache.find(cacheKey);
        if (it != _cache.end()) {
            auto cachedResult = (*it).second;

			auto pools = luc.poolCollection();
			for (auto& pool : *pools) {
				pool.set_value(cachedResult[pool.idx()]);
			}
            return true;
        }

		_landUnitData->getVariable("run_delay")->set_value("false");
        bool slowPoolStable = false;
        bool lastRotation = false;

        int currentRotation = 0;
        double aboveGroundSlowSoil = 0;
        double belowGroundSlowSoil = 0;
        double currentSlowPoolValue = 0;	


        // Record total slow pool carbon at the end of previous spinup pass (every 125 steps).
        double _lastSlowPoolValue = 0;	

        notificationCenter.postNotification(std::make_shared<TimingInitNotification>(
            &luc, _ageReturnInterval, startDate, endDate));

        notificationCenter.postNotification(std::make_shared<TimingPostInitNotification>());

        // Loop up to the maximum number of rotations/passes.
        while (++currentRotation <= _maxRotationValue) {
            // Fire spinup pass, each pass is up to the stand age return interval.
            fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval);

            // At the end of each pass, set the current stand age as 0.
            _age->set_value(0);

            // Get the slow pool values at the end of age interval.
            aboveGroundSlowSoil = _aboveGroundSlowSoil->value();
            belowGroundSlowSoil = _belowGroundSlowSoil->value();
            currentSlowPoolValue = aboveGroundSlowSoil + belowGroundSlowSoil;

            // Check if the slow pool is stable.
            slowPoolStable = isSlowPoolStable(_lastSlowPoolValue, currentSlowPoolValue);

            // Update previous toal slow pool value.
            _lastSlowPoolValue = currentSlowPoolValue;

            if (slowPoolStable && currentRotation > _miniumRotation) {
                // Slow pool is stable, and the minimum rotations are done.
                MOJA_LOG_DEBUG << "Slow pool is stable at rotation: " << currentRotation;
                        
                // Set the last rotation flag as true.
                lastRotation = true;						
            }								

            if (currentRotation == _maxRotationValue) {
                if (!slowPoolStable) {
                    MOJA_LOG_ERROR << "Slow pool is not stable at maximum rotation: " << currentRotation;
                }

                // Whenever the max rotations are reached, set the last rotation
                // flag as true even if the slow pool is not stable.
                lastRotation = true;
            }

            if (lastRotation) {
                // CBM spinup is done, notify to simulate the last disturbance.
				notificationCenter.postNotification(std::make_shared<flint::DisturbanceEventNotification>(&luc,
					DynamicObject({ { "disturbance", _lastDistTypeID } })),
					std::make_shared<PostNotificationNotification>(&luc, "DisturbanceEventNotification"));

				_landUnitData->getVariable("run_delay")->set_value("true");
                break; // Exit the while (rotation) loop.
            }
            else {
                // CBM spinup is not done, notify to simulate the historic disturbance.
				notificationCenter.postNotification(std::make_shared<flint::DisturbanceEventNotification>(&luc,
                    DynamicObject({ { "disturbance", _historicDistTypeID }})),
					std::make_shared<PostNotificationNotification>(&luc, "DisturbanceEventNotification"));
            }		

			// At the end of each pass, set the current stand age as 0.
			_age->set_value(0);
        }

		if (_standDelay > 0){
			// Fire up the spinup sequencer to do turnover and delay only			
			_landUnitData->getVariable("run_delay")->set_value("true");
			fireSpinupSequenceEvent(notificationCenter, luc,  _standDelay);			
		}
			
        if (lastRotation) {			
			// turn off the last rotation to regrow to the stand age
			_landUnitData->getVariable("run_delay")->set_value("false");

			// Fire up the spinup sequencer to grow the stand to the original stand age.
            fireSpinupSequenceEvent(notificationCenter, luc, _standAge);
        }  

        std::vector<double> cacheValue;
        
		auto pools = luc.poolCollection();
		for (auto& pool : *pools) {
			cacheValue.push_back(pool.value());
		}
        _cache[cacheKey] = cacheValue;

        return true;
    }

    bool CBMSpinupSequencer::isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue) {
        bool stable = false;
        if (lastSlowPoolValue != 0) {
            double var =  currentSlowPoolValue / lastSlowPoolValue;					
            if (var > 0.999 && var < 1.001) {
                stable = true;
            }
        }

        return stable;
    }

    void CBMSpinupSequencer::fireSpinupSequenceEvent(NotificationCenter& notificationCenter, flint::ILandUnitController& luc, int maximumSteps) {
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

            notificationCenter.postNotification(
                std::make_shared<flint::TimingStepNotification>(&luc, curStep, 1, useStartDate, endStepDate),
                std::make_shared<PostNotificationNotification>(&luc, "TimingStepNotification"));

            notificationCenter.postNotification(std::make_shared<TimingPreEndStepNotification>(&luc, endStepDate),
				std::make_shared<PostNotificationNotification>(&luc, "TimingPreEndStepNotification"));
            notificationCenter.postNotification(std::make_shared<flint::TimingEndStepNotification>(&luc, endStepDate));
            notificationCenter.postNotification(std::make_shared<flint::TimingPostStepNotification>(&luc, endStepDate));

            curStepDate.addYears(1);
            endStepDate = curStepDate;
            endStepDate.addYears(1);
        }
    }

}}} // namespace moja::modules::cbm
