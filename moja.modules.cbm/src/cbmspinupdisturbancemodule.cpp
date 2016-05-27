#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/cbmspinupdisturbancemodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMSpinupDisturbanceModule::configure(const DynamicObject& config) { }

    void CBMSpinupDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.connect_signal(signals::LocalDomainInit,  &CBMSpinupDisturbanceModule::onLocalDomainInit,  *this);
        notificationCenter.connect_signal(signals::DisturbanceEvent, &CBMSpinupDisturbanceModule::onDisturbanceEvent, *this);
		notificationCenter.connect_signal(signals::TimingInit,       &CBMSpinupDisturbanceModule::onTimingInit,       *this);
	}

    void CBMSpinupDisturbanceModule::onLocalDomainInit() {
        fetchMatrices();
        fetchDMAssociations();
        _spu = _landUnitData->getVariable("spatial_unit_id");
    }

    void CBMSpinupDisturbanceModule::onTimingInit() {
        _spuId = _spu->value();
    }

    void CBMSpinupDisturbanceModule::onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr n) {
        // Get the disturbance type for either historical or last disturbance event.
        std::string disturbanceType = n->event()["disturbance"];
        auto dmId = _dmAssociations.at(std::make_pair(disturbanceType, _spuId));
        const auto& it = _matrices.find(dmId);
        auto disturbanceEvent = _landUnitData->createProportionalOperation();
        const auto& operations = it->second;
        for (const auto& transfer : operations) {
            auto srcPool = transfer->sourcePool();
            auto dstPool = transfer->destPool();
            if (srcPool != dstPool) {
                disturbanceEvent->addTransfer(srcPool, dstPool, transfer->proportion());
            }
        }

        _landUnitData->submitOperation(disturbanceEvent);
    }

    void CBMSpinupDisturbanceModule::fetchMatrices() {
        _matrices.clear();
        const auto& transfers = _landUnitData->getVariable("disturbance_matrices")->value()
            .extract<const std::vector<DynamicObject>>();

        for (const auto& row : transfers) {
            auto transfer = std::make_shared<CBMDistEventTransfer>(*_landUnitData, row);
            int dmId = transfer->disturbanceMatrixId();
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
