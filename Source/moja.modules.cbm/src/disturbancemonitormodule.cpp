#include "moja/modules/cbm/disturbancemonitormodule.h"

#include <moja/flint/ivariable.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * @brief Subscribe to signals LocalDomainInit, TimingInit, OutputStep and DisturbanceEvent  
	 * 
	 * *********************/
    void DisturbanceMonitorModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit,  &DisturbanceMonitorModule::onLocalDomainInit,  *this);
        notificationCenter.subscribe(signals::TimingInit,       &DisturbanceMonitorModule::onTimingInit,       *this);
		notificationCenter.subscribe(signals::OutputStep,       &DisturbanceMonitorModule::onOutputStep,       *this);
		notificationCenter.subscribe(signals::DisturbanceEvent, &DisturbanceMonitorModule::onDisturbanceEvent, *this);
	}

	/**
	 * @brief Initialise Local Domain
	 * 
	 * Assign the value of the variable "current_disturbance" in _landUnitData to DisturbanceMonitorModule._moduleEnabled \n
	 * If _moduleEnabled is true, then DisturbanceMonitorModule._moduleEnabled is set to \n
	 * variable "current_disturbance" in _landUnitData
	 * 
	 * @return void
	 * ********************/
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
		int distType = data["disturbance_type_code"].extract<int>();
        _currentDisturbance->set_value(distType);
	}

}}} // namespace moja::modules::cbm
