/**
 * @file 
 * @brief The CBMAgeIndicators module is responsible for setting the age class of the active 
pixel based on its actual age. The GCBM’s database output uses age class instead of 
exact age to keep the size of results manageable.

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
     * object _ageClassHelper of AgeClassHelper is instantiated
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
     * Obtain the ageClass of variable "age" in _landUnitData using AgeClassHelper.toAgeClass(), \n,
     * assign this to variable "age_class" in _landUnitData
     * 
     * @return void
     * ************************/

	void CBMAgeIndicators::doTimingStep() {
		int standAge = _landUnitData->getVariable("age")->value();
		int ageClass = _ageClassHelper.toAgeClass(standAge);
		_landUnitData->getVariable("age_class")->set_value(ageClass);
	}
}}}
