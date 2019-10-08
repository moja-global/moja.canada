#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"

#include <moja/flint/ivariable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

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

    void CBMLandClassTransitionModule::doLocalDomainInit() {
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
        _isDecaying = _landUnitData->getVariable("is_decaying");
        _historicLandClass = _landUnitData->getVariable("historic_land_class");
        _currentLandClass = _landUnitData->getVariable("current_land_class");
        _unfcccLandClass = _landUnitData->getVariable("unfccc_land_class");

        if (_landUnitData->hasVariable("last_pass_disturbance_timeseries")) {
            _lastPassDisturbanceTimeseries = _landUnitData->getVariable("last_pass_disturbance_timeseries");
        }

        fetchLandClassTransitions();
    }

    void CBMLandClassTransitionModule::doTimingInit() {
        _lastCurrentLandClass = _currentLandClass->value().convert<std::string>();
        setUnfcccLandClass();
        _yearsSinceTransition = 0;

        // Set the initial stand decaying status. The carbon in a stand always decays
        // unless the stand is initially a non-forest land class and the last-pass
        // disturbance was not a deforestation event - i.e. a non-forest stand that
        // will be afforested at some point has its decay paused until then.
        bool isForest = _isForest->value();
        auto standCreationDisturbance = getCreationDisturbance();
        bool deforestedInSpinup = _landClassTransitions[standCreationDisturbance] != ""
            && !_landClassForestStatus[_landClassTransitions[standCreationDisturbance]];

        if (!isForest && !deforestedInSpinup) {
            _isDecaying->set_value(false);
        } else {
            _isDecaying->set_value(true);
        }
    }
    
    void CBMLandClassTransitionModule::doTimingStep() {
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
        _isDecaying->set_value(true);
    }

    std::string CBMLandClassTransitionModule::getCreationDisturbance() {
        // Creation disturbance is either the last in a timeseries:
        if (_lastPassDisturbanceTimeseries != nullptr) {
            const auto& lastPassTimeseries = _lastPassDisturbanceTimeseries->value();
            if (!lastPassTimeseries.isEmpty()) {
                int maxYear = -1;
                std::string creationDisturbance;
                for (const auto& event : lastPassTimeseries.extract<const std::vector<DynamicObject>>()) {
                    int year = event["year"];
                    if (year > maxYear) {
                        maxYear = year;
                        creationDisturbance = event["disturbance_type"].extract<std::string>();
                    }
                }

                return creationDisturbance;
            }
        }

        // Or the usual last pass disturbance type:
        const auto& spinup = _landUnitData->getVariable("spinup_parameters")->value();
        const auto& spinupParams = spinup.extract<DynamicObject>();

        return spinupParams["last_pass_disturbance_type"].convert<std::string>();
    }

    void CBMLandClassTransitionModule::fetchLandClassTransitions() {
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
