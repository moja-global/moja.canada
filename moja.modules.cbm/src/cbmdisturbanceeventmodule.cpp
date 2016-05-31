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
		notificationCenter.connect_signal(signals::LocalDomainInit, &CBMDisturbanceEventModule::onLocalDomainInit, *this);
		notificationCenter.connect_signal(signals::TimingInit, &CBMDisturbanceEventModule::onTimingInit, *this);
		notificationCenter.connect_signal(signals::TimingStep, &CBMDisturbanceEventModule::onTimingStep, *this);
	}

    void CBMDisturbanceEventModule::onLocalDomainInit() {
        for (const auto& layerName : _layerNames) {
            _layers.push_back(_landUnitData->getVariable(layerName));
        }

        fetchMatrices();
        fetchDMAssociations();
        fetchLandClassTransitions();

        _softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
        _softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _softwoodOther = _landUnitData->getPool("SoftwoodOther");
        _softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
        _softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

        _hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
        _hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
        _hardwoodOther = _landUnitData->getPool("HardwoodOther");
        _hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
        _hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

        _landClass = _landUnitData->getVariable("current_land_class");
        _spu = _landUnitData->getVariable("spatial_unit_id");
    }

    void CBMDisturbanceEventModule::onTimingInit() {
        _landUnitEvents.clear();
        int spu = _spu->value();

        // Pre-load every disturbance event for this land unit.
        for (const auto layer : _layers) {
            const auto& events = layer->value();
            if (events.isEmpty()) {
                continue;
            }

            if (events.isVector()) {
                for (const auto& event : events.extract<std::vector<DynamicObject>>()) {
                    std::string disturbanceType = event["disturbance_type"];
                    int year = event["year"];
                    auto dmId = _dmAssociations.at(std::make_pair(disturbanceType, spu));
                    
                    const auto& it = _landClassTransitions.find(disturbanceType);
                    std::string landClass = it != _landClassTransitions.end() ? (*it).second : "";

                    _landUnitEvents.push_back(CBMDistEventRef(dmId, year, landClass));
                }
            }
            else {
                std::string disturbanceType = events["disturbance_type"];
                int year = events["year"];
                auto dmId = _dmAssociations.at(std::make_pair(disturbanceType, spu));

                const auto& it = _landClassTransitions.find(disturbanceType);
                std::string landClass = it != _landClassTransitions.end() ? (*it).second : "";

                _landUnitEvents.push_back(CBMDistEventRef(dmId, year, landClass));
            }
        }
    }
    
    void CBMDisturbanceEventModule::onTimingStep() {
        // Load the LU disturbance event for this time/location and apply the moves defined
        const auto& timing = _landUnitData->timing();
        for (auto& e : _landUnitEvents) {
            if (e.year() == timing->curStartDate().year()) {
                int dmId = e.disturbanceMatrixId();
                const auto& it = _matrices.find(dmId);
                auto& md = metaData();
                md.disturbanceType = dmId;
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
                _landUnitData->applyOperations();

                if (e.hasLandClassTransition()) {
                    _landClass->set_value(e.landClassTransition());
                }

                double totalBiomass = _hardwoodCoarseRoots->value()
                    + _hardwoodFineRoots->value() + _hardwoodFoliage->value()
                    + _hardwoodMerch->value() + _hardwoodOther->value()
                    + _softwoodCoarseRoots->value() + _softwoodFineRoots->value()
                    + _softwoodFoliage->value() + _softwoodMerch->value()
                    + _softwoodOther->value();

                if (totalBiomass < 0.001) {
                    _landUnitData->getVariable("age")->set_value(0);
                }
            }
        }
    }

    void CBMDisturbanceEventModule::fetchMatrices() {
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

    void CBMDisturbanceEventModule::fetchDMAssociations() {
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

    void CBMDisturbanceEventModule::fetchLandClassTransitions() {
        const auto& transitions = _landUnitData->getVariable("land_class_transitions")->value();
        if (transitions.isVector()) {
            for (const auto& transition : transitions.extract<const std::vector<DynamicObject>>()) {
                std::string disturbanceType = transition["disturbance_type"];
                std::string landClass = transition["land_class_transition"];
                _landClassTransitions.insert(std::make_pair(disturbanceType, landClass));
            }
        }
        else {
            std::string disturbanceType = transitions["disturbance_type"];
            std::string landClass = transitions["land_class_transition"];
            _landClassTransitions.insert(std::make_pair(disturbanceType, landClass));
        }
    }

}}} // namespace moja::modules::cbm
