#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/peatlandpreparemodule.h"

#include <boost/algorithm/string.hpp> 

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandPrepareModule::configure(const DynamicObject& config) { 		
	}

    void PeatlandPrepareModule::subscribe(NotificationCenter& notificationCenter) { 
		notificationCenter.connect_signal(signals::TimingInit,       &PeatlandPrepareModule::onTimingInit,       *this);
	}
    

    void PeatlandPrepareModule::onTimingInit() {	
		_isInitialPoolLoaded = false;
		_isPeatland = _landUnitData->getVariable("run_peatland")->value();
    }
}}}
