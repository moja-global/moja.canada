#include "moja/modules/cbm/esgymspinupsequencer.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ilandunitcontroller.h>

#include <moja/logging.h>
#include <moja/signals.h>

#include <boost/algorithm/string.hpp> 

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {

    /**
     * Get spinup parameters for this land unit
     * 
     * If value of variable "spinup_parameters" in parameter landUnitData is false, return false \n
     * Extract DynamicObject from variable "spinup_parameters" in parameter landUnitData and store it in a variable spinupParams \n
     * Assign values of ESGYMSpinupSequencer::returnInverval, ESGYMSpinupSequencer::maxRotation, ESGYMSpinupSequencer::historicDistType, ESGYMSpinupSequencer::lastDistType in spinupParams to
     * ESGYMSpinupSequencer._ageReturnInterval, ESGYMSpinupSequencer._maxRotationValue, ESGYMSpinupSequencer._historicDistType and ESGYMSpinupSequencer._lastPassDistType \n
     * If ESGYMSpinupSequencer::inventoryDelay exists in spinupParams, assign it to ESGYMSpinupSequencer._standDelay, else assign ESGYMSpinupSequencer::delay \n
     * Assign ESGYMSpinupSequencer._miniumRotation, ESGYMSpinupSequencer._age,ESGYMSpinupSequencer._standAge, 
     * ESGYMSpinupSequencer._delay values of variables "minimum_rotation", "age", "initial_age", "delay" in parameter landUnitData,
     * ESGYMSpinupSequencer._aboveGroundSlowSoil, ESGYMSpinupSequencer._belowGroundSlowSoil values of pools "AboveGroundSlowSoil", "BelowGroundSlowSoil" in parameter landUnitData \n
     * Set the value of ESGYMSpinupSequencer._delay to ESGYMSpinupSequencer._standDelay
     * 
     * Return true
     * 
     * @param landUnit flint::ILandUnitDataWrapper&
     * @return bool
     */
	bool ESGYMSpinupSequencer::getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData) {
		const auto& spinup = landUnitData.getVariable("spinup_parameters")->value();
		if (spinup.isEmpty()) {
			return false;
		}

		const auto& spinupParams = spinup.extract<DynamicObject>();
		_ageReturnInterval = spinupParams[ESGYMSpinupSequencer::returnInverval];
		_maxRotationValue = spinupParams[ESGYMSpinupSequencer::maxRotation];
		_historicDistType = spinupParams[ESGYMSpinupSequencer::historicDistType].convert<std::string>();
		_lastPassDistType = spinupParams[ESGYMSpinupSequencer::lastDistType].convert<std::string>();
        _standDelay = spinupParams.contains(ESGYMSpinupSequencer::inventoryDelay)
            ? spinupParams[ESGYMSpinupSequencer::inventoryDelay]
            : spinupParams[ESGYMSpinupSequencer::delay];

		_miniumRotation = landUnitData.getVariable("minimum_rotation")->value();

		_age = landUnitData.getVariable("age");
		_aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
		_belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");

		// Get the stand age of this land unit.
		_standAge = landUnitData.getVariable("initial_age")->value();

		// Set and pass the delay information.
		_delay = landUnitData.getVariable("delay");
		_delay->set_value(_standDelay);

		return true;
	}

    /**
     * If ESGYMSpinupSequencer._distTypeCodes is empty and _landUnitData has a variable "disturbance_type_codes", invoke ESGYMSpinupSequencer.fetchDistTypeCodes() \n
     * If getSpinupParameters() on _landUnitData returns false, return false \n
     * 
     * Set the stepping period of the simulation to TimeStepping::Annual \n 
     * If _rampStartDate is true, set the simulation start date to _rampStartDate, 
     * set the simulation end date to luc.timing().startDate(), 
     * set the current timestep start date, current timestamp end date, current fractional timestep start date and 
     * current fractional timestep end date to the start date of the simulation, result of the startDate() on _landUnitData->timing()
     * 
     * Post notifications moja::signals::TimingInit and moja::signals::TimingPostInit 
     * 
     * Create a variable currentRotation with value 0, till the time currentRotation is less than ESGYMSpinupSequencer._maxRotationValue, 
     * fire spinup pass, each pass is up to the stand age return interval, get the slow pool values at the end of the current age interval
     * as the sum of values _aboveGroundSlowSoil and _belowGroundSlowSoil and assign it to variable currentSlowPoolValue, 
     * the slow pool value at the end of the previous iteration is stored in variable lastSlowPoolValue \n
     * If the result of ESGYMSpinupSequencer.isSlowPoolStable() with arguments lastSlowPoolValue and currentSlowPoolValue is true,
     * and variable currentRotation > _miniumRotation, break from the loop, as slow pool is stable, and the minimum rotations are done \n
     * Whenever the max rotations are reached, stop even if the slow pool is not stable 
    
     * CBM spinup is not done, notify to simulate the historic disturbance.
     * 
     * Perform the optional ramp-up from spinup to regular simulation values \n
     * Determine the number of years the final stages of the simulation need to run without advancing the timestep into the ramp-up period; i.e. all spinup
     * timeseries variables are aligned to the end of spinup for each pixel.
     * 
     * Fire up the spinup sequencer to grow the stand to the original stand age, set value of _age to 0 \n
     * If there is stand delay, ESGYMSpinupSequencer._standDelay > 0 , due to deforestation disturbance, fire up the stand delay to do turnover and decay only   
     * 
     * @param notificationCenter flint::NotificationCenter&
     * @param luc flint::ILandUnitController&
     * @return bool
     */
	bool ESGYMSpinupSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
        if (_distTypeCodes.empty() && _landUnitData->hasVariable("disturbance_type_codes")) {
            fetchDistTypeCodes();
        }

		// Get spinup parameters for this land unit
		if (!getSpinupParameters(*_landUnitData)) {
			return false;
		}

		_landUnitData->getVariable("run_delay")->set_value("false");

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

		while (++currentRotation <= _maxRotationValue) {
			// Fire spinup pass, each pass is up to the stand age return interval.
			_age->set_value(0);
			fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval, false);

			// Get the slow pool values at the end of age interval.			
			currentSlowPoolValue = _aboveGroundSlowSoil->value() + _belowGroundSlowSoil->value();

			// Check if the slow pool is stable.
			slowPoolStable = isSlowPoolStable(lastSlowPoolValue, currentSlowPoolValue);

			// Update previous toal slow pool value.
			lastSlowPoolValue = currentSlowPoolValue;
			lastMossSlowPoolValue = currentMossSlowPoolValue;

			if (slowPoolStable && currentRotation > _miniumRotation) {
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

    /**
     * Check if the slow pool is stable
     * 
     * Return true if the ratio of parameter currentSlowPoolValue / parameter lastSlowPoolValue > 0.999 and < 1.001, 
     * where lastSlowPoolValue != 0, else return false
     * 
     * @param lastSlowPoolValue double
     * @param currentSlowPoolValue double
     * @return bool
     **/
	bool ESGYMSpinupSequencer::isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue) {
		double changeRatio = 0;
		if (lastSlowPoolValue != 0) {
			changeRatio = currentSlowPoolValue / lastSlowPoolValue;
		}

		return changeRatio > 0.999 && changeRatio < 1.001;
	}

    /**
     * Fire timing events
     * 
     * For each time step in the range 0 to paraemter maximumSteps, get the timing object from _landUnitData,
     * invoke setStep() on timing, set the current timestep to the previous timestep + 1, \n
     * setStartStepDate() on timing, increment the current timestep start date by 1 year, \n
     * setEndStepDate() on timing, increment the current timestep end date by 1 year, \n
     * setCurStartDate() on timing,  increment the current fractional timestep start date by 1 year, \n
     * setCurEndDate() on timing, increment the current fractional timestep end date by 1 year, \n
     * 
     * Post notifications moja::signals::TimingStep, moja::signals::TimingPreEndStep and also post a generic follow-up notification after the subscriber
     * receives the original signal \n
     * Post notification moja::signals::TimingEndStep and moja::signals::TimingPostStep 
     * 
     * @param notificationCenter NotificationCenter&
     * @param luc flint::ILandUnitController&
     * @param maximumSteps int
     * @param incrementStep bool
     * @return void
     */
    void ESGYMSpinupSequencer::fireSpinupSequenceEvent(
        NotificationCenter& notificationCenter,
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

    /**
     * Fire historical and last disturbance 
     * 
     * Post the moja::signals::DisturbanceEvent notification with argument data, and also post a generic follow-up notification after the subscriber
     * receives the original signal \n
     * data is a DynamicVar with key "disturbance", value as parameter disturbanceName, 
     * key "disturbance_type_code", which takes the value of parameter disturbanceName in ESGYMSpinupSequencer._distTypeCodes if count of parameter
     * disturbanceName in ESGYMSpinupSequencer._distTypeCodes > 0 else takes value -1 , 
     * key "transfers", a vector of CBMDistEventTransfer to keep the event pool transfers
     * 
     * @param notificationCenter NotificationCenter&
     * @param luc flint::ILandUnitController&
     * @param disturbanceName string
     * @return void
     */
	void ESGYMSpinupSequencer::fireHistoricalLastDisturbanceEvent(
        NotificationCenter& notificationCenter,
		ILandUnitController& luc,
		std::string disturbanceName) {

        // Create a place holder vector to keep the event pool transfers.
        auto transfer = std::make_shared<std::vector<CBMDistEventTransfer>>();
        int distCode = _distTypeCodes.count(disturbanceName) == 0 ? -1 : _distTypeCodes[disturbanceName];

        // Fire the disturbance with the transfers vector to be filled in by
        // any modules that build the disturbance matrix.
        DynamicVar data = DynamicObject({
            { "disturbance", disturbanceName },
            { "disturbance_type_code", distCode },
            { "transfers", transfer }
        });

        notificationCenter.postNotificationWithPostNotification(
            moja::signals::DisturbanceEvent, data);
	}

    /**
    * Get disturbance type and disturbance type codes
    * 
    * If _landUnitData has the variable "disturbance_type_codes", for each code in variable "disturbance_type_codes", 
    * populate ESGYMSpinupSequencer._distTypeCodes mapping the "disturbance_type" to the "disturbance_type_code" value.
    * 
    * @return void
    */    
    void ESGYMSpinupSequencer::fetchDistTypeCodes() {
        const auto& distTypeCodes = _landUnitData->getVariable("disturbance_type_codes")->value();
        if (distTypeCodes.isVector()) {
            for (const auto& code : distTypeCodes.extract<const std::vector<DynamicObject>>()) {
                std::string distType = code["disturbance_type"];
                int distTypeCode = code["disturbance_type_code"];
                _distTypeCodes[distType] = distTypeCode;
            }
        } else {
            std::string distType = distTypeCodes["disturbance_type"];
            int distTypeCode = distTypeCodes["disturbance_type_code"];
            _distTypeCodes[distType] = distTypeCode;
        }
    }
}}}