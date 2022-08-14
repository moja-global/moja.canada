/**
 * @file 
 * Moss module to response to the fire disturbance events in CBM
 ********************/
#include "moja/modules/cbm/mossdisturbancemodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/helper.h"

#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/logging.h>

#include <boost/algorithm/string.hpp> 

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 * 
			 * @param config const DynamicObject&
			 * @return void
			 * *********************/
			void MossDisturbanceModule::configure(const DynamicObject& config) {
			}

			/**
			 * Subscribe to the signals LocalDomainInit, DisturbanceEvent and TimingInit
			 * 
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 * **************************/
			void MossDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossDisturbanceModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &MossDisturbanceModule::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossDisturbanceModule::onTimingInit, *this);
			}

			/**
			 * Invoke MossDisturbanceModule.recordMossTransfers() on the value of variable "moss_fire_parameters" in _landUnitData
			 * 
			 * @return void
			 * **************************/
			void MossDisturbanceModule::doLocalDomainInit() {
				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {
					fetchMossDistMatrices();
					fetchMossDMAssociations();
				}
			}

			/**
			 * If variable "enable_moss" exists in _landUnitData and it has a value, 
			 * invoke Helper.runMoss() with arguments as value of variables "growth_curve_id", "moss_leading_species" and "leading_species" in _landUnitData \n
			 * Assign MossDisturbanceModule.runMoss to true if variable "peatland_class" in _landUnitData is empty, variable "growth_curve_id" in _landUnitData
			 * is not empty, and Helper.runMoss() returns true
			 * 
			 * @return void
			 * **************************/
			void MossDisturbanceModule::doTimingInit() {
				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {
					auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
					bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

					auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
					auto speciesName = _landUnitData->getVariable("leading_species")->value();

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					_runMoss = peatlandId < 0
						&& isGrowthCurveDefined
						&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
				}
			}

			/**
			 * If MossDisturbanceModule.runMoss is false, return. \n
			 * Else, get the disturbance type for either historical or last disturbance event from "disturbance" in parameter n, 
			 * and check if it is fire disturbance, MossDisturbanceModule::fireEvent \n
			 * If it is a fire disturbance and runMoss is true, for every source and sink pool pairs, in _sourcePools and _destPools, obtain th transfer rates from 
			 * _transferRates. \n
			 * Instantiate an object of CBMDistEventTransfer with *_landUnitData, sourcePoolName, sinkPoolName, transferRate and append it to "transfers" in parameter n
			 * 
			 * @param n DynamicVar
			 * @return void
			 * **********************/
			void MossDisturbanceModule::doDisturbanceEvent(DynamicVar n) {
				if (!_runMoss) { return; } //skip if not run moss

				auto& data = n.extract<const DynamicObject>();

				// Get the disturbance type for either historical or last disturbance event.
				std::string disturbanceType = data["disturbance"];
				const auto& dmAssociation = _dmAssociations.find(disturbanceType);

				if (dmAssociation != _dmAssociations.end()) {
					int dmId = dmAssociation->second;

					//this disturbance is applied to the current moss layer
					auto distMatrix = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();


					const auto& it = _matrices.find(dmId);
					if (it != _matrices.end()) {
						const auto& operations = it->second;
						for (const auto& transfer : operations) {
							distMatrix->push_back(CBMDistEventTransfer(transfer));
						}
					}
					else {
						MOJA_LOG_FATAL << "Missing disturbance matrix for ID: " + dmId;
					}
				}
			}

			/**
			 * Clear MossDisturbanceModule_transferRates \n
			 * Add values of "FL2CO2", "FL2CH4", "FL2CO", "FL2FS", "FL2SS", "SL2CO2", "SL2CH4", "SL2CO", 
			 * "SL2FS", "SL2SS", "FF2CO2", "FF2CH4", "FF2CO", "FF2FS", "FF2SS", "SF2CO2", "SF2CH4", "SF2CO", "SF2FS", "SF2SS"
			 * in parameter data to MossDisturbanceModule_transferRates
			 * 
			 * @param data const DynamicObject&
			 * @return void
			 * ************************/
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

			void MossDisturbanceModule::fetchMossDistMatrices() {
				_matrices.clear();
				const auto& transfers = _landUnitData->getVariable("moss_disturbance_matrices")->value()
					.extract<const std::vector<DynamicObject>>();

				for (const auto& row : transfers) {
					auto transfer = CBMDistEventTransfer(*_landUnitData, row);
					int dmId = transfer.disturbanceMatrixId();
					const auto& v = _matrices.find(dmId);
					if (v == _matrices.end()) {
						EventVector vec;
						vec.push_back(transfer);
						_matrices.emplace(dmId, vec);
					}
					else {
						auto& vec = v->second;
						vec.push_back(transfer);
					}
				}
			}

			void MossDisturbanceModule::fetchMossDMAssociations() {
				_dmAssociations.clear();
				const auto& dmAssociations = _landUnitData->getVariable("moss_dm_associations")->value()
					.extract<const std::vector<DynamicObject>>();

				for (const auto& dmAssociation : dmAssociations) {
					std::string distType = dmAssociation["disturbance_type"];
					int dmId = dmAssociation["moss_dm_id"];
					_dmAssociations.insert(std::make_pair(distType, dmId));
				}
			}
		}
	}
}
