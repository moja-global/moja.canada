#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/peatlanddisturbance.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <boost/algorithm/string.hpp> 

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * Configuration function
	 * 
	 * @param DynamicObject& config
	 * @return void
	 * ************/
    void PeatlandDisturbance::configure(const DynamicObject& config) { }

	/**
	 * Subscribe to the signals LocalDomainInit, DisturbanceEvent and TimingInit
	 * 
	 * @param NotificationCenter& notificationCenter
	 * @return void
	 * ******************/
    void PeatlandDisturbance::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.connect_signal(signals::LocalDomainInit,  &PeatlandDisturbance::onLocalDomainInit,  *this);
        notificationCenter.connect_signal(signals::DisturbanceEvent, &PeatlandDisturbance::onDisturbanceEvent, *this);
		notificationCenter.connect_signal(signals::TimingInit,       &PeatlandDisturbance::onTimingInit,       *this);
	}

	/**
	 * 
	 * Assign PeatlandDisturbance._spu value of variable "spatial_unit_id" in _landUnitData
	 * 
	 * @return void
	 * *************/
    void PeatlandDisturbance::onLocalDomainInit() {       
        _spu = _landUnitData->getVariable("spatial_unit_id");
    }

     /**
	 * 
	 * Invoke PeatlandDisturbance.fetchDMAssociations(), \n
	 * Assign the value of PeatlandDisturbance._spu to PeatlandDisturbance._spuId
	 * 
	 * @return void
	 * *************/
    void PeatlandDisturbance::onTimingInit() {
		fetchDMAssociations();
        _spuId = _spu->value();
    }

	/**
	 * onDisturbanceEvent
	 * 
	 * Assign a variable disturbancetype, the disturbance type for either historical or last disturbance event from parameter n, \n
	 * Create a variable dmId 
	 * 
	 * 
	 * @param flint::DisturbanceEventNotification::Ptr n
	 * @return void
	 * ************/
    void PeatlandDisturbance::onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr n) {
        // Get the disturbance type for either historical or last disturbance event.
        std::string disturbanceType = n->event()["disturbance"];
		auto transfer = n->event()["transfers"].extract<const std::vector<CBMDistEventTransfer::Ptr>>();

        auto dmId = _dmAssociations.at(std::make_pair(disturbanceType, _spuId));
		auto transferTwo = std::make_shared<CBMDistEventTransfer>(*_landUnitData, "CO2", "CH4", dmId, 0.102);

		transfer.push_back(transferTwo);        
    } 

	/**
	* Insert DMAssociations.
	* 
	* Clear contents of PeatlandDisturbance._dmAssociations \n
	* For each disturbance matrix association contained in variable "disturbance_matrix_associations" of _landUnitData, \n
	* insert into PeatlandDisturbance._dmAssociations the values of variables "disturbance_type", "spatial_unit_id", \n
	* and "disturbance_matrix_id" as a triplet using make_pair
	* 
	* ********************/
	void PeatlandDisturbance::fetchDMAssociations() {
		_dmAssociations.clear();
		const auto& dmAssociations = _landUnitData->getVariable("disturbance_matrix_associations")->value()
			.extract<const std::vector<DynamicObject>>();

		for (const auto& dmAssociation : dmAssociations) {
			std::string disturbanceType = dmAssociation["disturbance_type"];
			int spu = dmAssociation["spatial_unit_id"];
			int dmId = dmAssociation["disturbance_matrix_id"];
			_dmAssociations.insert(std::make_pair(
				std::make_pair(disturbanceType, spu),
				dmId));
		}
	}

}
}
}
