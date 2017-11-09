#include "moja/modules/cbm/cbmdisturbancelistener.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ioperation.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/itiming.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void CBMDisturbanceListener::configure(const DynamicObject& config) {
        auto layerNames = config["vars"];
        for (const auto& layerName : layerNames) {
            _layerNames.push_back(layerName);
        }
    }

    void CBMDisturbanceListener::subscribe(NotificationCenter& notificationCenter) {
		_notificationCenter = &notificationCenter;
		notificationCenter.subscribe(signals::LocalDomainInit,	&CBMDisturbanceListener::onLocalDomainInit,	*this);
		notificationCenter.subscribe(signals::SystemShutdown,	&CBMDisturbanceListener::onSystemShutdown,	*this);
		notificationCenter.subscribe(signals::TimingInit,		&CBMDisturbanceListener::onTimingInit,		*this);
		notificationCenter.subscribe(signals::TimingStep,		&CBMDisturbanceListener::onTimingStep,		*this);
	}

    void CBMDisturbanceListener::doLocalDomainInit() {
        for (const auto& layerName : _layerNames) {
            _layers.push_back(_landUnitData->getVariable(layerName));
        }

        fetchMatrices();
        fetchDMAssociations();
        fetchLandClassTransitions();
		fetchDistTypeCodes();

        _landClass = _landUnitData->getVariable("current_land_class");
        _spu = _landUnitData->getVariable("spatial_unit_id");
    }

	void CBMDisturbanceListener::doSystemShutdown() {
		for (const auto& layerName : _errorLayers) {
			MOJA_LOG_DEBUG << (boost::format(
				"Disturbance layer '%1%' is not in the expected format. "
                "Check if the layer is empty or missing its attribute table."
			) % layerName).str();
		}
	}

    void CBMDisturbanceListener::doTimingInit() {
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

	std::string CBMDisturbanceListener::GetDisturbanceTypeName(const DynamicObject& obj) {
		if (obj.contains("disturbance_type")) {
			std::string name = obj["disturbance_type"].extract<std::string>();
			if (obj.contains("disturbance_type_id")) {
				//both id and name have been specified, better check it just in case
				int id = obj["disturbance_type_id"].extract<int>();
				auto match = _distTypeNames.find(id);
				if (match == _distTypeNames.end()) {
					MOJA_LOG_FATAL << (boost::format(
						"specified disturbance type id (%1%) not found")
						% id).str();
				}
				if (match->second != name) {
					MOJA_LOG_FATAL << (boost::format(
						"specified disturbance type id (%1%) does not correspond to specified disturbance type name (%2%)")
						% id % name).str();
				}
			}
			return name;
		}
		else if (obj.contains("disturbance_type_id")) {
			int id = obj["disturbance_type_id"].extract<int>();
			auto match = _distTypeNames.find(id);
			if (match == _distTypeNames.end()) {
				MOJA_LOG_FATAL << (boost::format(
					"specified disturbance type id (%1%) not found")
					% id).str();
			}
			return match->second;
		}
		else {
			MOJA_LOG_FATAL << "disturbance event must specify either name or disturbance id";
		}
		return "";
	}

	bool CBMDisturbanceListener::addLandUnitEvent(const DynamicVar& ev) {
		if (!ev.isStruct()) {
			return false;
		}

		const auto& event = ev.extract<DynamicObject>();

		std::string disturbanceType = GetDisturbanceTypeName(event);
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
    
    void CBMDisturbanceListener::doTimingStep() {
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

    void CBMDisturbanceListener::fetchMatrices() {
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

    void CBMDisturbanceListener::fetchDMAssociations() {
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

    void CBMDisturbanceListener::fetchLandClassTransitions() {
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

	void CBMDisturbanceListener::fetchDistTypeCodes() {

		const auto& distTypeCodes = _landUnitData->getVariable("disturbance_type_codes")->value();
		if (distTypeCodes.isVector()) {
			for (const auto& code : distTypeCodes.extract<const std::vector<DynamicObject>>()) {
				std::string distType = code["disturbance_type"];
				int distTypeCode = code["disturbance_type_code"];
				_distTypeCodes[distType] = distTypeCode;
				_distTypeNames[distTypeCode] = distType;
			}
		} else {
			std::string distType = distTypeCodes["disturbance_type"];
			int distTypeCode = distTypeCodes["disturbance_type_code"];
			_distTypeCodes[distType] = distTypeCode;
			_distTypeNames[distTypeCode] = distType;
		}
	}

}}} // namespace moja::modules::cbm
