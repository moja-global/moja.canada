#include "moja/modules/cbm/growthmultipliermodule.h"
#include "moja/logging.h"

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void GrowthMultiplierModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit,  &GrowthMultiplierModule::onLocalDomainInit,  *this);
		notificationCenter.subscribe(signals::TimingInit,       &GrowthMultiplierModule::onTimingInit,       *this);
		notificationCenter.subscribe(signals::TimingStep,       &GrowthMultiplierModule::onTimingStep,       *this);
		notificationCenter.subscribe(signals::DisturbanceEvent, &GrowthMultiplierModule::onDisturbanceEvent, *this);
		notificationCenter.subscribe(signals::TimingShutdown,   &GrowthMultiplierModule::onTimingShutdown,	 *this);
	}

	void GrowthMultiplierModule::onLocalDomainInit() {
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

    void GrowthMultiplierModule::onTimingInit() {
		clearMultipliers();
	}
    
	void GrowthMultiplierModule::onTimingShutdown() {
		clearMultipliers();
	}

	void GrowthMultiplierModule::clearMultipliers() {
		if (!_moduleEnabled) {
			return;
		}

		_currentGrowthMultipliers->set_value(Dynamic());
		_activeMultiplierSet = GrowthMultiplierSet();
	}

	void GrowthMultiplierModule::onTimingStep() {
		if (!_moduleEnabled) {
			return;
		}

		advanceMultipliers();
	}
	
	void GrowthMultiplierModule::advanceMultipliers() {
		if (!_activeMultiplierSet.end()) {
			_currentGrowthMultipliers->set_value(_activeMultiplierSet.next());
		} else {
			if (!_currentGrowthMultipliers->value().isEmpty()) {
				_currentGrowthMultipliers->set_value(Dynamic());
			}
		}
	}

	void GrowthMultiplierModule::onDisturbanceEvent(Dynamic e) {
		if (!_moduleEnabled) {
			return;
		}

		auto& data = e.extract<const DynamicObject>();
		auto distType = data["disturbance"].convert<std::string>();
		auto it = _growthMultiplierSets.find(distType);
		_activeMultiplierSet = it == _growthMultiplierSets.end()
			? GrowthMultiplierSet()
			: it->second;

		advanceMultipliers();
	}

}}} // namespace moja::modules::cbm
