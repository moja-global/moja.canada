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
        _cset = _landUnitData->getVariable("classifier_set");
        _initialLandClass = _landUnitData->getVariable("initial_land_class");
        _landClass = _landUnitData->getVariable("land_class");
    }

    void CBMBuildLandUnitModule::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& /*n*/) {
        auto gcid = _initialGCID->value();
        _gcid->set_value(gcid.isEmpty() ? -1 : gcid);
        _landClass->set_value(_initialLandClass->value());

        bool success = !_initialAge->value().isEmpty() && !_cset->value().isEmpty();
        // What CBM variables need to be set between land unit simulations?
        // External variables are handled already, just internal variables. State values? Classifiers?
        _buildWorked->set_value(success);
    }

}}} // namespace moja::modules::cbm
