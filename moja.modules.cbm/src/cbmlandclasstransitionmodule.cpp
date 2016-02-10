#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"
#include "moja/observer.h"
#include "moja/logging.h"

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void CBMLandClassTransitionModule::configure(const DynamicObject& config) { }

    void CBMLandClassTransitionModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>	(*this, &IModule::onLocalDomainInit	));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>		(*this, &IModule::onTimingInit		));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>		(*this, &IModule::onTimingStep		));

		notificationCenter.connect_signal(signals::LocalDomainInit	, &CBMLandClassTransitionModule::onLocalDomainInit	, *this);
		notificationCenter.connect_signal(signals::TimingInit		, &CBMLandClassTransitionModule::onTimingInit		, *this);
		notificationCenter.connect_signal(signals::TimingStep		, &CBMLandClassTransitionModule::onTimingStep		, *this);
	}

    void CBMLandClassTransitionModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
        const auto& transitions = _landUnitData->getVariable("land_class_transitions")->value()
            .extract<const std::vector<DynamicObject>>();

        for (const auto& row : transitions) {
            _landClassForestStatus[row["land_class"]] = row["is_forest"];
        }

        _historicLandClass = _landUnitData->getVariable("historic_land_class");
        _currentLandClass = _landUnitData->getVariable("current_land_class");
        _unfcccLandClass = _landUnitData->getVariable("unfccc_land_class");
        _gcId = _landUnitData->getVariable("growth_curve_id");
    }

    void CBMLandClassTransitionModule::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {
        _lastCurrentLandClass = _currentLandClass->value().convert<std::string>();
        setUnfcccLandClass();
        applyForestType();
    }
    
    void CBMLandClassTransitionModule::onTimingStep(const flint::TimingStepNotification::Ptr& /*n*/) {
        std::string currentLandClass = _currentLandClass->value();
        if (currentLandClass == _lastCurrentLandClass) {
            return; // no change in land class since last timestep.
        }

        _historicLandClass->set_value(_lastCurrentLandClass);
        _lastCurrentLandClass = currentLandClass;
        setUnfcccLandClass();
        applyForestType();
    }

    void CBMLandClassTransitionModule::setUnfcccLandClass() {
        static std::string landClass = "UNFCCC_%1%_R_%2%";
        _unfcccLandClass->set_value((boost::format(landClass)
            % _historicLandClass->value().convert<std::string>()
            % _currentLandClass->value().convert<std::string>()).str());
    }

    void CBMLandClassTransitionModule::applyForestType() {
        std::string currentLandClass = _currentLandClass->value();
        auto isForest = _landClassForestStatus[currentLandClass];
        if (!isForest) {
            _gcId->set_value(-1);
        }
    }

}}} // namespace moja::modules::cbm
