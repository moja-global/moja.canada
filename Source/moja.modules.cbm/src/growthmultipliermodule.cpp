/**
 * @file
 * This module applies and tracks growth multipliers following disturbance events. Growth multipliers modify the pixel’s carbon increments by a proportion that can 
 * vary over time to represent degradation or enhancement of the stand. Growth multipliers are normally stored in the growth_multiplier_* tables in the GCBM input database
 ********************/
#include "moja/modules/cbm/growthmultipliermodule.h"

#include <moja/flint/ivariable.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * Configuration function
	 * 
	 * Assign value of "debugging_enabled" in parameter config to GrowthMultiplierModule._debuggingEnabled if it exists
	 * 
	 * @param config const DynamicObject&
	 * @return void
	 * *********************/
    void GrowthMultiplierModule::configure(const DynamicObject& config) {
        if (config.contains("debugging_enabled")) {
            _debuggingEnabled = config["debugging_enabled"];
        }
    }

	/**
	 * Subscribe to the signals LocalDomainInit, TimingInit, TimingStep, DisturbanceEvent and TimingShutdown 
	 * 
	 * @param notificationCenter NotificationCenter&
	 * @return void
	 * ************************/
    void GrowthMultiplierModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit,  &GrowthMultiplierModule::onLocalDomainInit,  *this);
		notificationCenter.subscribe(signals::TimingInit,       &GrowthMultiplierModule::onTimingInit,       *this);
		notificationCenter.subscribe(signals::TimingStep,       &GrowthMultiplierModule::onTimingStep,       *this);
		notificationCenter.subscribe(signals::DisturbanceEvent, &GrowthMultiplierModule::onDisturbanceEvent, *this);
		notificationCenter.subscribe(signals::TimingShutdown,   &GrowthMultiplierModule::onTimingShutdown,	 *this);
	}

	/**
	 * If _landUnitData does not have variable "current_growth_multipliers", set GrowthMultiplierModule._moduleEnabled to false and return \n
	 * Else, set GrowthMultiplierModule._moduleEnabled to true. \n
	 * Assign GrowthMultiplierModule._currentGrowthMultipliers value of variable "current_growth_multipliers" in _landUnitData \n
	 * For each multiplier in variable "growth_multipliers" of _landUnitData, if the "disturbance_type" of the multiplier is not found in
	 * GrowthMultiplierModule._growthMultiplierSets, create an object of GrowthMultiplierSet, invoke GrowthMultiplierSet.add() on the object with "forest_type", "time_step" and "multiplier" of the multiplier. \n
	 * Add to GrowthMultiplierModule._growthMultiplierSets, the "disturbance_type" as a key and set it to the GrowthMultiplierModule object created 
	 * 
	 * @return void
	 * ***********************/
	void GrowthMultiplierModule::doLocalDomainInit() {
		if (!_landUnitData->hasVariable("current_growth_multipliers")) {
			_moduleEnabled = false;
			return;
		}

		_moduleEnabled = true;
		_currentGrowthMultipliers = _landUnitData->getVariable("current_growth_multipliers");
		const auto& growthMultiplierData = _landUnitData->getVariable("growth_multipliers")->value();
        if (growthMultiplierData.isVector()) {
            const auto& allMultipliers = growthMultiplierData.extract<const std::vector<DynamicObject>>();
            for (const auto& row : allMultipliers) {
				auto distType = row["disturbance_type"].convert<std::string>();
				auto it = _growthMultiplierSets.find(distType);
				if (it != _growthMultiplierSets.end()) {
					it->second.add(row["forest_type"], row["time_step"], row["multiplier"]);
				} else {
					GrowthMultiplierSet multiplierSet;
					multiplierSet.add(row["forest_type"], row["time_step"], row["multiplier"]);
					_growthMultiplierSets[distType] = multiplierSet;
				}
            }
        } else {
			auto distType = growthMultiplierData["disturbance_type"].convert<std::string>();
			auto it = _growthMultiplierSets.find(distType);
			if (it != _growthMultiplierSets.end()) {
				it->second.add(growthMultiplierData["forest_type"],
							   growthMultiplierData["time_step"],
							   growthMultiplierData["multiplier"]);
			} else {
				GrowthMultiplierSet multiplierSet;
				multiplierSet.add(growthMultiplierData["forest_type"],
								  growthMultiplierData["time_step"],
								  growthMultiplierData["multiplier"]);

				_growthMultiplierSets[distType] = multiplierSet;
			}
		}
    }

	/**
	 * Invoke GrowthMultiplierModule.clearMultipliers()
	 * 
	 * @return void
	 * ********************************/
    void GrowthMultiplierModule::doTimingInit() {
		clearMultipliers();
	}
    
	/**
	 * Invoke GrowthMultiplierModule.clearMultipliers()
	 * 
	 * @return void
	 * ********************************/
	void GrowthMultiplierModule::doTimingShutdown() {
		clearMultipliers();
	}

	/**
	 * If GrowthMultiplierModule._moduleEnabled is false, return \n
	 * Set the value of GrowthMultiplierModule._currentGrowthMultipliers to DynamicVar(), instantiate GrowthMultiplierModule._activeMultiplierSet as an object of GrowthMultiplierSet
	 * 
	 * @return void
	 ***********************/
	void GrowthMultiplierModule::clearMultipliers() {
		if (!_moduleEnabled) {
			return;
		}

		_currentGrowthMultipliers->set_value(DynamicVar());
		_activeMultiplierSet = GrowthMultiplierSet();
	}

	/**
	 * If GrowthMultiplierModule._moduleEnabled is false, return \n
	 * Invoke GrowthMultiplierModule.advanceMultipliers()
	 * 
	 * @return void
	 * **********************/
	void GrowthMultiplierModule::doTimingStep() {
		if (!_moduleEnabled) {
			return;
		}

		advanceMultipliers();
	}
	
	/**
	 * Set the next multiplier in GrowthMultiplierModule._activeMultiplierSet to GrowthMultiplierModule._currentGrowthMultipliers
	 * 
	 * If GrowthMultiplierSet.end() returns false on GrowthMultiplierModule._activeMultiplierSet, set the value of 
	 * GrowthMultiplierModule._currentGrowthMultipliers to GrowthMultiplierSet.next() on GrowthMultiplierModule._activeMultiplierSet. \n
	 * Else, if the value of GrowthMultiplierModule._currentGrowthMultipliers is not empty, set it to DynamicVar()
	 * 
	 * @return void
	 * *********************/
	void GrowthMultiplierModule::advanceMultipliers() {
		if (!_activeMultiplierSet.end()) {
			_currentGrowthMultipliers->set_value(_activeMultiplierSet.next());
		} else {
			if (!_currentGrowthMultipliers->value().isEmpty()) {
				_currentGrowthMultipliers->set_value(DynamicVar());
			}
		}
	}

	/**
	 * If GrowthMultiplierModule._moduleEnabled is false, return. \n
	 * Else if the disturbanceType in parameter e, given by "disturbance" is not found in GrowthMultiplierModule._growthMultiplierSets, 
	 * assign GrowthMultiplierModule._activeMultiplierSet an object of GrowthMultiplierSet, else the GrowthMultiplierSet object corresponding to the disturbanceType in GrowthMultiplierModule._growthMultiplierSets. \n
	 * If GrowthMultiplierModule._debuggingEnabled is true and GrowthMultiplierSet.end() returns false on GrowthMultiplierModule._activeMultiplierSet, print all the growth multipliers
	 * for the current disturbanceType stored in GrowthMultiplierModule._activeMultiplierSet
	 * 
	 * @param e DynamicVar
	 * @return void
	 * *****************************/
	void GrowthMultiplierModule::doDisturbanceEvent(DynamicVar e) {
		if (!_moduleEnabled) {
			return;
		}

		auto& data = e.extract<const DynamicObject>();
		auto distType = data["disturbance"].convert<std::string>();
		auto it = _growthMultiplierSets.find(distType);
		_activeMultiplierSet = it == _growthMultiplierSets.end()
			? GrowthMultiplierSet()
			: it->second;

        if (_debuggingEnabled && !_activeMultiplierSet.end()) {
            MOJA_LOG_DEBUG << "Attached growth multipliers following " << distType
                           << ": " << _activeMultiplierSet.toString();
        }
	}

}}} // namespace moja::modules::cbm
