#include "moja/flint/variable.h"

#include "moja/modules/cbm/peatlandpreparemodule.h"

#include <boost/algorithm/string.hpp> 

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandPrepareModule::configure(const DynamicObject& config) { 		
	}

    void PeatlandPrepareModule::subscribe(NotificationCenter& notificationCenter) { 
		notificationCenter.subscribe(signals::TimingInit,       &PeatlandPrepareModule::onTimingInit,       *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandPrepareModule::onLocalDomainInit,  *this);
	}
    

	void PeatlandPrepareModule::onLocalDomainInit(){
		_acrotelm = _landUnitData->getPool("Acrotelm");
		_catotelm = _landUnitData->getPool("Catotelm");
		_atmosphere = _landUnitData->getPool("Atmosphere");
	}

    void PeatlandPrepareModule::onTimingInit() {	
		_isInitialPoolLoaded = false;
		_isPeatland = _landUnitData->getVariable("run_peatland")->value();

		// if it is peatland, by landunit id, from database or spatial layer
		// get the initial data by variable "peatland_initial_pools"
		if (_isPeatland) {
			const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_pools")->value();
			loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());	
		}
    }

	void PeatlandPrepareModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {	
		auto init = _landUnitData->createStockOperation();	

		init->addTransfer(_atmosphere, _acrotelm, data["acrotelm"])
			->addTransfer(_atmosphere, _catotelm, data["catotelm"]);

		_landUnitData->submitOperation(init);
	}
}}}
