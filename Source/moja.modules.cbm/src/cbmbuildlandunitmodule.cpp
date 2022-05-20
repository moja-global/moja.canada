/**
 * @file
 * @brief The brief description goes here.
 *
 * The detailed description if any, goes here
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
    * Add all mask variables if mask variables have values.
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
    * @brief Initiate Local Domain.
    *
    * Initialise CBMBuildLandUnitModule._initialAge, CBMBuildLandUnitModule._age, CBMBuildLandUnitModule._buildWorked, CBMBuildLandUnitModule._initialCSet, \n
    * CBMBuildLandUnitModule._cset, CBMBuildLandUnitModule._intialHistoricLandClass, CBMBuildLandUnitModule._initialCurrentLandClass, _historicLandClass \n
    * _currentLandClass and _isForest from _landUnitData \n
    * Add CBMBuildLandUnitModule._initialCSet for the non-peatland run and, 
    * all mask variables to CBMBuildLandUnitModule._maskVars
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
    * Assign variable initialCSet as CBMBuildLandUnitModule._initialCSet, if initialCSet empty, \n
    * check if _landUnitData has variable "peatland_class" \n
    * Assign variable peatlandClass the value of "peatland_class" on _landUnitData, if peatlandClass is empty, \n 
    * assign a false boolean value to CBMBuildLandUnitModule._buildWorked variable and return 
    * 
    * Assign CBMBuildLandUnitModule._cset as initialCSet \n 
    * If the value of each mask variable in CBMBuildLandUnitModule._maskVars is empty, assign a false boolean \n
    * value to _buildWorked variable and return 
    * 
    * Assign variable historicLandClass as CBMBuildLandUnitModule._initialHistoricLandClassvariable \n 
    * and assign historicLandClass to CBMBuildLandUnitModule._historicLandClass variable. 
    * 
    * Assign variable currentLandClass as CBMBuildLandUnitModule._initialCurrentLandClass
    * If currentLandClass is empty, assign  CBMBuildLandUnitModule._currentLandClass as historicLandClass \n
    * else, assign CBMBuildLandUnitModule._currentLandClass as currrentLandClass
    * 
    * If the value of  CBMBuildLandUnitModule._intialAge is empty, assign the number 0 to  CBMBuildLandUnitModule._age \n
    * Assign a true boolean value to  CBMBuildLandUnitModule.._isForest variable. \n
    * Assign a true boolean value to  CBMBuildLandUnitModule.._buildWorked variable.
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
