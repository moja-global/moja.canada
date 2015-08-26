#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/observer.h"

namespace moja {
	namespace modules {
		namespace cbm {

			void CBMDisturbanceEventModule::configure(const DynamicObject& config) { }

			void CBMDisturbanceEventModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(*this, &IModule::onLocalDomainInit));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(*this, &IModule::onTimingInit));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(*this, &IModule::onTimingStep));
			}

			void CBMDisturbanceEventModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
				// Pre load every disturbance event for this LocalDomain, cache them in a Map by LandUnitId
				const auto& transfers = _landUnitData->getVariable("AllUniqueDisturbanceEvents")->value()
					.extract<const std::vector<DynamicObject>>();

				// TODO: move this into the Transform for caching! Seems bad form to do module caching, might not work with the Spinup
				for (const auto& row : transfers) {
					auto transfer = std::make_shared<CBMDistEventTransfer>(*_landUnitData, row);
					//event_map_key key(transfer->disturbance_type_id(), transfer->spatial_unit_id());
					event_map_key key = std::make_tuple(transfer->disturbance_type_id(), transfer->spatial_unit_id());

					const auto& v = _events.find(key);
					if (v == _events.end()) {
						event_vector vec;
						vec.push_back(transfer);
						_events.emplace(key, vec );
					}
					else {
						auto& vec = v->second;
						vec.push_back(transfer);
					}
				}
			}

			void CBMDisturbanceEventModule::onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {
				_landUnitEvents.clear();
				// Pre load every disturbance event for this LandUnit, cache them in a Map by Year
				const auto& transfers = _landUnitData->getVariable("EventsForThisLandUnit")->value()
					.extract<const std::vector<DynamicObject>>();
				for (const auto& row : transfers) {
					_landUnitEvents.push_back(CBMDistEventRef(row));
				}
			}

			void CBMDisturbanceEventModule::onPostTimingInit(const flint::TimingPostInitNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onTimingShutdown(const flint::TimingShutdownNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onTimingStep(const flint::TimingStepNotification::Ptr& /*n*/) {
				// Load the LU disturbance event for this time/location and apply the moves defined
				int landUnitId = _landUnitData->getVariable("LandUnitId")->value();

				const auto& timing = _landUnitData->timing();
				for (auto& e : _landUnitEvents) {
					if (e.t_year() == timing->curStartDate().year()) {
						//auto key(e->disturbance_type_id(), e->spatial_unit_id());
						auto key = std::make_tuple(e.disturbance_type_id(), e.spatial_unit_id());

						const auto& it = _events.find(key);
						if (it == _events.end()) {
							// Whoops - seems this is legal
						} else {
							auto& md = metaData();
							md.disturbanceType = e.disturbance_type_id();
							auto disturbanceEvent = _landUnitData->createProportionalOperation();
							const auto& operations = it->second;
							for (const auto& transfer : operations) {
								auto srcPool = transfer->source_pool();
								auto dstPool = transfer->dest_pool();
								if (srcPool != dstPool)
									disturbanceEvent->addTransfer(srcPool, dstPool, transfer->proportion());
							}
							
							_landUnitData->submitOperation(disturbanceEvent);
						}
					}
				}
			}

			void CBMDisturbanceEventModule::onTimingPreEndStep(const flint::TimingPreEndStepNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onTimingEndStep(const flint::TimingEndStepNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onTimingPostStep(const flint::TimingPostStepNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onOutputStep(const flint::OutputStepNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onPostDisturbanceEvent(const flint::PostDisturbanceEventNotification::Ptr& /*n*/) {
			}

			void CBMDisturbanceEventModule::onPostNotification(const flint::PostNotificationNotification::Ptr& /*n*/) {
			}

		}
	}
} // namespace moja::modules::cbm
