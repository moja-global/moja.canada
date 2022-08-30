/**
* @file
* 
* ******/
#include "moja/modules/cbm/cbmsequencer.h"

#include <moja/flint/flintexceptions.h>
#include <moja/signals.h>

using namespace moja::flint;

namespace moja {
namespace modules {
namespace cbm {

	/**
    * Post event notifications
	* 
	* Post event notifications for signals TimingInit and TimingPostInit. \n
	* While CBMSequencer._startDate is less than CBMSequencer._endDate, post event notifications for signals \n 
	* TimingPreEndStep, TimingEndStep. OutputStep and TimingPostStep. In every iteration, increment the value of curStepDate by 1 year \n
	* Post event notifications for signal TimingShutdown at the end of the iteration and return True if no exception
	* 
    * @param notificationCenter NotificationCenter&
	* @param luc flint::ILandUnitController&
	* @exception SimulationError&: Handle exceptions during simulation
    * @exception exception: Handle exceptions
    * @return bool
    * ************************/
	bool CBMSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
		try {
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
            
            // If there are timeseries variables defined for the main simulation
            // without a matching definition in the spinup variables list, this
            // ensures that spinup consistently uses the first value in the series.
            timing->init();

			return true;
		} catch (const SimulationError&) {
			throw;
		} catch (const std::exception& e) {
			BOOST_THROW_EXCEPTION(SimulationError()
				<< Details(e.what())
				<< LibraryName("moja.modules.cbm")
				<< ModuleName("unknown")
				<< ErrorCode(0));
		}
	};

}}} // namespace moja::modules::cbm
