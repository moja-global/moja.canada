#include "moja/modules/cbm/cbmageindicators.h"

#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    void CBMAgeIndicators::configure(const DynamicObject& config) { 		
	}

	void CBMAgeIndicators::subscribe(NotificationCenter& notificationCenter) {		
		notificationCenter.subscribe(signals::TimingStep,       &CBMAgeIndicators::onTimingStep,      *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &CBMAgeIndicators::onLocalDomainInit, *this);
	}
    
	void CBMAgeIndicators::doLocalDomainInit() {
		int ageClassRange = 20; // default age class range
		if (_landUnitData->hasVariable("age_class_range")) {
			ageClassRange = _landUnitData->getVariable("age_class_range")->value();
		}

		int ageMaximum = 300; // default maximum age
		if (_landUnitData->hasVariable("age_maximum")) {
			ageMaximum = _landUnitData->getVariable("age_maximum")->value();
		}

		numAgeClasses = ageMaximum / ageClassRange;
	}  

	void CBMAgeIndicators::doTimingStep() {
		int standAge = this->_landUnitData->getVariable("age")->value();
		int ageClass = toAgeClass(standAge);

		this->_landUnitData->getVariable("age_class")->set_value(ageClass);
	}	

	int CBMAgeIndicators::toAgeClass(int standAge) {	
		int first_end_point = ageClassRange - 1;	// The endpoint age of the first age class.
		double offset;					// An offset of the age to ensure that the first age class will have the endpoint FIRSTENDPOINT.
		double classNum;				// The age class as an double.
		double temp;					// The integral part of the age class as a double.		

		//reserve 1 for non-forest stand with age < 0
		if (standAge < 0) {
			return 1;
		}
		// Calculate the age class as an integer starting from 2.  
		// in GCBM must use 2.0 for ageClassId offset
		offset = first_end_point - (ageClassRange / 2.0) + 0.5;
		classNum = ((standAge - offset) / ageClassRange) + 2.0;

		if (modf(classNum, &temp) >= 0.5) {
			classNum = ceil(classNum);
		}
		else {
			classNum = floor(classNum);
		}

		/* If the calculated age class is too great, use the oldest age class. */
		if ((int)classNum > numAgeClasses)
			classNum = numAgeClasses;

		/* Convert the age class as an integer into an age class. */
		return ((int)classNum);		
	}
}}}
