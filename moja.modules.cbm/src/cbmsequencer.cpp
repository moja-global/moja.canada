#include "moja/modules/cbm/cbmsequencer.h"

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {

	bool CBMSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
		int nSteps = endDate.year() - startDate.year();
		nSteps++; // For init step

		notificationCenter.postNotification(std::make_shared<TimingInitNotification>(&luc, nSteps, startDate, endDate));
		notificationCenter.postNotification(std::make_shared<TimingPostInitNotification>());

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

			notificationCenter.postNotification(std::make_shared<flint::TimingStepNotification>(&luc, curStep, 1, useStartDate, endStepDate), std::make_shared<PostNotificationNotification>(&luc, "TimingStepNotification"));
			notificationCenter.postNotification(std::make_shared<TimingPreEndStepNotification>(&luc, endStepDate));
			notificationCenter.postNotification(std::make_shared<flint::TimingEndStepNotification>(&luc, endStepDate));
			notificationCenter.postNotification(std::make_shared<flint::OutputStepNotification>(&luc));
			notificationCenter.postNotification(std::make_shared<flint::TimingPostStepNotification>(&luc, endStepDate));

			curStepDate.addYears(1);
			endStepDate = curStepDate;
			endStepDate.addYears(1);
			curStep++;
		}

		auto tEnd = std::make_shared<flint::TimingShutdownNotification>();
		notificationCenter.postNotification(tEnd);

		return true;
	};

}}} // namespace moja::modules::cbm
