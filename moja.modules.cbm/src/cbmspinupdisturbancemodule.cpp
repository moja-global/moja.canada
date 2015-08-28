#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja\modules\cbm\cbmspinupdisturbancemodule.h"
#include "moja\modules\cbm\cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace CBM {

    void CBMSpinupDisturbanceModule::configure(const DynamicObject& config) { }

    void CBMSpinupDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::DisturbanceEventNotification>>(
            *this, &IModule::onDisturbanceEvent));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(
            *this, &IModule::onTimingInit));
    }

    void CBMSpinupDisturbanceModule::onTimingInit(const flint::TimingInitNotification::Ptr&) {
        // The disturbance matrix may be different for each landunit even if it has
        // the same disturbance type ID clear up the spinup events for each land unit.
        _events.clear();

        // Get the disturbance matrix to be applied for historical and last disturbance type.
        const auto& distMatrixInfo = _landUnitData->getVariable("spinup_disturbance_matrix")->value()
            .extract<const std::vector<DynamicObject>>();		
        
        for (const auto& row : distMatrixInfo) {
            auto transfer = std::make_shared<CBMDistEventTransfer>(*_landUnitData, row);

            auto key = transfer->disturbance_type_id();
            const auto& v = _events.find(key);

            if (v == _events.end()) {
                matrix_vector vec;
                vec.push_back(transfer);
                _events.emplace(key, vec);
            }
            else {
                auto& vec = v->second;
                vec.push_back(transfer);
            }
        }
    }

    void CBMSpinupDisturbanceModule::onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr& n) {
        // Get the disturbance type for either historical or last disturbance event.
        int disturbanceType = n->event()["disturbance"];

        const auto& it = _events.find(disturbanceType);	

        if (it == _events.end()) {
            // Whoops - seems this is legal
        }
        else {			
            auto disturbanceEvent = _landUnitData->createProportionalOperation();

            const auto& operations = it->second;
            for (const auto& transfer : operations) {
                auto srcPool = transfer->source_pool();
                auto dstPool = transfer->dest_pool();
                if (srcPool != dstPool) {
                    disturbanceEvent->addTransfer(srcPool, dstPool, transfer->proportion());
                }
            }

            _landUnitData->submitOperation(disturbanceEvent);
        }
    }

}}}