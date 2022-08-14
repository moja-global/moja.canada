/**
* @file 
* @brief The brief description goes here.
* 
* The detailed description if any, goes here 
* ******/
#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"

#include <moja/flint/ivariable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    /**
    * @brief Configuration function.
    * 
    * @param config DynamicObject&
    * @return void
    * ************************/
    void CBMLandClassTransitionModule::configure(const DynamicObject& config) { }

    /**
    * @brief Subscribe signals LocalDomainInit,TimingInit and TimingStep.
    * 
    * @param notificationCenter NotificationCenter&
    * @return void
    * ************************/
    void CBMLandClassTransitionModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit	, &CBMLandClassTransitionModule::onLocalDomainInit	, *this);
		notificationCenter.subscribe(signals::TimingInit		, &CBMLandClassTransitionModule::onTimingInit		, *this);
		notificationCenter.subscribe(signals::TimingStep		, &CBMLandClassTransitionModule::onTimingStep		, *this);
	}

    /**
    * @brief Perform at start of simulation.
    * 
    * Initialise a constant variable landClasses as land_class_data variable value. \n
    * If landClasses is a vector, Initialise a constant variable allTransistions as landClasses (vector<DynamicObject>). \n
    * For each constant variable row in allTransistions, assign CBMLandClassTransitionModule._landClassForestStatus[row["land_class"]] as row["is_forest"] and \n
    * CBMLandClassTransitionModule._landClassElapsedTime[row["land_class"]] as row["years_to_permanent"]. \n
    * If not, assign CBMLandClassTransitionModule._landClassForestStatus[landClasses["land_class"]] as landClasses["is_forest"] and \n
    * CBMLandClassTransitionModule._landClassElapsedTime[landClasses["land_class"]] as landClasses["years_to_permanent"]. 
    * 
    * Initialise CBMLandClassTransitionModule._isForest,CBMLandClassTransitionModule._isDecaying,CBMLandClassTransitionModule._historicLandClass, \n
    * CBMLandClassTransitionModule._currentLandClass and CBMLandClassTransitionModule._unfcccLandClass.
    * 
    * if _landUnitData has variable last_pass_disturbance_timeseries, initialise CBMLandClassTransitionModule._lastPassDisturbanceTimeseries.\n
    * Invoke fetchLandClassTransitions().
    * 
    * @return void
    * ************************/
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

    /**
    * @brief Set initial decay status.
    * 
    * Assign CBMLandClassTransitionModule._lastCurrentLandClass as CBMLandClassTransitionModule._currentLandClass value (string). \n
    * Invoke setUnfcccLandClass(). \n
    * Assign CBMLandClassTransitionModule._yearsSinceTransition as 0. \n
    * Initialise bool variable isForest as CBMLandClassTransitionModule._isForest value. \n
    * Initialise variable standCreationDisturbance as getCreationDisturbance(). \n
    * Initialise bool variable deforestedInSpinup as CBMLandClassTransitionModule._landClassTransistions[standCreationDisturbance] !=""&& \n
    * !CBMLandClassTransitionModule._landclassForestStatus[CBMLandClassTransitionModule._landClassTransitions[standCreationDisturbance]]. \n
    * If the carbon in a stand is initially a forest land class and the last_pass disturbance \n
    * is a deforestation event, set CBMLandClassTransitionModule._isDecaying to true. \n
    * if not, set CBMLandClassTransitionModule._isDecaying to false.
    * 
    * @return void
    * ************************/
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
    
    /**
    * @brief doTimingStep
    * 
    * Iterate CBMLandClassTransitionModule.yearsSinceTransition by 1; \n
    * Initialise string varaible currentLandClass as CBMLandClassTransitionModule._currentLandClass value. \n
    * if currentLandClass is equal to CBMLandClassTransitionModule._lastCurrentLandClass, invoke updateRemainigStatus() using currentLandClass as a parameter and \n
    * End program. 
    * 
    * Assign CBMLandClassTransitionModule._historicLandClass as CBMLandClassTransitionModule._landCurrentLandClass. \n
    * Assign CBMLandClassTransitionModule._lastCurrentLandClass as currentLandClass. \n
    * Invoke CBMLandClassTransitionModule.setUnfcccLandClass(); \n
    * Assign CBMLandClassTransitionModule._yearsSinceTransition as 0. \n
    * Set CBMLandClassTransitionModule._isDecaying to true. \n
    * 
    * @return void
    * ************************/
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

    /**
    * @brief getCreationDisturbance.
    * 
    * if CBMLandClassTransitionModule._lastPassDisturbanceTimeseries is not equal to nullptr, \n
    * initialise constant variable lastPassTimeseries as CBMLandClassTransitionModule._lastPassDisturbanceTimeSeries value. \n
    * if lastPassTimeseries is not empty, initialise integer variable maxYear as -1. \n
    * Initialise string variable creationDisturbance. \n
    * For each constant variable event in lastPassTimeseries (vector<DynamicObject>), \n
    * initalise integer variable year as event["year"]. \n
    * if year is greater than maxYear, assign maxYear as year and creationDisturbance as event["disturbance_type"] (string) \n
    * return creationDisturbance. \n
    * else, initialise a constant variable spinup as spinup_parameters value and spinupParams as spinup (DynamicObject).\n
    * return spinupParams["last_pass_disturbance_type"](string). \n
    * 
    * @return string
    * ************************/
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

    /**
    * @brief fetchLandClasstransitions
    * 
    * Initialise constant variable transitions as land_class_transitions value. \n
    * if transitions is a vector, for each constant variable transition in transitions (vector<DynamicObject>). \n
    * Initialise string variables disturbanceType as transition["disturbance_type"] and landClass as transition["land_class_transition"]. \n
    * Invoke make_pair() using disturbanceType and landClass and insert it into CBMLandClassTransitionModule._landClassTransitions. \n
    * if not, initialise string variables disturbanceType as transitions["disturbance_type"] and landClass as transitions["land_class_transition"]. \n
    * Insert disturbanceType,landClass into CBMLandClassTransitionModule._landClassTransitions.
    * 
    * @return void
    * ************************/
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

    /**
    * @brief updateRemainingStatus
    * 
    * Initialise string variable historicLandClass as CBMLandClassTransitionModule._historicLandClass value. \n
    * if landClass is equal to hisitoricLandClass end program. 
    * 
    * Initialise intege variable targetYears as CBMLandClassTransitionModule._landClassElapsedTime[landClass]. \n
    * if CBMLandClassTransitionModule._yearsSinceTransition is greater than targetYears, \n
    * set CBMLandClassTransitionModule._historicLandClass as landClass. \n
    * invoke CBMLandClassTransitionModule.setUnfccLandClass(). \n
    * 
    * @param landClass string
    * @return void
    * ************************/
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

    /**
    * @brief setUnfcccLandClass
    * 
    * Assign string variable currentLandClass as CBMLandClassTransitionModule._currentLandClass, \n
    * CBMLandClassTransitionModule._isForest as CBMLandClassTransitionModule._landClassForestStatus[currentLandClass] \n,
    * CBMLandClassTransitionModule._unfcccLandClass based on CBMLandClassTransitionModule._historicLandClass \n
    * and currentLandClass
    * 
    * @return void
    * ************************/
    void CBMLandClassTransitionModule::setUnfcccLandClass() {
        std::string currentLandClass = _currentLandClass->value();
        _isForest->set_value(_landClassForestStatus[currentLandClass]);

        static std::string landClass = "UNFCCC_%1%_R_%2%";
        _unfcccLandClass->set_value((boost::format(landClass)
            % _historicLandClass->value().convert<std::string>()
            % currentLandClass).str());
    }

}}} // namespace moja::modules::cbm
