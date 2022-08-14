/**
 * @file
 * @brief The CBMBuildLandUnitModule sets up the initial state of each pixel – the age (or -1 if 
 * null, indicating non-forest), the starting classifier set, and the value of the 
 * landUnitBuildSuccess variable.
 * The module checks the value of the landUnitBuildSuccess variable to determine 
 * whether to simulate a pixel or not. A pixel can only be simulated if the 
 * CBMBuildLandUnitModule module finds a non-null classifier set for the pixel (individual 
 * classifiers may be null, as long as at least one in the set has a value), and none of the 
 * variables in the user-defined mask have a null value
 * ******/
#include "moja/modules/cbm/cbmbuildlandunitmodule.h"

#include <moja/flint/ivariable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    /**
    * @brief Configuration function
    * 
    * Add all mask variables to CBMBuildLandUnitModule._maskVarNames if parameter config has variable "mask_vars" 
    *
    * @param config DynamicObject&
    * @return void
    * ************************/
    void CBMBuildLandUnitModule::configure(const DynamicObject& config) {
		// Mask IN: a pixel is simulated if all mask variables have values.
		if (config.contains("mask_vars")) {
			for (const auto& varName : config["mask_vars"]) {
				_maskVarNames.push_back(varName);
			}
		}
	}

    /**
    * @brief Subscribe the signal LocalDomainInit and PreTimingSequence.
    * 
    * @param notificationCenter NotificationCenter&
    * @return void
    * ************************/
    void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit,	 &CBMBuildLandUnitModule::onLocalDomainInit,   *this);
		notificationCenter.subscribe(signals::PreTimingSequence, &CBMBuildLandUnitModule::onPreTimingSequence, *this);
	}

    /**
    * @brief Perform at the start of the simulation
    *
    * Initialise CBMBuildLandUnitModule._initialAge, CBMBuildLandUnitModule._age, CBMBuildLandUnitModule._buildWorked, CBMBuildLandUnitModule._initialCSet, \n
    * CBMBuildLandUnitModule._cset, CBMBuildLandUnitModule._intialHistoricLandClass, CBMBuildLandUnitModule._initialCurrentLandClass, _historicLandClass \n
    * _currentLandClass and _isForest from _landUnitData \n
    * Add CBMBuildLandUnitModule._initialCSet for the non-peatland run and all mask variables to CBMBuildLandUnitModule._maskVars
    * 
    * @return void
    * ************************/

    void CBMBuildLandUnitModule::doLocalDomainInit() {
        _initialAge = _landUnitData->getVariable("initial_age");
        _age = _landUnitData->getVariable("age");
        _buildWorked = _landUnitData->getVariable("landUnitBuildSuccess");
        _initialCSet = _landUnitData->getVariable("initial_classifier_set");
        _cset = _landUnitData->getVariable("classifier_set");
        _initialHistoricLandClass = _landUnitData->getVariable("initial_historic_land_class");
        _initialCurrentLandClass = _landUnitData->getVariable("initial_current_land_class");
        _historicLandClass = _landUnitData->getVariable("historic_land_class");
        _currentLandClass = _landUnitData->getVariable("current_land_class");
        _isForest = _landUnitData->getVariable("is_forest");  
      

        if (!_landUnitData->getVariable("enable_peatland")->value()) {
            //for non-peatland run, add initial classifier set as mask value
            _maskVars.push_back(_initialCSet);            
        }

		for (const auto& varName : _maskVarNames) {
			_maskVars.push_back(_landUnitData->getVariable(varName));
		}
    }

    /**
    * @brief Run before start of simulation
    * 
    * If CBMBuildLandUnitModule._initialCSet is empty, _landUnitData has the variable "peatland_class" and it is empty, \n 
    * assign a false boolean value to CBMBuildLandUnitModule._buildWorked and return \n
    * Else assign CBMBuildLandUnitModule._cset the value of CBMBuildLandUnitModule._initialCSet \n 
    * If the value of each mask variable in CBMBuildLandUnitModule._maskVars is empty, assign a false boolean \n
    * value to CBMBuildLandUnitModule._buildWorked and return 
    * 
    * Assign CBMBuildLandUnitModule._historicLandClass the value of CBMBuildLandUnitModule._initialHistoricLandClass \n
    * CBMBuildLandUnitModule._currentLandClass the value of CBMBuildLandUnitModule._initialCurrentLandClass if it is not empty, \n
    * else to CBMBuildLandUnitModule._historicLandClass

    * If the value of  CBMBuildLandUnitModule._intialAge is empty, assign the number 0 to CBMBuildLandUnitModule._age \n
    * Assign a true boolean value to  CBMBuildLandUnitModule._isForest and to CBMBuildLandUnitModule._buildWorked
    * 
    * @return void
    * ************************/

    void CBMBuildLandUnitModule::doPreTimingSequence() {
        auto initialCSet = _initialCSet->value();
		if (initialCSet.isEmpty()) {
            if (_landUnitData->hasVariable("peatland_class")) {
                auto peatlandClass = _landUnitData->getVariable("peatland_class")->value();
                if (peatlandClass.isEmpty()) {
                   _buildWorked->set_value(false);
                   return;
                }
            } 
		}

        _cset->set_value(initialCSet);

        for (const auto var : _maskVars) {
            if (var->value().isEmpty()) {
                _buildWorked->set_value(false);
                return;
            }
        }

        auto historicLandClass = _initialHistoricLandClass->value();
        _historicLandClass->set_value(historicLandClass);

        auto currentLandClass = _initialCurrentLandClass->value();
        if (currentLandClass.isEmpty()) {
            _currentLandClass->set_value(historicLandClass);
        } else {
            _currentLandClass->set_value(currentLandClass);
        }

        /*
        TODO: This is broken right now, but needs to be fixed at some point if
              disabling spinup is ever going to work as intended - otherwise nothing
              will be setting the initial age of pixels, and the previous pixel's
              final age will be the current pixel's starting age.

        if (!_landUnitData->config()->spinup()->enabled()) {
            _age->set_value(_initialAge->value());
        }
        */

        if (_initialAge->value().isEmpty()) {
            _age->set_value(0);
        }

        // Pixels are always reset to forested; growth module needs this in order to run
        // in spinup. CBMLandClassTransitionModule takes care of the real initial isForest
        // setup.
        _isForest->set_value(true);

        _buildWorked->set_value(true);
    }

}}} // namespace moja::modules::cbm
