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

			void PeatlandDisturbanceModule::configure(const DynamicObject& config) { }

			void PeatlandDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandDisturbanceModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &PeatlandDisturbanceModule::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandDisturbanceModule::onTimingInit, *this);
			}

			void PeatlandDisturbanceModule::doLocalDomainInit() {
				fetchPeatlandDistMatrices();
				fetchPeatlandDMAssociations();
				fetchPeatlandDistModifiers();

				_wtdModifier = _landUnitData->getVariable("peatland_annual_wtd_modifiers");
			}

			void PeatlandDisturbanceModule::doTimingInit() {
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				_runPeatland = _peatlandId > 0;

				//reset water table modifier string for new pixel
				_wtdModifier->reset_value();
			}

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

					//new concept, after the disturbnace, apply the modifier(WTD) up to years
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
