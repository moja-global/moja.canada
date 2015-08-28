#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/observer.h"

namespace moja {
namespace modules {
namespace CBM {

    void CBMBuildLandUnitModule::configure(const DynamicObject& config) { }

    void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(
            std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(
                *this, &IModule::onLocalDomainInit));

        notificationCenter.addObserver(
            std::make_shared<Observer<IModule, flint::PreTimingSequenceNotification>>(
                *this, &IModule::onPreTimingSequence));
    }

    void CBMBuildLandUnitModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
        _initialAge = _landUnitData->getVariable("initial_age");
        _buildWorked = _landUnitData->getVariable("landUnitBuildSuccess");
    }

    void CBMBuildLandUnitModule::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& /*n*/) {
        bool success = !_initialAge->value().isEmpty();
        // What CBM variables need to be set between land unit simulations?
        // External variables are handled already, just internal variables. State values? Classifiers?
        _buildWorked->set_value(success);
    }

}}} // namespace moja::Modules::CBM
