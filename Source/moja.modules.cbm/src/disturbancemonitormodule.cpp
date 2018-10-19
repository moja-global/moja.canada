#include "moja/modules/cbm/disturbancemonitormodule.h"

#include <moja/flint/ivariable.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    void DisturbanceMonitorModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit,  &DisturbanceMonitorModule::onLocalDomainInit,  *this);
        notificationCenter.subscribe(signals::TimingInit,       &DisturbanceMonitorModule::onTimingInit,       *this);
		notificationCenter.subscribe(signals::TimingStep,       &DisturbanceMonitorModule::onTimingStep,       *this);
		notificationCenter.subscribe(signals::DisturbanceEvent, &DisturbanceMonitorModule::onDisturbanceEvent, *this);
	}

	void DisturbanceMonitorModule::doLocalDomainInit() {
        _moduleEnabled = _landUnitData->hasVariable("current_disturbance");
        if (!_moduleEnabled) {
            return;
        }
        
        _currentDisturbance = _landUnitData->getVariable("current_disturbance");
    }

    void DisturbanceMonitorModule::doTimingInit() {
		if (!_moduleEnabled) {
            return;
        }
        
        _currentDisturbance->set_value(DynamicVar());
	}
    
	void DisturbanceMonitorModule::doOutputStep() {
		if (!_moduleEnabled) {
			return;
		}

        _currentDisturbance->set_value(DynamicVar());
	}

	void DisturbanceMonitorModule::doDisturbanceEvent(DynamicVar e) {
		if (!_moduleEnabled) {
			return;
		}

		auto& data = e.extract<const DynamicObject>();
		auto distType = data["disturbance"].convert<std::string>();
        _currentDisturbance->set_value(distType);
	}

}}} // namespace moja::modules::cbm
