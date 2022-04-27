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
    * @brief configuration function.
    *
    * This function add the value of the mask variables if the 
    * dynamicobject contains the mask variables.
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
    * @brief subscribe to signal.
    *
    * This function subscribes the signal localDomainInit and PreTimingSequence
    * using the function onLocalDomainInit,onPreTimingSequence respectively.
    * The values are passed and assigned here
    *
    * @param notificationCenter NotificationCenter&
    * @return void
    * ************************/
    void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit,	 &CBMBuildLandUnitModule::onLocalDomainInit,   *this);
		notificationCenter.subscribe(signals::PreTimingSequence, &CBMBuildLandUnitModule::onPreTimingSequence, *this);
	}

    /**
    * @brief  initiate Local domain.
    *
    * This function get the variable from the land unit data and add it to the mask value.
    * if value of enable peatland doesn't exist in the land unit data, initial classifier set
    * is added to the mask value. 
    *
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
    * @brief PreTimingSequence
    *
    * This function set values to the land unit build success,historic land class,
    * current land class, age and is forest variable.
    *
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
