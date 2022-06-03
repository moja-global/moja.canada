/**
 * @file 
 * @brief Compute and update forest land unit age class information.
 * ******/

#include "moja/modules/cbm/cbmageindicators.h"
#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

	 /**
     * @brief Configuration function.
     * 
     * @param config DynamicObject&
     * @return void
     * ************************/

    void CBMAgeIndicators::configure(const DynamicObject& config) { }

	 /**
     * @brief Subscribe to the signals TimingInit, TimingEndStep and LocalDomainInit.
     * 
     * @param notificationCenter NotificationCenter&
     * @return void
     * ************************/

	void CBMAgeIndicators::subscribe(NotificationCenter& notificationCenter) {		
        notificationCenter.subscribe(signals::TimingInit,       &CBMAgeIndicators::onTimingStep,      *this);
        notificationCenter.subscribe(signals::TimingEndStep,    &CBMAgeIndicators::onTimingStep,      *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &CBMAgeIndicators::onLocalDomainInit, *this);
	}
    
	 /**
     * @brief Initiate Local Domain.
     * 
     * Event is fired at the start of the simulation. If _landUnitData has variables "age_class_range" and "age_maximum" , \n 
     * _ageClassHelper of class AgeClassHelper is instantiated
     * 
     * @return void
     * ************************/

	void CBMAgeIndicators::doLocalDomainInit() {
		if (_landUnitData->hasVariable("age_class_range") && _landUnitData->hasVariable("age_maximum")) {
			int ageClassRange = _landUnitData->getVariable("age_class_range")->value();
            int ageMaximum = _landUnitData->getVariable("age_maximum")->value();
            _ageClassHelper = AgeClassHelper(ageClassRange, ageMaximum);
        }
	}

	 /**
     * @brief Perform on each timing step.
     * 
     * standAge (integer variable) is determined by _landUnitData->getVariable("age") \n
     * ageClass (integer variable) is determined by AgeClassHelper.toAgeClass(standAge) \n
     * Variable "age_class" in _landUnitData is set to ageClass
     * 
     * _landUnitData is used to get reference to pools , state variables associated with a module and \n
     * create and submit carbon transfers (createStockOperation/createProportionalOperation/submitOperation)
     * @return void
     * ************************/

	void CBMAgeIndicators::doTimingStep() {
		int standAge = _landUnitData->getVariable("age")->value();
		int ageClass = _ageClassHelper.toAgeClass(standAge);
		_landUnitData->getVariable("age_class")->set_value(ageClass);
	}
}}}
