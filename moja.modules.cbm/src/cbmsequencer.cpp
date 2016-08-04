#include "moja/modules/cbm/cbmsequencer.h"

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {

	bool CBMSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
		int nSteps = _endDate.year() - _startDate.year();
		nSteps++; // For init step

		notificationCenter.postNotification(moja::signals::TimingInit);
		notificationCenter.postNotification(moja::signals::TimingPostInit);

		auto curStep = 1;
		auto curStepDate = _startDate;
		auto endStepDate = _startDate;
        const auto timing = _landUnitData->timing();
        while (curStepDate < _endDate) {
			timing->setStartStepDate(curStepDate);
			timing->setEndStepDate(endStepDate);
			timing->setCurStartDate(curStepDate);
			timing->setCurEndDate(endStepDate);
			timing->setStep(curStep);

			auto useStartDate = curStepDate;

			notificationCenter.postNotificationWithPostNotification(moja::signals::TimingStep);
			notificationCenter.postNotification(moja::signals::TimingPreEndStep);
			notificationCenter.postNotification(moja::signals::TimingEndStep);
			notificationCenter.postNotification(moja::signals::OutputStep);
			notificationCenter.postNotification(moja::signals::TimingPostStep);

			curStepDate.addYears(1);
			endStepDate = curStepDate;
			endStepDate.addYears(1);
			curStep++;
		}

		notificationCenter.postNotification(moja::signals::TimingShutdown);

		return true;
	};

}}} // namespace moja::modules::cbm
