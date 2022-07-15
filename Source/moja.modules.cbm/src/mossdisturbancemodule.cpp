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

			void MossDisturbanceModule::configure(const DynamicObject& config) {
			}

			void MossDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossDisturbanceModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &MossDisturbanceModule::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossDisturbanceModule::onTimingInit, *this);
			}

			void MossDisturbanceModule::doLocalDomainInit() {
				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {
					fetchMossDistMatrices();
					fetchMossDMAssociations();
				}
			}

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
