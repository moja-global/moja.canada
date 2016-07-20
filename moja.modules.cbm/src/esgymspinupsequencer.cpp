#include "moja/modules/cbm/esgymspinupsequencer.h"
#include "moja/logging.h"
#include <boost/algorithm/string.hpp> 
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {
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
		_standDelay = spinupParams[ESGYMSpinupSequencer::delay];

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

	bool ESGYMSpinupSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
		// Get spinup parameters for this land unit
		if (!getSpinupParameters(*_landUnitData)) {
			return false;
		}

		_landUnitData->getVariable("run_delay")->set_value("false");

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
			fireSpinupSequenceEvent(notificationCenter, luc, _ageReturnInterval);

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
			fireHistoricalLastDisturbnceEvent(notificationCenter, luc, _historicDistType);
		}
		return true;
	}

	bool ESGYMSpinupSequencer::isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue) {
		double changeRatio = 0;
		if (lastSlowPoolValue != 0) {
			changeRatio = currentSlowPoolValue / lastSlowPoolValue;
		}

		return changeRatio > 0.999 && changeRatio < 1.001;
	}

	void ESGYMSpinupSequencer::fireSpinupSequenceEvent(NotificationCenter& notificationCenter,
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

	void ESGYMSpinupSequencer::fireHistoricalLastDisturbnceEvent(NotificationCenter& notificationCenter,
		ILandUnitController& luc,
		std::string disturbanceName) {
		//create a place holder vector to keep the event pool transfers
		auto transfer = std::make_shared<std::vector<CBMDistEventTransfer::Ptr>>();

		//fire the disturbance with the transfer
		Dynamic data = DynamicObject({
			{ "disturbance", disturbanceName },
			{ "transfers", transfer }
		});
		notificationCenter.postNotificationWithPostNotification(
			moja::signals::DisturbanceEvent, data);

	}

}}}