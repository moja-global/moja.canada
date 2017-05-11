#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"
#include "moja/logging.h"

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void CBMLandClassTransitionModule::configure(const DynamicObject& config) { }

    void CBMLandClassTransitionModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit	, &CBMLandClassTransitionModule::onLocalDomainInit	, *this);
		notificationCenter.subscribe(signals::TimingInit		, &CBMLandClassTransitionModule::onTimingInit		, *this);
		notificationCenter.subscribe(signals::TimingStep		, &CBMLandClassTransitionModule::onTimingStep		, *this);
	}

    void CBMLandClassTransitionModule::onLocalDomainInit() {
        const auto& landClasses = _landUnitData->getVariable("land_class_data")->value();
        if (landClasses.isVector()) {
            const auto& allTransitions = landClasses.extract<const std::vector<DynamicObject>>();
            for (const auto& row : allTransitions) {
                _landClassForestStatus[row["land_class"]] = row["is_forest"];
                _landClassElapsedTime[row["land_class"]] = row["years_to_permanent"];
            }
        } else {
            _landClassForestStatus[landClasses["land_class"]] =
                landClasses["is_forest"];
            _landClassElapsedTime[landClasses["land_class"]] =
                landClasses["years_to_permanent"];
        }

        _isForest = _landUnitData->getVariable("is_forest");
        _historicLandClass = _landUnitData->getVariable("historic_land_class");
        _currentLandClass = _landUnitData->getVariable("current_land_class");
        _unfcccLandClass = _landUnitData->getVariable("unfccc_land_class");
    }

    void CBMLandClassTransitionModule::onTimingInit() {
        _lastCurrentLandClass = _currentLandClass->value().convert<std::string>();
        setUnfcccLandClass();
        _yearsSinceTransition = 0;
    }
    
    void CBMLandClassTransitionModule::onTimingStep() {
        _yearsSinceTransition++;
        std::string currentLandClass = _currentLandClass->value();
        if (currentLandClass == _lastCurrentLandClass) {
            updateRemainingStatus(currentLandClass);
            return; // no change in land class since last timestep.
        }

        _historicLandClass->set_value(_lastCurrentLandClass);
        _lastCurrentLandClass = currentLandClass;
        setUnfcccLandClass();
        _yearsSinceTransition = 0;
    }

    void CBMLandClassTransitionModule::updateRemainingStatus(std::string landClass) {
        // The 10/20-year "flip" when X_R_Y becomes Y_R_Y, i.e. CL_R_FL -> FL_R_FL.
        std::string historicLandClass = _historicLandClass->value();
        if (landClass == historicLandClass) {
            return;
        }

        int targetYears = _landClassElapsedTime[landClass];
        if (_yearsSinceTransition > targetYears) {
            _historicLandClass->set_value(landClass);
            setUnfcccLandClass();
        }
    }

    void CBMLandClassTransitionModule::setUnfcccLandClass() {
        std::string currentLandClass = _currentLandClass->value();
        _isForest->set_value(_landClassForestStatus[currentLandClass]);

        static std::string landClass = "UNFCCC_%1%_R_%2%";
        _unfcccLandClass->set_value((boost::format(landClass)
            % _historicLandClass->value().convert<std::string>()
            % currentLandClass).str());
    }

}}} // namespace moja::modules::cbm
