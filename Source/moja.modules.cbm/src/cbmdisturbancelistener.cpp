#include "moja/modules/cbm/cbmdisturbancelistener.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/itiming.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			void CBMDisturbanceListener::configure(const DynamicObject& config) {
				auto layerNames = config["vars"];
				if (layerNames.size() == 0 || layerNames.isEmpty()) {
					return;
				}

				for (const auto& layerName : layerNames) {
					_layerNames.push_back(layerName);
				}

				_conditionConfig = config.contains("conditions") ? config["conditions"] : DynamicVar();
			}

			void CBMDisturbanceListener::subscribe(NotificationCenter& notificationCenter) {
				_notificationCenter = &notificationCenter;
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMDisturbanceListener::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::SystemShutdown, &CBMDisturbanceListener::onSystemShutdown, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMDisturbanceListener::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &CBMDisturbanceListener::onTimingStep, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMDisturbanceListener::onDisturbanceEvent, *this);
			}

			void CBMDisturbanceListener::doLocalDomainInit() {
				for (const auto& layerName : _layerNames) {
					_layers.push_back(_landUnitData->getVariable(layerName));
				}

				fetchMatrices();
				fetchDMAssociations();
				fetchLandClassTransitions();
				fetchDistTypeCodes();
				fetchPeatlandDMAssociations();
				fetchDisturbanceOrder();

				_landClass = _landUnitData->getVariable("current_land_class");
				_spu = _landUnitData->getVariable("spatial_unit_id");
				_classifierSet = _landUnitData->getVariable("classifier_set");
				_age = _landUnitData->getVariable("age");
			}

			void CBMDisturbanceListener::doSystemShutdown() {
				for (const auto& layerName : _errorLayers) {
					MOJA_LOG_DEBUG << (boost::format(
						"Disturbance layer '%1%' is not in the expected format. "
						"Check if the layer is empty or missing its attribute table."
					) % layerName).str();
				}
			}

			void CBMDisturbanceListener::doDisturbanceEvent(DynamicVar n) {
				const auto& timing = _landUnitData->timing();
				auto year = timing->curStartDate().year();

				auto& data = n.extract<const DynamicObject>();
				std::string disturbanceType = data["disturbance"];

				_disturbanceHistory->emplace_front(DisturbanceHistoryRecord{
					disturbanceType, year, _age->value() });
			}

			void CBMDisturbanceListener::doTimingInit() {
				_disturbanceHistory->clear();

				if (_classifierNames.empty()) {
					const auto& cset = _classifierSet->value().extract<DynamicObject>();
					for (const auto& key : cset) {
						_classifierNames.emplace(key.first);
					}
				}

				// Initialize disturbance conditions from configuration if needed.
				if (!_disturbanceConditionsInitialized && _conditionConfig.size() > 0) {
					for (const auto& conditionConf : _conditionConfig) {
						auto condition = conditionConf.extract<DynamicObject>();

						std::vector<std::string> matchDisturbanceTypes;
						auto disturbanceTypes = condition["disturbance_type"];
						if (disturbanceTypes.isVector()) {
							for (const auto& disturbanceType : disturbanceTypes) {
								matchDisturbanceTypes.push_back(disturbanceType.convert<std::string>());
							}
						}
						else {
							matchDisturbanceTypes.push_back(disturbanceTypes.convert<std::string>());
						}

						auto overrideDisturbanceType = condition.contains("override_disturbance_type") ?
							condition["override_disturbance_type"] : "";

						std::vector<std::shared_ptr<IDisturbanceSubCondition>> matchConditions;
						std::vector<std::shared_ptr<IDisturbanceSubCondition>> runConditions;
						std::vector<std::shared_ptr<IDisturbanceSubCondition>> overrideConditions;

						auto matchCondition = createSubCondition(condition);
						matchConditions.push_back(matchCondition);

						if (condition.contains("run_conditions")) {
							for (const auto& runConditionConf : condition["run_conditions"]) {
								auto runCondition = runConditionConf.extract<DynamicObject>();
								auto condition = createSubCondition(runCondition);
								runConditions.push_back(condition);
							}
						}

						if (condition.contains("override_conditions")) {
							for (const auto& overrideConditionConf : condition["override_conditions"]) {
								auto overrideCondition = overrideConditionConf.extract<DynamicObject>();
								auto condition = createSubCondition(overrideCondition);
								overrideConditions.push_back(condition);
							}
						}

						_disturbanceConditions.push_back(DisturbanceCondition(
							matchDisturbanceTypes, matchConditions, runConditions, overrideConditions,
							overrideDisturbanceType));
					}

					_disturbanceConditionsInitialized = true;
				}

				_landUnitEvents.clear();
				// Pre-load every disturbance event for this land unit.
				for (const auto layer : _layers) {
					const auto& events = layer->value();
					if (events.isEmpty()) {
						continue;
					}

					bool success = true;
					if (events.isVector()) {
						for (const auto& event : events.extract<const std::vector<DynamicObject>>()) {
							success = addLandUnitEvent(event);
						}
					}
					else {
						success = addLandUnitEvent(events);
					}

					if (!success) {
						_errorLayers.insert(layer->info().name);
					}
				}

				for (auto& eventYear : _landUnitEvents) {
					std::stable_sort(
						eventYear.second.begin(), eventYear.second.end(),
						[this](const CBMDistEventRef& first, const CBMDistEventRef& second
							) {
								if (_disturbanceOrder.find(first.disturbanceType()) == _disturbanceOrder.end()) {
									return false;
								}

								return _disturbanceOrder[first.disturbanceType()] < _disturbanceOrder[second.disturbanceType()];
						});
				}
			}

			std::string CBMDisturbanceListener::getDisturbanceTypeName(const DynamicObject& eventData) {
				if (eventData.contains("disturbance_type")) {
					std::string name = eventData["disturbance_type"].extract<std::string>();
					if (eventData.contains("disturbance_type_id")) {
						// Both id and name have been specified, better check it just in case.
						int id = eventData["disturbance_type_id"].extract<int>();
						auto match = _distTypeNames.find(id);
						if (match == _distTypeNames.end()) {
							MOJA_LOG_FATAL << (boost::format(
								"specified disturbance type id (%1%) not found")
								% id).str();
						}
						if (match->second != name) {
							MOJA_LOG_FATAL << (boost::format(
								"specified disturbance type id (%1%) does not correspond to specified disturbance type name (%2%)")
								% id % name).str();
						}
					}
					return name;
				}
				else if (eventData.contains("disturbance_type_id")) {
					int id = eventData["disturbance_type_id"];
					auto match = _distTypeNames.find(id);
					if (match == _distTypeNames.end()) {
						MOJA_LOG_FATAL << (boost::format(
							"specified disturbance type id (%1%) not found")
							% id).str();
					}
					return match->second;
				}

				MOJA_LOG_FATAL << "disturbance event must specify either name or disturbance id";
				return "";
			}

			bool CBMDisturbanceListener::addLandUnitEvent(const DynamicVar& eventData) {
				if (!eventData.isStruct()) {
					return false;
				}

				const auto& event = eventData.extract<DynamicObject>();

				auto disturbanceType = getDisturbanceTypeName(event);
				int year = event["year"];

				int transitionId = -1;
				if (event.contains("transition") && !event["transition"].isEmpty()) {
					transitionId = event["transition"];
				}

				std::vector<std::shared_ptr<IDisturbanceSubCondition>> conditions;
				if (event.contains("conditions") && !event["conditions"].isEmpty()) {
					for (const auto& condition : event["conditions"]) {
						std::string varName = condition[0];
						auto targetType = condition[1] == "<" ? DisturbanceConditionType::LessThan
							: condition[1] == ">=" ? DisturbanceConditionType::AtLeast
							: DisturbanceConditionType::EqualTo;
						DynamicVar target = condition[2];

						if (_classifierNames.find(varName) != _classifierNames.end()) {
							conditions.push_back(std::make_shared<VariableDisturbanceSubCondition>(
								_classifierSet, targetType, target, varName));
						}
						else {
							conditions.push_back(std::make_shared<VariableDisturbanceSubCondition>(
								_landUnitData->getVariable(varName), targetType, target));
						}
					}
				}

				_landUnitEvents[year].push_back(CBMDistEventRef(
					disturbanceType, year, transitionId, conditions, event));

				return true;
			}

			void CBMDisturbanceListener::doTimingStep() {
				// Load the LU disturbance event for this time/location and apply the moves defined.
				const auto& timing = _landUnitData->timing();
				auto currentYear = timing->curStartDate().year();

				for (auto& e : _landUnitEvents[currentYear]) {
					if (!e.checkConditions()) {
						MOJA_LOG_DEBUG << (boost::format("Conditions not met for %1% in %2% - skipped")
							% e.disturbanceType() % currentYear).str();
						continue;
					}

					DisturbanceConditionResult result;
					for (auto& condition : _disturbanceConditions) {
						if (!condition.isApplicable(e.disturbanceType())) {
							continue;
						}

						// Apply the first matching condition from each category (run/override),
						// i.e. conditions are prioritized in the order they're configured.
						result.merge(condition.check());
					}

					if (result.hadRunConditions && !result.shouldRun) {
						MOJA_LOG_DEBUG << (boost::format("Conditions not met for %1% in %2% - skipped")
							% e.disturbanceType() % currentYear).str();
						continue;
					}

					if (result.newDisturbanceType != "") {
						e.setDisturbanceType(result.newDisturbanceType);
					}

					int disturbanceTypeCode = -1;
					const auto& code = _distTypeCodes.find(e.disturbanceType());
					if (code != _distTypeCodes.end()) {
						disturbanceTypeCode = code->second;
					}

					// Check if running on peatland.
					bool runPeatland = false;
					int peatlandId = -1;
					if (_landUnitData->hasVariable("enable_peatland") &&
						_landUnitData->getVariable("enable_peatland")->value().extract<bool>()) {
						auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
						peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();
						runPeatland = peatlandId > 0;
					}

					if (!runPeatland) {
						fireCBMDisturbanceEvent(e);
					}
					else {
						std::string disturbanceType = e.disturbanceType();

						//check if the disturbance is applied in this peatland
						const auto& dmAssociation = _peatlandDmAssociations.find(std::make_pair(peatlandId, disturbanceType));
						if (dmAssociation != _peatlandDmAssociations.end()) {
							//disturbance is applied in this peatland, fire peatland disturbance event
							firePeatlandDisturbanceEvent(e);
						}
						else {
							//disturbance is not applied to peatland, fire regular CBM disturbance event
							fireCBMDisturbanceEvent(e);
						}
					}
				}
			}

			void CBMDisturbanceListener::firePeatlandDisturbanceEvent(CBMDistEventRef& e) {
				int disturbanceTypeCode = -1;
				const auto& code = _distTypeCodes.find(e.disturbanceType());
				if (code != _distTypeCodes.end()) {
					disturbanceTypeCode = code->second;
				}

				// Check if the disturbance transitions to a new land class.
				const auto& it = _landClassTransitions.find(e.disturbanceType());
				std::string landClassTransition = it != _landClassTransitions.end() ? (*it).second : "";

				if (landClassTransition != "") {
					_landClass->set_value(landClassTransition);
				}

				auto distMatrix = std::make_shared<std::vector<CBMDistEventTransfer>>();

				//if disturbance is applied in this peatland, prepare the event data object
				//disturbance matrix will be injected by peatland disturbance module
				auto data = DynamicObject({
						{ "disturbance", e.disturbanceType() },
						{ "disturbance_type_code", disturbanceTypeCode },
						{ "transfers", distMatrix },
						{ "transition", e.transitionRuleId() }
					});

				// Merge any additional metadata into disturbance data.
				for (const auto& item : e.metadata()) {
					if (!data.contains(item.first)) {
						data[item.first] = item.second;
					}
				}
				// Now fire the disturbance events.
				_notificationCenter->postNotificationWithPostNotification(
					moja::signals::DisturbanceEvent, (DynamicVar)data);
			}

			void CBMDisturbanceListener::fireCBMDisturbanceEvent(CBMDistEventRef& e) {
				// Find the disturbance matrix for the disturbance type/SPU.
				int spu = _spu->value();
				auto key = std::make_pair(e.disturbanceType(), spu);
				const auto& dm = _dmAssociations.find(key);
				if (dm == _dmAssociations.end()) {
					MOJA_LOG_FATAL << (boost::format(
						"Missing DM association for dist type %1% in SPU %2% - skipped")
						% e.disturbanceType() % spu).str();
					return;
				}

				auto dmId = dm->second;

				// Check if the disturbance transitions to a new land class.
				const auto& it = _landClassTransitions.find(e.disturbanceType());
				std::string landClassTransition = it != _landClassTransitions.end() ? (*it).second : "";

				if (landClassTransition != "") {
					_landClass->set_value(landClassTransition);
				}

				// Find the disturbance type code.
				int disturbanceTypeCode = -1;
				const auto& code = _distTypeCodes.find(e.disturbanceType());
				if (code != _distTypeCodes.end()) {
					disturbanceTypeCode = code->second;
				}

				auto distMatrix = std::make_shared<std::vector<CBMDistEventTransfer>>();
				{
					const auto& it = _matrices.find(dmId);
					const auto& operations = it->second;
					for (const auto& transfer : operations) {
						distMatrix->push_back(CBMDistEventTransfer(transfer));
					}
				}

				auto data = DynamicObject({
						{ "disturbance", e.disturbanceType() },
						{ "disturbance_type_code", disturbanceTypeCode },
						{ "transfers", distMatrix },
						{ "transition", e.transitionRuleId() }
					});

				// Merge any additional metadata into disturbance data.
				for (const auto& item : e.metadata()) {
					if (!data.contains(item.first)) {
						data[item.first] = item.second;
					}
				}

				// Now fire the disturbance events.
				_notificationCenter->postNotificationWithPostNotification(
					moja::signals::DisturbanceEvent, (DynamicVar)data);
			}

			void CBMDisturbanceListener::fetchMatrices() {
				_matrices.clear();
				const auto& transfers = _landUnitData->getVariable("disturbance_matrices")->value()
					.extract<const std::vector<DynamicObject>>();

				for (const auto& row : transfers) {
					auto transfer = CBMDistEventTransfer(*_landUnitData, row);
					int dmId = transfer.disturbanceMatrixId();
					const auto& v = _matrices.find(dmId);
					if (v == _matrices.end()) {
						EventVector vec;
						vec.push_back(transfer);
						_matrices.emplace(dmId, vec);
					}
					else {
						auto& vec = v->second;
						vec.push_back(transfer);
					}
				}
			}

			void CBMDisturbanceListener::fetchDMAssociations() {
				_dmAssociations.clear();
				const auto& dmAssociations = _landUnitData->getVariable("disturbance_matrix_associations")->value()
					.extract<const std::vector<DynamicObject>>();

				for (const auto& dmAssociation : dmAssociations) {
					std::string disturbanceType = dmAssociation["disturbance_type"];
					int spu = dmAssociation["spatial_unit_id"];
					int dmId = dmAssociation["disturbance_matrix_id"];
					_dmAssociations.insert(std::make_pair(
						std::make_pair(disturbanceType, spu),
						dmId));
				}
			}

			void CBMDisturbanceListener::fetchLandClassTransitions() {
				const auto& transitions = _landUnitData->getVariable("land_class_transitions")->value();
				if (transitions.isVector()) {
					for (const auto& transition : transitions.extract<const std::vector<DynamicObject>>()) {
						std::string disturbanceType = transition["disturbance_type"];
						std::string landClass = transition["land_class_transition"];
						_landClassTransitions.insert(std::make_pair(disturbanceType, landClass));
					}
				}
				else {
					std::string disturbanceType = transitions["disturbance_type"];
					std::string landClass = transitions["land_class_transition"];
					_landClassTransitions.insert(std::make_pair(disturbanceType, landClass));
				}
			}

			void CBMDisturbanceListener::fetchDistTypeCodes() {
				if (!_landUnitData->hasVariable("disturbance_type_codes")) {
					return;
				}

				const auto& distTypeCodes = _landUnitData->getVariable("disturbance_type_codes")->value();
				if (distTypeCodes.isVector()) {
					for (const auto& code : distTypeCodes.extract<const std::vector<DynamicObject>>()) {
						std::string distType = code["disturbance_type"];
						int distTypeCode = code["disturbance_type_code"];
						_distTypeCodes[distType] = distTypeCode;
						_distTypeNames[distTypeCode] = distType;
					}
				}
				else {
					std::string distType = distTypeCodes["disturbance_type"];
					int distTypeCode = distTypeCodes["disturbance_type_code"];
					_distTypeCodes[distType] = distTypeCode;
					_distTypeNames[distTypeCode] = distType;
				}
			}

			void CBMDisturbanceListener::fetchPeatlandDMAssociations() {
				_peatlandDmAssociations.clear();

				//do following when the peatland_dm_associations is defined in configuration
				if (_landUnitData->hasVariable("peatland_dm_associations")) {
					const auto& dmAssociations = _landUnitData->getVariable("peatland_dm_associations")->value()
						.extract<const std::vector<DynamicObject>>();

					for (const auto& dmAssociation : dmAssociations) {
						int peatlandId = dmAssociation["peatland_id"];
						std::string distType = dmAssociation["disturbance_type"];
						int dmId = dmAssociation["peatland_dm_id"];
						int wtdModifierId = dmAssociation["wtd_modifier_id"];
						_peatlandDmAssociations.insert(std::make_pair(
							std::make_pair(peatlandId, distType), std::make_pair(dmId, wtdModifierId)));
					}
				}
			}

			void CBMDisturbanceListener::fetchDisturbanceOrder() {
				int order = 1;
				if (_landUnitData->hasVariable("user_disturbance_order")) {
					const auto& userOrder = _landUnitData->getVariable("user_disturbance_order")->value().extract<std::vector<DynamicVar>>();
					for (const auto& orderedDistType : userOrder) {
						_disturbanceOrder[orderedDistType] = order++;
					}
				}

				if (_landUnitData->hasVariable("default_disturbance_order")) {
					const auto& defaultOrder = _landUnitData->getVariable("default_disturbance_order")->value().extract<std::vector<DynamicVar>>();
					for (const auto& orderedDistType : defaultOrder) {
						if (_disturbanceOrder.find(orderedDistType) == _disturbanceOrder.end()) {
							_disturbanceOrder[orderedDistType] = order++;
						}
					}
				}
			}

			std::shared_ptr<IDisturbanceSubCondition> CBMDisturbanceListener::createSubCondition(const DynamicObject& config) {
				std::vector<std::shared_ptr<IDisturbanceSubCondition>> subConditions;
				for (const auto& kvp : config) {
					// Should we ignore this key?
					if (kvp.first == "disturbance_type" || kvp.first == "run_conditions" ||
						kvp.first == "override_conditions" || kvp.first == "override_disturbance_type") {

						continue;
					}

					// Is the condition a disturbance sequence?
					if (kvp.first == "disturbance_sequence") {
						std::vector<DisturbanceHistoryCondition> sequence;
						for (const auto& sequenceItemConfig : kvp.second.extract<const std::vector<DynamicVar>>()) {
							const auto& sequenceItem = sequenceItemConfig.extract<const std::vector<DynamicVar>>();
							std::string disturbanceType = sequenceItem[0].extract<std::string>();
							auto ageComparisonType = DisturbanceConditionType::AtLeast;
							DynamicVar ageAtDisturbance = 0;

							int maxYearsAgo = 9999;
							if (sequenceItem.size() > 1) {
								maxYearsAgo = sequenceItem[1].convert<int>();
							}

							if (sequenceItem.size() > 2) {
								ageComparisonType = sequenceItem[2] == "<" ? DisturbanceConditionType::LessThan
									: sequenceItem[2] == ">=" ? DisturbanceConditionType::AtLeast
									: sequenceItem[2] == "<->" ? DisturbanceConditionType::Between
									: DisturbanceConditionType::EqualTo;

								if (ageComparisonType == DisturbanceConditionType::Between) {
									ageAtDisturbance = DynamicVector{ sequenceItem[3], sequenceItem[4] };
								}
								else {
									ageAtDisturbance = sequenceItem[3];
								}
							}

							sequence.push_back(DisturbanceHistoryCondition{
								disturbanceType, maxYearsAgo, ageComparisonType, ageAtDisturbance });
						}

						subConditions.push_back(std::make_shared<DisturbanceSequenceSubCondition>(
							_landUnitData->timing(), _disturbanceHistory, sequence));

						continue;
					}

					// Extract the comparison type (<, =, >=, <->, !=) and target.
					auto targetType = DisturbanceConditionType::EqualTo;
					DynamicVar target;

					auto targetConfig = kvp.second;
					if (targetConfig.isVector()) {
						targetType = targetConfig[0] == "<" ? DisturbanceConditionType::LessThan
							: targetConfig[0] == ">=" ? DisturbanceConditionType::AtLeast
							: targetConfig[0] == "<->" ? DisturbanceConditionType::Between
							: targetConfig[0] == "!=" ? DisturbanceConditionType::NotIn
							: DisturbanceConditionType::In;

						if (targetType == DisturbanceConditionType::Between) {
							target = DynamicVector{ targetConfig[1], targetConfig[2] };
						}
						else if (targetType == DisturbanceConditionType::In ||
							targetType == DisturbanceConditionType::NotIn) {
							target = targetConfig;
						}
						else {
							target = targetConfig[1];
						}
					}
					else {
						target = targetConfig;
					}

					// Is the condition a single variable?
					if (_landUnitData->hasVariable(kvp.first)) {
						// If the variable is a classifier, get it from the "live" classifier set.
						if (_classifierNames.find(kvp.first) != _classifierNames.end()) {
							subConditions.push_back(std::make_shared<VariableDisturbanceSubCondition>(
								_classifierSet, targetType, target, kvp.first));
						}
						else {
							subConditions.push_back(std::make_shared<VariableDisturbanceSubCondition>(
								_landUnitData->getVariable(kvp.first), targetType, target));
						}

						continue;
					}

					// Is the condition one or more pools?
					std::vector<const flint::IPool*> pools;
					std::vector<std::string> poolNames;
					boost::algorithm::split(poolNames, kvp.first, boost::is_any_of("+"));

					for (std::string& poolName : poolNames) {
						boost::trim(poolName);
						const auto pool = _landUnitData->poolCollection().findPool(poolName);
						if (pool == nullptr) {
							throw RuntimeException("No pool or variable found with name: " + poolName);
						}

						pools.push_back(pool);
					}

					subConditions.push_back(std::make_shared<PoolDisturbanceSubCondition>(
						pools, targetType, target));
				}

				return std::make_shared<CompositeDisturbanceSubCondition>(subConditions);
			}

		}
	}
} // namespace moja::modules::cbm
