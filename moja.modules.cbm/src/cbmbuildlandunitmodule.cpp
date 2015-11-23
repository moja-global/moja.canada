#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/observer.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMBuildLandUnitModule::configure(const DynamicObject& config) { }

    void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(
            *this, &IModule::onLocalDomainInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::PreTimingSequenceNotification>>(
            *this, &IModule::onPreTimingSequence));
    }

    void CBMBuildLandUnitModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
        _initialAge = _landUnitData->getVariable("initial_age");
        _buildWorked = _landUnitData->getVariable("landUnitBuildSuccess");
        _initialGCID = _landUnitData->getVariable("initial_growth_curve_id");
        _gcid = _landUnitData->getVariable("growth_curve_id");
    }

    void CBMBuildLandUnitModule::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& /*n*/) {
        auto gcid = _initialGCID->value();
        _gcid->set_value(gcid.isEmpty() ? -1 : gcid);
        bool success = !_initialAge->value().isEmpty();
        // What CBM variables need to be set between land unit simulations?
        // External variables are handled already, just internal variables. State values? Classifiers?
        _buildWorked->set_value(success);
    }

}}} // namespace moja::modules::cbm
