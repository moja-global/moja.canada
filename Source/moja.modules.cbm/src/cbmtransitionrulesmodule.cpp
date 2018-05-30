#include "moja/modules/cbm/cbmtransitionrulesmodule.h"

#include <moja/flint/ivariable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    void CBMTransitionRulesModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit	, &CBMTransitionRulesModule::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit		, &CBMTransitionRulesModule::onTimingInit, *this);
        notificationCenter.subscribe(signals::TimingShutdown	, &CBMTransitionRulesModule::onTimingShutdown, *this);
        notificationCenter.subscribe(signals::DisturbanceEvent	, &CBMTransitionRulesModule::onDisturbanceEvent, *this);
    }

    void CBMTransitionRulesModule::doLocalDomainInit() {
        _age = _landUnitData->getVariable("age");
        _cset = _landUnitData->getVariable("classifier_set");
        _regenDelay = _landUnitData->getVariable("regen_delay");

        if (!_landUnitData->hasVariable("transition_rules") ||
			_landUnitData->getVariable("transition_rules")->value().isEmpty()) {

			return;
        }

		const auto& transitionRules = _landUnitData->getVariable("transition_rules")->value();
		if (transitionRules.isVector()) {
			for (auto transitionRule : transitionRules.extract<std::vector<DynamicObject>>()) {
				int id = transitionRule["id"];
				int age = transitionRule["age"];
				int regenDelay = transitionRule["regen_delay"];
				_transitions[id] = TransitionRule(id, age, regenDelay);
			}
		} else {
			int id = transitionRules["id"];
			int age = transitionRules["age"];
			int regenDelay = transitionRules["regen_delay"];
			_transitions[id] = TransitionRule(id, age, regenDelay);
		}

        auto transitionRuleClassifiers = _landUnitData->getVariable("transition_rule_classifiers")->value();

        if (transitionRuleClassifiers.isVector()) {
            for (auto transitionRule : transitionRuleClassifiers.extract<std::vector<DynamicObject>>()) {
            int id = transitionRule["id"];
            std::string classifierName = transitionRule["classifier_name"];
            std::string classifierValue = transitionRule["classifier_value"];
                _transitions[id].addClassifier(classifierName, classifierValue);
            }
        } else {
            int id = transitionRuleClassifiers["id"];
            std::string classifierName = transitionRuleClassifiers["classifier_name"];
            std::string classifierValue = transitionRuleClassifiers["classifier_value"];
            _transitions[id].addClassifier(classifierName, classifierValue);
        }
    }

	void CBMTransitionRulesModule::doTimingInit() {
		_regenDelay->set_value(0);
	}

	void CBMTransitionRulesModule::doTimingShutdown() {
		_regenDelay->set_value(0);
	}

	void CBMTransitionRulesModule::doDisturbanceEvent(DynamicVar n) {
		auto& data = n.extract<const DynamicObject>();
        if (!data.contains("transition")) {
            return;
        }

        int transitionRuleId = data["transition"];
        if (transitionRuleId < 0) {
            return;
        }

        auto transition = _transitions.at(transitionRuleId);
        _regenDelay->set_value(transition.regenDelay());

        int resetAge = transition.resetAge();
        if (resetAge > -1) {
            _age->set_value(transition.resetAge());
        }
        
        auto cset = _cset->value();
        for (auto classifier : transition.classifiers()) {
            if (classifier.second != "?") {
                cset[classifier.first] = classifier.second;
            }
        }

        _cset->set_value(cset);
    }

}}} // namespace moja::modules::cbm
