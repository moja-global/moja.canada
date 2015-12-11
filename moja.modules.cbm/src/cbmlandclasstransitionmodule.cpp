#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"
#include "moja/observer.h"
#include "moja/logging.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMLandClassTransitionModule::configure(const DynamicObject& config) { }

    void CBMLandClassTransitionModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(
            *this, &IModule::onLocalDomainInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(
            *this, &IModule::onTimingInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(
            *this, &IModule::onTimingStep));
    }

    void CBMLandClassTransitionModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
        const auto& transitions = _landUnitData->getVariable("land_class_transitions")->value()
            .extract<const std::vector<DynamicObject>>();

        for (const auto& row : transitions) {
            _landClassForestStatus[row["land_class"]] = row["is_forest"];
        }

        _landClass = _landUnitData->getVariable("land_class");
        _gcId = _landUnitData->getVariable("growth_curve_id");
    }

    void CBMLandClassTransitionModule::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {
        _previousLandClass = _landClass->value().convert<std::string>();
    }
    
    void CBMLandClassTransitionModule::onTimingStep(const flint::TimingStepNotification::Ptr& /*n*/) {
        std::string landClass = _landClass->value();
        auto isForest = _landClassForestStatus[landClass];
        if (!isForest) {
            _gcId->set_value(-1);
        }
    }

}}} // namespace moja::modules::cbm
