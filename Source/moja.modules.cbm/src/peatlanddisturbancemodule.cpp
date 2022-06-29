#include "moja/modules/cbm/peatlanddisturbancemodule.h"

#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/logging.h>

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/join.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			* Configuration function
			* 
			* @param config DynamicObject&
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::configure(const DynamicObject& config) { }

			/**
			* Subscribe signals LocalDomainInit,DisturbanceEvent and TimingInit
			* 
			* @param notificationCenter NotificationCenter&
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandDisturbanceModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &PeatlandDisturbanceModule::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandDisturbanceModule::onTimingInit, *this);
			}

			/**
			* Invoke fetchPeatLandDistMatrices(),fetchPeatlandDMAssociations() and fetchPeatlandDistModifiers(). \n
			* Assign PeatlandDisturbanceModule._wtdModifier as "peatland_annual_wtd_modifiers" in _landUnitData.
			* 
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::doLocalDomainInit() {
				fetchPeatlandDistMatrices();
				fetchPeatlandDMAssociations();
				fetchPeatlandDistModifiers();

				_wtdModifier = _landUnitData->getVariable("peatland_annual_wtd_modifiers");
			}

			/**
			* Assign PeatlandDisturbanceModule._peatlandId as "peatland_class" in _landUnitData,if not empty else assign it as 1. \n
			* Assign PeatlandDisturbanceModule._runPeatland as PeatlandDisturbanceModule._peatlandId greater than 0. \n
			* Reset PeatlandDisturbanceModule._wtdModifier value.
			* 
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::doTimingInit() {
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				_runPeatland = _peatlandId > 0;

				//reset water table modifier string for new pixel
				_wtdModifier->reset_value();
			}

			/**
			* Assign string variable disturbanceType as "disturbance" in parameter n. \n
			* Find the disturbance module using PeatlandDisturbanceModule._peatlandId and disturbanceType in PeatlandDisturbanceModule._dmAssociations. \n
			* If the disturbance module is not equal to the last value of PeatlandDisturbanceModule._dmAssociations, \n
			* Initialise variables distMatrix as "transfers" in parameter n, \n
			* dmIDandWtdModifer as the second value of the disturbance module, dmId as the first value of dmIDandWtdModifer, \n
			* wtdModifierId as the second value of dmIDandWtdModifier.Find wtdModifierId in PeatlandDisturbanceModule._modifiers and assign the second value to modifierVector variable vec. \n
			* Join vec by "," and assign the value to string variable modifiers. apply the modifier(WTD) up to years. If years is greater than 0, set PeatlandDisturbanceModule._wtdModifier value as modifiers. \n
			* find dmId in PeatlandDisturbanceModule._matrices and assign the value to a constant variable it. \n
			* if it is not equal to the last value of PeatlandDisturbanceModule._matrices \n
			* for each transfer in the second value of it, invoke CBMDistEventTransfer() using transfer and add to distMatrix. \n
			* else print out a log.
			* 
			* @param n DynamicVar
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::doDisturbanceEvent(DynamicVar n) {
				if (!_runPeatland) { return; }

				auto& data = n.extract<const DynamicObject>();

				// Get the disturbance type.
				std::string disturbanceType = data["disturbance"];

				const auto& dmAssociation = _dmAssociations.find(std::make_pair(_peatlandId, disturbanceType));
				if (dmAssociation != _dmAssociations.end()) {

					// this distubance type is applied to the current peatland
					auto distMatrix = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();

					const auto& dmIDandWtdModifer = dmAssociation->second;
					int dmId = dmIDandWtdModifer.first;
					int wtdModifierId = dmIDandWtdModifer.second;
					modifierVector vec = _modifiers.find(wtdModifierId)->second;
					std::string modifiers = boost::algorithm::join(vec, ";");

					//new concept, after the disturbance, apply the modifier(WTD) up to years
					std::string yearStr = modifiers.substr(0, modifiers.find_first_of("_"));
					int years = std::stoi(yearStr);

					if (years > 0) {
						// set modifier only if years value > 0
						_wtdModifier->set_value(modifiers);
					}
					
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
			* Clear PeatlandDisturbanceModule._matrices. \n
			* Initialise constant variable transfers as "peatland_disturbance_matrices" value in _landUnitData. \n
			* For each row in transfers, Initialise transfer as CBMDistEventTransfer() using *_landUnitData and row. \n
			* Invoke disturbanceMatrixId() in transfer and assign the value to  an integer variable dmId. \n
			* Find dmId in PeatlandDisturbanceModule._matrices and assign the value to variable v. \n
			* If v is equal to the last value in PeatlandDisturbanceModule._matrices. \n
			* Initialise EventVector variable vec and add transfer to vec. \n
			* Insert a new element using dmId and vec into PeatlandDisturbanceModule._matrices. \n
			* else, initialise variable vec as the second value of v and add transfer to vec.
			* 
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::fetchPeatlandDistMatrices() {
				_matrices.clear();
				const auto& transfers = _landUnitData->getVariable("peatland_disturbance_matrices")->value()
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
			/**
			* Clear PeatlandDisturbanceModule._dmAssociations. \n
			* For each dmAssociation in "peatland_dm_associations" in _landUnitData, Initialise variables peatlandId as "peatland_id" in dmAssociation, \n
			* distType as "disturbance_type" in dmAssociation, dmId as "peatland_dm_id" in dmAssociation and wtdModifierId as "wtd_modifier_id" in dmAssociation. \n
			* Insert peatlandId,distType,dmId and wtdModifierId into PeatlandDisturbanceModule._dmAssociations.
			* 
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::fetchPeatlandDMAssociations() {
				_dmAssociations.clear();
				const auto& dmAssociations = _landUnitData->getVariable("peatland_dm_associations")->value()
					.extract<const std::vector<DynamicObject>>();

				for (const auto& dmAssociation : dmAssociations) {
					int peatlandId = dmAssociation["peatland_id"];
					std::string distType = dmAssociation["disturbance_type"];
					int dmId = dmAssociation["peatland_dm_id"];
					int wtdModifierId = dmAssociation["wtd_modifier_id"];
					_dmAssociations.insert(std::make_pair(
						std::make_pair(peatlandId, distType), std::make_pair(dmId, wtdModifierId)));
				}
			}

			/**
			* Clear PeatlandDisturbanceModule._modifiers. \n
			* For each row in "peatland_wtd_modifiers" in _landUnitData, initialise integer variables modifierId as "id" in row, \n
			* year as "year" in row,modifier in "modifier" in row. \n
			* Find modifierId in PeatlandDisturbanceModule._modifiers and assign the value to constant variable v. \n
			* If v is equal to the last value of PeatlandDisturbanceModule._modifiers, initialise modifierVector variable vec. \n
			* Separate year and modifier by "_" and add it into vec. \n
			* Insert a new element using modifierId and vec into PeatlandDisturbanceModule._moidifiers. \n
			* Else, initialise variable vec as the second value of v, \n
			* Separate year and modifier by "_" and add it into vec.
			* 
			* @return void
	        * **********************************/
			void PeatlandDisturbanceModule::fetchPeatlandDistModifiers() {
				_modifiers.clear();
				const auto& modifierList = _landUnitData->getVariable("peatland_wtd_modifiers")->value()
					.extract<const std::vector<DynamicObject>>();

				for (const auto& row : modifierList) {
					int modifierId = row["id"];
					int year = row["year"];
					int modifier = row["modifier"];

					const auto& v = _modifiers.find(modifierId);
					if (v == _modifiers.end()) {
						modifierVector vec;
						//each modifier will be recorded as year_modifier
						vec.push_back(std::to_string(year) + "_" + std::to_string(modifier));
						_modifiers.emplace(modifierId, vec);
					}
					else {
						auto& vec = v->second;
						vec.push_back(std::to_string(year) + "_" + std::to_string(modifier));
					}
				}
			}
		}
	}
}
