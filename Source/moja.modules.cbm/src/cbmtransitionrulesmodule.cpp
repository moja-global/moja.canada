#include "moja/modules/cbm/cbmtransitionrulesmodule.h"

#include <moja/flint/ivariable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/logging.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			void CBMTransitionRulesModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMTransitionRulesModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMTransitionRulesModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingShutdown, &CBMTransitionRulesModule::onTimingShutdown, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMTransitionRulesModule::onDisturbanceEvent, *this);
			}

			void CBMTransitionRulesModule::doLocalDomainInit() {
				_gcId = _landUnitData->getVariable("growth_curve_id");
				_spuId = _landUnitData->getVariable("spatial_unit_id");
				_age = _landUnitData->getVariable("age");
				_cset = _landUnitData->getVariable("classifier_set");
				_regenDelay = _landUnitData->getVariable("regen_delay");

				_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
				_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
				_softwoodOther = _landUnitData->getPool("SoftwoodOther");
				_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
				_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

				_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
				_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
				_hardwoodOther = _landUnitData->getPool("HardwoodOther");
				_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
				_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

				if (!_landUnitData->hasVariable("transition_rules") ||
					_landUnitData->getVariable("transition_rules")->value().isEmpty()) {

					return;
				}

				if (_landUnitData->hasVariable("transition_rule_matches")) {
					_transitionRuleMatches = _landUnitData->getVariable("transition_rule_matches");
					_allowMatchingRules = true;
				}

				const auto& transitionRules = _landUnitData->getVariable("transition_rules")->value();
				if (transitionRules.isVector()) {
					for (auto transitionRuleData : transitionRules.extract<std::vector<DynamicObject>>()) {
						TransitionRule rule(transitionRuleData);
						_transitions[rule.id()] = rule;
					}
				}
				else {
					TransitionRule rule(transitionRules.extract<const DynamicObject>());
					_transitions[rule.id()] = rule;
				}

				auto transitionRuleClassifiers = _landUnitData->getVariable("transition_rule_classifiers")->value();
				if (!transitionRuleClassifiers.isEmpty()) {
					if (transitionRuleClassifiers.isVector()) {
						for (auto transitionRule : transitionRuleClassifiers.extract<std::vector<DynamicObject>>()) {
							int id = transitionRule["id"];
							std::string classifierName = transitionRule["classifier_name"];
							std::string classifierValue = transitionRule["classifier_value"];
							_transitions[id].addClassifier(classifierName, classifierValue);
						}
					}
					else {
						int id = transitionRuleClassifiers["id"];
						std::string classifierName = transitionRuleClassifiers["classifier_name"];
						std::string classifierValue = transitionRuleClassifiers["classifier_value"];
						_transitions[id].addClassifier(classifierName, classifierValue);
					}
				}
			}

			void CBMTransitionRulesModule::doTimingInit() {
				_regenDelay->set_value(0);
				_standSpuId = _spuId->value();
			}

			void CBMTransitionRulesModule::doTimingShutdown() {
				_regenDelay->set_value(0);
			}

			int CBMTransitionRulesModule::findTransitionRule(const std::string& disturbanceType) {
				auto& matches = _transitionRuleMatches->value().extract<const DynamicObject>();
				return matches.contains(disturbanceType) ? matches[disturbanceType].extract<int>() : -1;
			}

			void CBMTransitionRulesModule::doDisturbanceEvent(DynamicVar n) {
				auto& data = n.extract<const DynamicObject>();
				int transitionRuleId = -1;
				if (data.contains("transition")) {
					transitionRuleId = data["transition"];
				}

				if (_allowMatchingRules && transitionRuleId == -1) {
					transitionRuleId = findTransitionRule(data["disturbance"]);
				}

				if (transitionRuleId == -1) {
					return;
				}

				if (_transitions.find(transitionRuleId) == _transitions.end()) {
					BOOST_THROW_EXCEPTION(flint::SimulationError()
						<< flint::Details((boost::format("Transition rule ID %1% not found") % transitionRuleId).str())
						<< flint::LibraryName("moja.modules.cbm")
						<< flint::ModuleName(metaData().moduleName)
						<< flint::ErrorCode(0));
				}

				auto transition = _transitions.at(transitionRuleId);
				_regenDelay->set_value(transition.regenDelay());

				auto cset = _cset->value();
				for (auto classifier : transition.classifiers()) {
					if (classifier.second != "?") {
						cset[classifier.first] = classifier.second;
					}
				}

				_cset->set_value(cset);

				auto resetType = transition.resetType();
				int resetAge = transition.resetAge();
				if (resetType == AgeResetType::Absolute && resetAge > -1) {
					_age->set_value(resetAge);
				}
				else if (resetType == AgeResetType::Relative) {
					int currentAge = _age->value();
					int newAge = std::max(0, currentAge + resetAge);
					_age->set_value(newAge);
				}
				else if (resetType == AgeResetType::Yield) {
					int newAge = findYieldCurveAge();
					_age->set_value(newAge);
				}
			}

			int CBMTransitionRulesModule::findYieldCurveAge() {
				// Get the stand growth curve ID associated to the pixel/svo.
				const auto& gcid = _gcId->value();
				auto standGrowthCurveId = gcid.isEmpty() ? -1 : gcid.convert<Int64>();

				// Try to get the stand growth curve and related yield table data from memory.
				bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(
					standGrowthCurveId, _standSpuId);

				if (!carbonCurveFound) {
					//call the stand growth curve factory to create the stand growth curve
					auto standGrowthCurve = _gcFactory->createStandGrowthCurve(
						standGrowthCurveId, _standSpuId, *_landUnitData);

					// Process and convert yield volume to carbon curves.
					_volumeToBioGrowth->generateBiomassCarbonCurve(*standGrowthCurve);
				}

				double standBiomass = calculateBiomass();
				const auto agCarbonCurve = _volumeToBioGrowth->getAboveGroundCarbonCurve(standGrowthCurveId, _standSpuId);
				int matchingAge = agCarbonCurve.size() - 1;
				for (int i = 0; i < agCarbonCurve.size(); i++) {
					if (agCarbonCurve[i] >= standBiomass) {
						matchingAge = i;
						break;
					}
				}

				return matchingAge;
			}

			double CBMTransitionRulesModule::calculateBiomass() {
				double totalAgBiomass = 0.0;
				for (const auto& pool : { _softwoodMerch, _softwoodFoliage, _softwoodOther,
										  _hardwoodMerch, _hardwoodFoliage, _hardwoodOther }) {
					totalAgBiomass += pool->value();
				}

				return totalAgBiomass;
			}

			TransitionRule::TransitionRule(const DynamicObject& data) {
				_id = data["id"];
				_resetAge = data["age"];
				_regenDelay = data["regen_delay"];

				if (!data.contains("reset_type")) {
					_resetType = AgeResetType::Absolute;
				}
				else {
					std::string resetType = data["reset_type"].convert<std::string>();
					if (boost::iequals(resetType, "absolute")) {
						_resetType = AgeResetType::Absolute;
					}
					else if (boost::iequals(resetType, "relative")) {
						_resetType = AgeResetType::Relative;
					}
					else if (boost::iequals(resetType, "yield")) {
						_resetType = AgeResetType::Yield;
					}
					else {
						MOJA_LOG_ERROR << "Invalid age reset type for transition rule: " << resetType;
					}
				}
			}

		}
	}
} // namespace moja::modules::cbm
