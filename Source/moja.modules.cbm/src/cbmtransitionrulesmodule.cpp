/**
* @file
* The CBMTransitionRulesModule module is responsible for transitioning each pixel’s age 
* and classifier set to the same or new values following a disturbance event,
* as well as applying any regeneration delay which will prevent the stand from growing for a number of years after a disturbance.
* ******/
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

			 /**
			 * Subscribe to signals LocalDomainInit,DisturbanceEvent,TimingInit and TimingShutdown.
			 * 
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 * ************************/
			void CBMTransitionRulesModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMTransitionRulesModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMTransitionRulesModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingShutdown, &CBMTransitionRulesModule::onTimingShutdown, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMTransitionRulesModule::onDisturbanceEvent, *this);
			}

			 /**
			 * Initialise CBMTransitionRulesModule._gcId,CBMTransitionRulesModule._spuId,CBMTransitionRulesModule._age, \n
			 * CBMTransitionRulesModule._cset,CBMTransitionRulesModule._regenDelay,CBMTransitionRulesModule._softwoddMerch, \n
			 * CBMTransitionRulesModule._softwoodFoliage,CBMTransitionRulesModule._softwoodOther,CBMTransitionRulesModule._softwoodCoarseRoots, \n
			 * CBMTransitionRulesModule._softwoodFineRoots,CBMTransitionRulesModule._hardwoodMerch,CBMTransitionRulesModule._hardwoodFoliage, \n
			 * CBMTransitionRulesModule._hardwoodOther,CBMTransitionRulesModule._hardwoodCoarseRoots and CBMTransitionRulesModule._hardwoodFineRoots. \n
			 * If _landUnitData has variable "transition_rules_matches", assign CBMTransitionRulesModule._transitionRuleMatches as "transition_rule_matches" in _landUnitData and \n
			 * CBMTransitionRulesModule._allowMatchingRules as true. \n
			 * Assign a constant variable transitionRules as "transition_rules" value in _landUnitData. \n
			 * If transitionRules is a vector, \n
			 * for each transitionRuleData in transitionRules, create a TransitionRule object using transitionRuleData as a parameter and \n
			 * assign the index TransitionRule object Id in CBMTransitionRulesModule._ transitions as the TransitionRule object. \n
			 * else, create a TransitionRule object using a dynamic object of transitionRules and \n
			 * assign the index TransistionRule object Id in CBMTransitionRulesModule._transitions  as the TransitionRule object. \n
			 * Assign a constant variable transitionRuleClassifiers as "transition_rule_classifiers" value in _landUnitData. \n
             * if transitionRuleClassifers is not empty,check if transitionRuleClassifiers is a vector and \n
			 * for each transitionRule in transitionRuleClassifiers, \n
			 * add classifier using "classifer_name" and "classifer_value" in transistionRule to index transitionRule id in CBMTransitionRulesModule._transitions. \n
			 * if transistionRuleClassifiers is not a vector, \n
			 * add classifier using "classifer_name" and "classifer_value" in transistionRuleClassifers to index transitionRuleClassifers id in CBMTransitionRulesModule._transitions.
			 * 
			 * @return void
			 * ************************/
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

			/**
			* Assign CBMTransitionRulesModule._regenDelay value as 0 and \n
			* CBMTransitionRulesModule._standSpuId as CBMTransitionRulesModule._spuId value.
			* 
			* @return void
			* ************************/
			void CBMTransitionRulesModule::doTimingInit() {
				_regenDelay->set_value(0);
				_standSpuId = _spuId->value();
			}

			/**
			* Assign CBMTransitionRulesModule._regenDelay value as 0.
			* 
			* @return void
			* ************************/
			void CBMTransitionRulesModule::doTimingShutdown() {
				_regenDelay->set_value(0);
			}

			/**
			* If CBMTransitionRulesModule._transitionRuleMatches value contains parameter disturbanceType,
			* return the index disturbanceType in CBMTransitionRulesModule._transitionRuleMatches.
			* else return -1.
			* 
			* @param disturbanceType string
			* @return int
			* ************************/
			int CBMTransitionRulesModule::findTransitionRule(const std::string& disturbanceType) {
				auto& matches = _transitionRuleMatches->value().extract<const DynamicObject>();
				return matches.contains(disturbanceType) ? matches[disturbanceType].extract<int>() : -1;
			}

			/**
			* Assign transitionRuleId as -1. \n
			* if parameter n contains "transition", assign transitionRuleId as "transition" of parameter n. \n
			* if CBMTransitionRulesModule._allowMatchingRules and transitionRuleId are both equal to -1, \n
			* Invoke findTransitionRule() using "disturbance" of parameter n as a parameter and assign the value to transitionRuleId.
			* Assign transition as transitionRuleId of CBMTransitionRulesModule._transitions value. \n
		    * Assign CBMTransitionRulesModule._regenDelay value as transition CBMTransitionRulesModule._regenDelay. \n
			* Assign cset as CBMTransitionRulesModule._cset value.
			* For each classifer in transition CBMTransitionRulesModule._classifers, \n
			* if the second element of classifer is not equal to "?", Assign first element of classifer of cset as second element of classifier. \n
			* Assign CBMTransitionRulesModule._cset value as cset. \n
			* Assign variables resetType as CBMTransitionRulesModule._resetType of transition and resetAge as CBMTransitionRulesModule._resetAge of transition. \n
			* if resetAge is equal to AgeResetType::Absolute and resetAge is greater than -1,
			* Assign CBMTransitionRulesModule._age value as resetAge.
			* else if resetAge is equal to AgeResetType::Relative, assign integer variables currentAge as CBMTransitionRulesModule._age value and \n
			* newAge as the max of 0 and sum of currentAge and resetAge. \n
			* Assign CBMTransitionRulesModule._age value as newAge. \n
			* else if resetType is equal to AgeResetType::Yield, invoke findYieldCurveAge() and assign the value to integer variable newAge. \n
			* Assign CBMTransitionRulesModule._age value as newAge.
			* 
			* @exp Simulation Error : Handles error during simulation.
			* @param n DynamicVar
			* @return void
			* ************************/
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

			/**
			* If CBMTransitionRulesModule._gcId is empty, assign variable standGrowthCurveId as - 1, \n
			* else assign standGrowthCurveId as CBMTransitionRulesModule._gcId. \n
			* Assign boolean variable carbonCurveFound as stand growth curve and related yield table from memory. \n
			* If carbonCurveFound is false, Create stand growth curve using standGrowthCurveId, CBMTransitionRulesModule._standSpuId and *_landUnitData. \n
			* Generate Biomass Carbon curve using standGrowthCurve as a parameter. \n
			* Invoke calculateBiomass() and assign the value to standBiomass. \n
			* Invoke getAboveGroundCarbonCurve using standGrowthCurveId and CBMTransitionRulesModule._standSpuId and assign it to agCarbonCurve. \n
			* Assign integer variable matchingAge as the size of agCarbonCurve -1. \n
			* For each iteration in ageCarbonCurve, check if ageCarbonCurve is greater than standBiomass and \n
			* assign matchingAge as the index.
			* return matchingAge.
			* 
			* @return int
			* ************************/
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

			/**
			* Assign double variable totalAgBiomass as 0.0 \n
			* for each pool in the array,add the pool value to totalAgBiomass. \n
			* return totalAgBiomass.
			* 
			* @return double
			* ************************/
			double CBMTransitionRulesModule::calculateBiomass() {
				double totalAgBiomass = 0.0;
				for (const auto& pool : { _softwoodMerch, _softwoodFoliage, _softwoodOther,
										  _hardwoodMerch, _hardwoodFoliage, _hardwoodOther }) {
					totalAgBiomass += pool->value();
				}

				return totalAgBiomass;
			}

			/**
			* Initialise CBMTransitionRulesModule._Id, CBMTransitionRulesModule._resetAge and CBMTransitionRulesModule._regenDelay. \n
			* if parameter data does not contain "reset_type", \n
			* assign CBMTransitionRulesModule._resetType as AgeResetType::Absolute. \n
			* else assign resetType as "reset_type" in parameter data. \n
			* if resetType is equal to "absolute", assign CBMTransitionRulesModule._resetType as AgeResetType::Absoulte. \n
			* else if resetType is equal to "relative", assign CBMTransitionRulesModule._resetType as AgeResetType::Relative. \n
			* else if resetType is equal to "yield", assign CBMTransitionRulesModule._resetType as AgeResetType::Yield. \n
			* else print out a log error.
			* 
			* @param data DynamicObject&
			* @return void
			* ************************/
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
