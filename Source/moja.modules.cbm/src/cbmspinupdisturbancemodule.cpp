#include "moja/modules/cbm/cbmspinupdisturbancemodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/algorithm/string.hpp> 

namespace moja {
	namespace modules {
		namespace cbm {

			void CBMSpinupDisturbanceModule::configure(const DynamicObject& config) { }

			void CBMSpinupDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMSpinupDisturbanceModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMSpinupDisturbanceModule::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMSpinupDisturbanceModule::onTimingInit, *this);
			}

			void CBMSpinupDisturbanceModule::doLocalDomainInit() {
				fetchMatrices();
				fetchDMAssociations();
				_spu = _landUnitData->getVariable("spatial_unit_id");
			}

			void CBMSpinupDisturbanceModule::doTimingInit() {
				_spuId = _spu->value();
			}

			void CBMSpinupDisturbanceModule::doDisturbanceEvent(DynamicVar n) {
				auto& data = n.extract<const DynamicObject>();

				// Get the disturbance type for either historical or last disturbance event fired in spinup call
				std::string disturbanceType = data["disturbance"];

				bool runPeatland = false;
				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {
					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();
					runPeatland = peatlandId > 0;
				}

				auto disturbanceEvent = _landUnitData->createProportionalOperation();

				if (!runPeatland) {
					// use CBM DM transfers
					auto dmId = _dmAssociations.at(std::make_pair(disturbanceType, _spuId));
					const auto& it = _matrices.find(dmId);

					if (it != _matrices.end()) {
						//valid disturbance matrix is found
						const auto& operations = it->second;
						for (const auto& transfer : operations) {
							auto srcPool = transfer.sourcePool();
							auto dstPool = transfer.destPool();
							if (srcPool != dstPool) {
								disturbanceEvent->addTransfer(srcPool, dstPool, transfer.proportion());
							}
						}
					}
				}
				else {
					// use peatland DM transfers which are injected in peatlandDisturbanceModule
					auto transferVec = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();

					for (const auto& transfer : *transferVec) {
						auto srcPool = transfer.sourcePool();
						auto dstPool = transfer.destPool();
						if (srcPool != dstPool) {
							disturbanceEvent->addTransfer(srcPool, dstPool, transfer.proportion());
						}
					}
				}

				_landUnitData->submitOperation(disturbanceEvent);
			}

			void CBMSpinupDisturbanceModule::fetchMatrices() {
				_matrices.clear();
				const auto& transfers = _landUnitData->getVariable("disturbance_matrices")->value()
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

			void CBMSpinupDisturbanceModule::fetchDMAssociations() {
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
