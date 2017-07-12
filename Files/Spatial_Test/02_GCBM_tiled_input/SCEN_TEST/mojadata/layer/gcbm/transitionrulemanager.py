import os
import csv

class TransitionRuleManager(object):

    class RuleInstance(object):

        def __init__(self, regen_delay, age_after, classifier_values=None):
            self._regen_delay = regen_delay
            self._age_after = age_after
            self._classifier_values = classifier_values or {}

        def __hash__(self):
            result = 13
            result += 7 * hash(self._regen_delay)
            result += 17 * hash(self._age_after)
            result += 23 * sum(((hash(v) for v in self._classifier_values)))
            return result

        def __eq__(self, other):
            return self._regen_delay == other._regen_delay \
                and self._age_after == other._age_after \
                and self._classifier_values == other._classifier_values

        def __ne__(self, other):
            return not self == other

        @property
        def regen_delay(self):
            return self._regen_delay

        @property
        def age_after(self):
            return self._age_after

        @property
        def classifier_values(self):
            return self._classifier_values.values()

        @property
        def classifier_names(self):
            return self._classifier_values.keys()

    def __init__(self, output_path):
        self._output_path = output_path
        self._transition_rules = {}
        self._next_id = 1

    def get_or_add(self, regen_delay, age_after, classifier_values=None):
        unique_rule = TransitionRuleManager.RuleInstance(regen_delay, age_after, classifier_values)
        id = self._transition_rules.get(unique_rule)
        if not id:
            id = self._next_id
            self._transition_rules[unique_rule] = self._next_id
            self._next_id += 1

        return id

    def write_rules(self):
        if not self._transition_rules:
            return

        rules_dir = os.path.dirname(os.path.abspath(self._output_path))
        if not os.path.exists(rules_dir):
            os.makedirs(rules_dir)

        with open(self._output_path, "wb") as out_file:
            writer = csv.writer(out_file)
            header = ["id", "regen_delay", "age_after"]
            header.extend(self._find_classifier_names())
            writer.writerow(header)
            for rule, id in sorted(self._transition_rules.iteritems(), key=lambda item: item[1]):
                rule_data = [id, rule.regen_delay, rule.age_after]
                rule_data.extend(rule.classifier_values)
                writer.writerow(rule_data)
    
    def _find_classifier_names(self):
        classifier_names = []
        for rule in self._transition_rules.keys():
            if len(rule.classifier_names) > len(classifier_names):
                classifier_names = rule.classifier_names

        return classifier_names
