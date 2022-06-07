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
	 * @return void
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

	/**
	 * @brief doTimingInit
	 * 
	 * If DisturbanceMonitorModule._moduleEnabled is true, set the value of DisturbanceMonitorModule._currentDisturbance to 
	 * DynamicVar()
	 * 
	 * @return void
	 * ********************/
    void DisturbanceMonitorModule::doTimingInit() {
		if (!_moduleEnabled) {
            return;
        }
        
        _currentDisturbance->set_value(DynamicVar());
	}
    
	/**
	 * @brief doOutputStep
	 * 
	 * If DisturbanceMonitorModule._moduleEnabled is true, set the value of DisturbanceMonitorModule._currentDisturbance to 
	 * DynamicVar()
	 * 
	 * @return void
	 * ********************/
	void DisturbanceMonitorModule::doOutputStep() {
		if (!_moduleEnabled) {
			return;
		}

        _currentDisturbance->set_value(DynamicVar());
	}

	/**
	 * @brief doDisturbanceEvent
	 * 
	 * If DisturbanceMonitorModule._moduleEnabled is true, then assign the value of variable "disturbance_type_code" in 
	 * parameter e to DisturbanceMonitorModule._currentDisturbance
	 * 
	 * @param DynamicVar e
	 * @return void
	 * ********************/
	void DisturbanceMonitorModule::doDisturbanceEvent(DynamicVar e) {
		if (!_moduleEnabled) {
			return;
		}

		auto& data = e.extract<const DynamicObject>();
		int distType = data["disturbance_type_code"].extract<int>();
        _currentDisturbance->set_value(distType);
	}

}}} // namespace moja::modules::cbm
