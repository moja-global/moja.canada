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
		flint::IVariable* _age_class_range = this->_landUnitData->getVariable("age_class_range");
		flint::IVariable* _age_maximum = this->_landUnitData->getVariable("age_maximum");

		age_class_range = 20;
		if (_age_class_range != NULL) {
			age_class_range = _age_class_range->value();
		}		

		int age_maximum = 300;
		if (_age_maximum != NULL) {
			age_maximum = _age_maximum->value();
		}

		number_of_age_classes = age_maximum / age_class_range;
	}  

	void CBMAgeIndicators::doTimingStep() {
		int standAge = this->_landUnitData->getVariable("age")->value();
		int ageClass = toAgeClass(standAge);

		this->_landUnitData->getVariable("age_class")->set_value(ageClass);
	}	

	int CBMAgeIndicators::toAgeClass(int standAge) {	
		int first_end_point = age_class_range - 1;	// The endpoint age of the first age class.
		double offset;					// An offset of the age to ensure that the first age class will have the endpoint FIRSTENDPOINT.
		double classNum;				// The age class as an double.
		double temp;					// The integral part of the age class as a double.									 
		if (standAge < 0) {
			return 0;
		}

		/* Calculate the age class as an integer.  First determine the offset to ensure the correct endpoint of the first
		* age class and use this value in calculating the age class. */
		offset = first_end_point - (age_class_range / 2.0) + 0.5;
		classNum = ((standAge - offset) / age_class_range) + 1.0;

		if (modf(classNum, &temp) >= 0.5) {
			classNum = ceil(classNum);
		}
		else {
			classNum = floor(classNum);
		}

		/* If the calculated age class is too great, use the oldest age class. */
		if ((int)classNum >= number_of_age_classes)
			classNum = (double)(number_of_age_classes - 1);

		/* Convert the age class as an integer into an age class. */
		return ((int)classNum);		
	}
}}}
