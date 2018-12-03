#include "moja/modules/cbm/cbmageindicators.h"

#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    void CBMAgeIndicators::configure(const DynamicObject& config) { }

	void CBMAgeIndicators::subscribe(NotificationCenter& notificationCenter) {		
        notificationCenter.subscribe(signals::TimingInit,       &CBMAgeIndicators::onTimingStep,      *this);
        notificationCenter.subscribe(signals::TimingEndStep,    &CBMAgeIndicators::onTimingStep,      *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &CBMAgeIndicators::onLocalDomainInit, *this);
	}
    
	void CBMAgeIndicators::doLocalDomainInit() {
		if (_landUnitData->hasVariable("age_class_range") && _landUnitData->hasVariable("age_maximum")) {
			int ageClassRange = _landUnitData->getVariable("age_class_range")->value();
            int ageMaximum = _landUnitData->getVariable("age_maximum")->value();
            _ageClassHelper = AgeClassHelper(ageClassRange, ageMaximum);
        }
	}

	void CBMAgeIndicators::doTimingStep() {
		int standAge = _landUnitData->getVariable("age")->value();
		int ageClass = _ageClassHelper.toAgeClass(standAge);
		_landUnitData->getVariable("age_class")->set_value(ageClass);
	}
}}}
