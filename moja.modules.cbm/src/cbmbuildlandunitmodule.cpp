#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/observer.h"

namespace moja {
	namespace modules {
		namespace cbm {

			void CBMBuildLandUnitModule::configure(const DynamicObject& config) { }

			void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(*this, &IModule::onLocalDomainInit));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::PreTimingSequenceNotification>>(*this, &IModule::onPreTimingSequence));
			}

			void CBMBuildLandUnitModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
				buildWorked = _landUnitData->getVariable("landUnitBuildSuccess");
			}

			void CBMBuildLandUnitModule::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& /*n*/) {
				bool success = true;
				// What CBM variables need to be set between land unit simulations?
				// External variables are handled already, just internal variables. State values? Classifiers?
				buildWorked->set_value(success);
			}
		}
	}
} // namespace moja::modules::cbm
