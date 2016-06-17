#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/mossdisturbancemodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <boost/algorithm/string.hpp> 

namespace moja {
namespace modules {
namespace cbm {

    void MossDisturbanceModule::configure(const DynamicObject& config) { 		
	}

    void MossDisturbanceModule::subscribe(NotificationCenter& notificationCenter) { 
		notificationCenter.connect_signal(signals::LocalDomainInit, &MossDisturbanceModule::onLocalDomainInit, *this);
		notificationCenter.connect_signal(signals::DisturbanceEvent, &MossDisturbanceModule::onDisturbanceEvent, *this);
		notificationCenter.connect_signal(signals::TimingInit, &MossDisturbanceModule::onTimingInit, *this);
	}
    
	void MossDisturbanceModule::onLocalDomainInit() {
		// get the data by variable "moss_fire_parameters"
		const auto& mossFireParams = _landUnitData->getVariable("moss_fire_parameters")->value();

		recordMossTransfers(mossFireParams.extract<DynamicObject>());
	}

    void MossDisturbanceModule::onTimingInit() {			
		_isMoss = _landUnitData->getVariable("run_moss")->value();
		//_isMoss = true; //temp set
    }
    
	void MossDisturbanceModule::onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr n) {
		if (!_isMoss) { return; } //skip if it is not a peatland

		// Get the disturbance type for either historical or last disturbance event.
		std::string disturbanceType = n->event()["disturbance"];
		boost::algorithm::to_lower(disturbanceType);

		//check if it is fire disturbance
		std::size_t foundFire = disturbanceType.find(MossDisturbanceModule::fireEvent);


		if (_isMoss && foundFire) {
			auto distMatrix = n->event()["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer::Ptr>>>();

			std::string sourcePoolName;
			std::string sinkPoolName;
			
			int transferIndex = 0;
			double actualRate = 0.0;

			for (int sourceIndex = 0; sourceIndex < _sourcePools.size(); sourceIndex++){
				for (int sinkIndex = 0; sinkIndex < _destPools.size(); sinkIndex++){
					sourcePoolName = _sourcePools.at(sourceIndex);
					sinkPoolName = _destPools.at(sinkIndex);

					actualRate = _transferRates.at(transferIndex++);
					if (actualRate > 0.0) {
						auto transferCO2 = std::make_shared<CBMDistEventTransfer>(*_landUnitData, sourcePoolName, sinkPoolName, actualRate);
						distMatrix->push_back(transferCO2);
					}
				}
			}				
		}
	}

    void MossDisturbanceModule::recordMossTransfers(const DynamicObject& data) {
		_transferRates.clear();

    	_transferRates.push_back(data["FL2CO2"]);     
		_transferRates.push_back(data["FL2CH4"]);     
		_transferRates.push_back(data["FL2CO"]);      
		_transferRates.push_back(data["FL2FS"]);      
		_transferRates.push_back(data["FL2SS"]);      
		_transferRates.push_back(data["SL2CO2"]);     
		_transferRates.push_back(data["SL2CH4"]);     
		_transferRates.push_back(data["SL2CO"]);      
		_transferRates.push_back(data["SL2FS"]);      
		_transferRates.push_back(data["SL2SS"]);      
		_transferRates.push_back(data["FF2CO2"]);     
		_transferRates.push_back(data["FF2CH4"]);     
		_transferRates.push_back(data["FF2CO"]);      
		_transferRates.push_back(data["FF2FS"]);      
		_transferRates.push_back(data["FF2SS"]);      
		_transferRates.push_back(data["SF2CO2"]);     
		_transferRates.push_back(data["SF2CH4"]);     
		_transferRates.push_back(data["SF2CO"]);      
		_transferRates.push_back(data["SF2FS"]);      
		_transferRates.push_back(data["SF2SS"]);
    }
}}}