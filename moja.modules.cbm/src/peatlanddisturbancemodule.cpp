#include "moja/flint/variable.h"

#include "moja/modules/cbm/peatlanddisturbancemodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <boost/algorithm/string.hpp> 

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandDisturbanceModule::configure(const DynamicObject& config) { 		
	}

    void PeatlandDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandDisturbanceModule::onLocalDomainInit,  *this);
        notificationCenter.subscribe(signals::DisturbanceEvent, &PeatlandDisturbanceModule::onDisturbanceEvent, *this);
		notificationCenter.subscribe(signals::TimingInit,       &PeatlandDisturbanceModule::onTimingInit,       *this);
	}

   
	void PeatlandDisturbanceModule::onLocalDomainInit() { 		
        _spu = _landUnitData->getVariable("spatial_unit_id");
    }

    void PeatlandDisturbanceModule::onTimingInit() {			
        _spuId = _spu->value();		

		// get the data by variable "peatland_fire_parameters"
		const auto& peatlandFireParams = _landUnitData->getVariable("peatland_fire_parameters")->value();

		//create the PeatlandFireParameters, set the value from the variable
		_fireParameter = std::make_shared<PeatlandFireParameters>();
		_fireParameter->setValue(peatlandFireParams.extract<DynamicObject>());		

		_isPeatland = true; //temp set
    }

	void PeatlandDisturbanceModule::onDisturbanceEvent(Dynamic n) {
		if (!_isPeatland) { return; } //skip if it is not a peatland

		auto& data = n.extract<const DynamicObject>();

		// Get the disturbance type for either historical or last disturbance event.
		std::string disturbanceType = data["disturbance"];
		boost::algorithm::to_lower(disturbanceType);

		//check if it is fire disturbance
		std::size_t foundFire = disturbanceType.find(PeatlandDisturbanceModule::fireEvent);
		
		if (_isPeatland && foundFire) {
			auto distMatrix = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer::Ptr>>>();

			std::string sourcePoolName;
			std::string sinkPoolName;
			double parameter = 0.0; 
			double actualRate = 0.0;

			for (int sourceIndex = 0; sourceIndex < _sourcePools.size(); sourceIndex++){
				sourcePoolName = _sourcePools.at(sourceIndex);
				parameter = _fireParameter->baseRates().at(sourceIndex);

				actualRate = _fireParameter->computeToCO2Rate(parameter);
				if (actualRate > 0.0) {
					auto transferCO2 = std::make_shared<CBMDistEventTransfer>(*_landUnitData, sourcePoolName, PeatlandDisturbanceModule::CO2, actualRate);
					distMatrix->push_back(transferCO2);
					//MOJA_LOG_INFO << sourcePoolName << "->" << "CO2: " << actualRate;
				}

				actualRate = _fireParameter->computeToCORate(parameter);
				if (actualRate > 0.0) {
					auto transferCO = std::make_shared<CBMDistEventTransfer>(*_landUnitData, sourcePoolName, PeatlandDisturbanceModule::CO, actualRate);
					distMatrix->push_back(transferCO);
					//MOJA_LOG_INFO << sourcePoolName << "->" << "CO: " << actualRate;
				}

				actualRate = _fireParameter->computeToCH4Rate(parameter);
				if (actualRate > 0.0) {
					auto transferCH4 = std::make_shared<CBMDistEventTransfer>(*_landUnitData, sourcePoolName, PeatlandDisturbanceModule::CH4, actualRate);
					distMatrix->push_back(transferCH4);
					//MOJA_LOG_INFO << sourcePoolName << "->" << "CH4: " << actualRate;
				}
			}

			//Acrotelm pool, index = 13
			//for woodyRootsLive, poolIndex = 2
			auto wroots = std::make_shared<CBMDistEventTransfer>(*_landUnitData, _sourcePools.at(2), _sourcePools.at(13), _fireParameter->CTwr());
			distMatrix->push_back(wroots);

			//MOJA_LOG_INFO << _sourcePools.at(2) << "->" << _sourcePools.at(13) <<": " << _fireParameter->CTwr();

			//for sedgeRootsLive, poolIndex = 4
			auto sroots = std::make_shared<CBMDistEventTransfer>(*_landUnitData, _sourcePools.at(4), _sourcePools.at(13), _fireParameter->CTsr());
			distMatrix->push_back(sroots);		
			//MOJA_LOG_INFO << _sourcePools.at(4) << "->" << _sourcePools.at(13) <<": " << _fireParameter->CTsr();
		}
    } 
}}}
