#include "moja/modules/cbm/cbmtransitionrulesmodule.h"
#include "moja/logging.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMTransitionRulesModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit	, &CBMTransitionRulesModule::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit		, &CBMTransitionRulesModule::onTimingInit, *this);
        notificationCenter.subscribe(signals::TimingStep		, &CBMTransitionRulesModule::onTimingStep, *this);
        notificationCenter.subscribe(signals::DisturbanceEvent	, &CBMTransitionRulesModule::onDisturbanceEvent, *this);
    }

    void CBMTransitionRulesModule::onLocalDomainInit() {
        _age = _landUnitData->getVariable("age");
        _cset = _landUnitData->getVariable("classifier_set");
        _regenDelay = _landUnitData->getVariable("regen_delay");

        if (!_landUnitData->hasVariable("transition_rules")) {
            return;
        }

        auto transitionRules = _landUnitData->getVariable("transition_rules")
            ->value().extract<std::vector<DynamicObject>>();

        for (auto transitionRule : transitionRules) {
            int id = transitionRule["id"];
            int age = transitionRule["age"];
            int regenDelay = transitionRule["regen_delay"];
            _transitions[id] = TransitionRule(id, age, regenDelay);
        }

        auto transitionRuleClassifiers = _landUnitData->getVariable("transition_rule_classifiers")
            ->value().extract<std::vector<DynamicObject>>();

        for (auto transitionRule : transitionRuleClassifiers) {
            int id = transitionRule["id"];
            std::string classifierName = transitionRule["classifier_name"];
            std::string classifierValue = transitionRule["classifier_value"];
            _transitions[id].addClassifier(classifierName, classifierValue);
        }
    }

    void CBMTransitionRulesModule::onDisturbanceEvent(Dynamic n) {
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
