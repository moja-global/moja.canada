#include "moja/modules/cbm/cbmsequencer.h"

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {

	bool CBMSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
		int nSteps = endDate.year() - startDate.year();
		nSteps++; // For init step

		notificationCenter.postNotification(moja::signals::TimingInit);
		notificationCenter.postNotification(moja::signals::TimingPostInit);

		auto curStep = 1;
		auto curStepDate = startDate;
		auto endStepDate = startDate;
		const auto timing = _landUnitData->timing();
		while (curStepDate < endDate)
		{
			timing->set_startStepDate(curStepDate);
			timing->set_endStepDate(endStepDate);
			timing->set_curStartDate(curStepDate);
			timing->set_curEndDate(endStepDate);
			timing->set_stepLengthInYears(1);
			timing->set_step(curStep);
			timing->set_fractionOfStep(1);

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
