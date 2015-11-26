#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/observer.h"
#include "moja/logging.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMDisturbanceEventModule::configure(const DynamicObject& config) {
        auto layerNames = config["vars"];
        for (const auto& layerName : layerNames) {
            _layerNames.push_back(layerName);
        }
    }

    void CBMDisturbanceEventModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::SystemInitNotification>>(
            *this, &IModule::onSystemInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(
            *this, &IModule::onTimingInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(
            *this, &IModule::onTimingStep));
    }

    void CBMDisturbanceEventModule::onSystemInit(const flint::SystemInitNotification::Ptr&) {
        for (const auto& layerName : _layerNames) {
            _layers.push_back(_landUnitData->getVariable(layerName));
        }

        // Pre-load every disturbance matrix and cache by disturbance type and spatial unit.
        const auto& transfers = _landUnitData->getVariable("disturbance_matrices")->value()
            .extract<const std::vector<DynamicObject>>();

        for (const auto& row : transfers) {
            auto transfer = std::make_shared<CBMDistEventTransfer>(*_landUnitData, row);
            event_map_key key = std::make_tuple(transfer->disturbance_type_id(),
                transfer->spatial_unit_id());

            const auto& v = _matrices.find(key);
            if (v == _matrices.end()) {
                event_vector vec;
                vec.push_back(transfer);
                _matrices.emplace(key, vec);
            }
            else {
                auto& vec = v->second;
                vec.push_back(transfer);
            }
        }

        _gcid = _landUnitData->getVariable("growth_curve_id");
    }

    void CBMDisturbanceEventModule::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {
        _landUnitEvents.clear();

        // Pre load every disturbance event for this LandUnit, cache them in a Map by Year
        for (const auto layer : _layers) {
            const auto& events = layer->value();
            if (events.isEmpty()) {
                continue;
            }

            if (events.isVector()) {
                for (const auto& event : events.extract<std::vector<DynamicObject>>()) {
                    _landUnitEvents.push_back(CBMDistEventRef(event));
                }
            }
            else {
                _landUnitEvents.push_back(CBMDistEventRef(events.extract<DynamicObject>()));
            }
        }

        _spu = _landUnitData->getVariable("spu")->value();
    }
    
    void CBMDisturbanceEventModule::onTimingStep(const flint::TimingStepNotification::Ptr& /*n*/) {
        // Load the LU disturbance event for this time/location and apply the moves defined
        const auto& timing = _landUnitData->timing();
        for (auto& e : _landUnitEvents) {
            if (e.year() == timing->curStartDate().year()) {
                auto key = std::make_tuple(e.disturbance_type_id(), _spu);

                const auto& it = _matrices.find(key);
                if (it == _matrices.end()) {
                    MOJA_LOG_ERROR << "Disturbance matrix not found for disturbance type "
                        << e.disturbance_type_id() << " in SPU " << _spu;
                } else {
                    auto& md = metaData();
                    md.disturbanceType = e.disturbance_type_id();
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
                    if (e.transition() == "Non-Forest") {
                        _gcid->set_value(-1);
                    }
                }
            }
        }
    }

}}} // namespace moja::modules::cbm
