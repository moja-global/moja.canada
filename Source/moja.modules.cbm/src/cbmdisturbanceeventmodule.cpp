#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/itiming.h>

#include <boost/format.hpp>

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
		_notificationCenter = &notificationCenter;
		notificationCenter.subscribe(signals::LocalDomainInit,	&CBMDisturbanceEventModule::onLocalDomainInit,	*this);
		notificationCenter.subscribe(signals::SystemShutdown,	&CBMDisturbanceEventModule::onSystemShutdown,	*this);
		notificationCenter.subscribe(signals::TimingInit,		&CBMDisturbanceEventModule::onTimingInit,		*this);
		notificationCenter.subscribe(signals::TimingStep,		&CBMDisturbanceEventModule::onTimingStep,		*this);
		notificationCenter.subscribe(signals::DisturbanceEvent,	&CBMDisturbanceEventModule::onDisturbanceEvent,	*this);
	}

    void CBMDisturbanceEventModule::doLocalDomainInit() {
        for (const auto& layerName : _layerNames) {
            _layers.push_back(_landUnitData->getVariable(layerName));
        }

        fetchMatrices();
        fetchDMAssociations();
        fetchLandClassTransitions();
		fetchDistTypeCodes();

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

	void CBMDisturbanceEventModule::doSystemShutdown() {
		for (const auto& layerName : _errorLayers) {
			MOJA_LOG_DEBUG << (boost::format(
				"Disturbance layer '%1%' is not in the expected format. Check if the layer is empty or missing its attribute table."
			) % layerName).str();
		}
	}

    void CBMDisturbanceEventModule::doTimingInit() {
        _landUnitEvents.clear();
        // Pre-load every disturbance event for this land unit.
		for (const auto layer : _layers) {
			const auto& events = layer->value();
			if (events.isEmpty()) {
				continue;
			}

			bool success = true;
			if (events.isVector()) {
				for (const auto& event : events) {
					success = addLandUnitEvent(event);
				}
			}
			else {
				success = addLandUnitEvent(events);
			}

			if (!success) {
				_errorLayers.insert(layer->info().name);
			}
        }
    }

	bool CBMDisturbanceEventModule::addLandUnitEvent(const DynamicVar& ev) {
		if (!ev.isStruct()) {
			return false;
		}

		const auto& event = ev.extract<DynamicObject>();
		std::string disturbanceType = event["disturbance_type"];
		int year = event["year"];

		int spu = _spu->value();
		auto key = std::make_pair(disturbanceType, spu);
		const auto& dm = _dmAssociations.find(key);
		if (dm == _dmAssociations.end()) {
			MOJA_LOG_FATAL << (boost::format(
				"Missing DM association for dist type %1% in SPU %2%")
				% disturbanceType % spu).str();
		}

		auto dmId = dm->second;

		const auto& it = _landClassTransitions.find(disturbanceType);
		std::string landClass = it != _landClassTransitions.end() ? (*it).second : "";

		int transitionId = -1;
		if (event.contains("transition") && !event["transition"].isEmpty()) {
			transitionId = event["transition"];
		}

		_landUnitEvents.push_back(CBMDistEventRef(disturbanceType, dmId, year, transitionId, landClass));
		return true;
	}
    
    void CBMDisturbanceEventModule::doTimingStep() {
        // Load the LU disturbance event for this time/location and apply the moves defined.
        const auto& timing = _landUnitData->timing();
        for (auto& e : _landUnitEvents) {
            if (e.year() == timing->curStartDate().year()) {
				if (e.hasLandClassTransition()) {
					_landClass->set_value(e.landClassTransition());
				}
								
				int disturbanceTypeCode = -1;
				const auto& code = _distTypeCodes.find(e.disturbanceType());
				if (code != _distTypeCodes.end()) {
					disturbanceTypeCode = code->second;
				}

				int dmId = e.disturbanceMatrixId();
                const auto& it = _matrices.find(dmId);

				// Create a vector to store all of the transfers for this event.
				auto distMatrix = std::make_shared<std::vector<CBMDistEventTransfer::Ptr>>();
				const auto& operations = it->second;
				for (const auto& transfer : operations) {
					distMatrix->push_back(transfer);
				}

				DynamicVar data = DynamicObject({
					{ "disturbance", e.disturbanceType() },
					{ "disturbance_type_code", disturbanceTypeCode },
					{ "transfers", distMatrix },
					{ "transition", e.transitionRuleId() }
				});

				// Now fire the disturbance events.
				_notificationCenter->postNotificationWithPostNotification(
					moja::signals::DisturbanceEvent, data);
            }
        }
    }

	void CBMDisturbanceEventModule::doDisturbanceEvent(DynamicVar n) {
		auto& data = n.extract<const DynamicObject>();

		// Get the disturbance type for either historical or last disturbance event.
		std::string disturbanceType = data["disturbance"];
		int disturbanceCode = data["disturbance_type_code"];
        
        Poco::Nullable<int> preDisturbanceAgeClass;
        if (_landUnitData->hasVariable("age_class")) {
            preDisturbanceAgeClass = _landUnitData->getVariable("age_class")->value().extract<int>();
        }

        DynamicVar metadata = DynamicObject({
			{ "disturbance", disturbanceType },
			{ "disturbance_type_code", disturbanceCode },
            { "pre_disturbance_age_class", preDisturbanceAgeClass }
		});

		auto disturbanceEvent = _landUnitData->createProportionalOperation(metadata);
		auto transferVec = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer::Ptr>>>();
		for (const auto& transfer : *transferVec) {
			auto srcPool = transfer->sourcePool();
			auto dstPool = transfer->destPool();
			if (srcPool != dstPool) {
				disturbanceEvent->addTransfer(srcPool, dstPool, transfer->proportion());
			}
		}

		_landUnitData->submitOperation(disturbanceEvent);
		_landUnitData->applyOperations();

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
            } else {
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
        } else {
            std::string disturbanceType = transitions["disturbance_type"];
            std::string landClass = transitions["land_class_transition"];
            _landClassTransitions.insert(std::make_pair(disturbanceType, landClass));
        }
    }

	void CBMDisturbanceEventModule::fetchDistTypeCodes() {
		if (!_landUnitData->hasVariable("disturbance_type_codes")) {
			return;
		}

		const auto& distTypeCodes = _landUnitData->getVariable("disturbance_type_codes")->value();
		if (distTypeCodes.isVector()) {
			for (const auto& code : distTypeCodes.extract<const std::vector<DynamicObject>>()) {
				std::string distType = code["disturbance_type"];
				int distTypeCode = code["disturbance_type_code"];
				_distTypeCodes[distType] = distTypeCode;
			}
		} else {
			std::string distType = distTypeCodes["disturbance_type"];
			int distTypeCode = distTypeCodes["disturbance_type_code"];
			_distTypeCodes[distType] = distTypeCode;
		}
	}

}}} // namespace moja::modules::cbm
