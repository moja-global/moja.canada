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
			* Invoke fetchPeatlandDistMatrices(),fetchPeatlandDMAssociations() and fetchPeatlandDistModifiers(). \n
			* Assign PeatlandDisturbanceModule._wtdModifier as "peatland_annual_wtd_modifiers" in _landUnitData.
			*
			* @return void
			* **********************************/
			void PeatlandDisturbanceModule::doLocalDomainInit() {
				fetchPeatlandDistMatrices();
				fetchPeatlandDMAssociations();

				_wtdModifier = _landUnitData->getVariable("peatland_annual_wtd_modifiers");
				_wtdModifierYear = _landUnitData->getVariable("peatland_wtd_modifier_year");
			}

			/**
			* Assign PeatlandDisturbanceModule._peatlandId as "peatland_class" in _landUnitData,if not empty else assign it as 1. \n
			* Assign PeatlandDisturbanceModule._runPeatland as PeatlandDisturbanceModule._peatlandId greater than 0. \n
			* Reset PeatlandDisturbanceModule._wtdModifier value.
			*
			* @return void
			* **********************************/
			void PeatlandDisturbanceModule::doTimingInit() {
				_runPeatland = false;

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					_runPeatland = _peatlandId > 0;

					//reset water table modifier ID and modifier year for a new pixel
					_wtdModifier->reset_value();
					_wtdModifierYear->reset_value();
				}
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

					_wtdModifier->set_value(wtdModifierId);
					_wtdModifierYear->set_value(1);

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
			 * Clear PeatlandDisturbanceModule._matrices \n
			 * For each transfer in variable "peatland_disturbance_matrices" in _landUnitData,
			 * create an object of CBMDistEventTransfer with *_landUnitData, transfer, and check if the disturbance matrix id,
			 * is present in PeatlandDisturbanceModule._matrices \n
			 * If it is present, add the value of the disturbance matrix id, transfer to PeatlandDisturbanceModule._matrices,
			 * else the transfer to the iterator resulting from the value of disturbanceId in PeatlandDisturbanceModule._matrices \n
			 *
			 * @return void
			 */
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
			*/
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
		}
	}
}
