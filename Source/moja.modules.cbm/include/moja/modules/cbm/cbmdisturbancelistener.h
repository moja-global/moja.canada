#ifndef MOJA_MODULES_CBM_CBMDISTURBANCELISTENER_H_
#define MOJA_MODULES_CBM_CBMDISTURBANCELISTENER_H_

#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/hash.h"
#include "moja/flint/ivariable.h"
#include "moja/flint/ipool.h"
#include "moja/flint/itiming.h"

#include <unordered_map>
#include <unordered_set>

namespace moja {
	namespace modules {
		namespace cbm {

			struct DisturbanceHistoryRecord {
				std::string disturbanceType;
				int year;
				int ageAtDisturbance;
			};

			struct DisturbanceConditionResult {
				bool hadRunConditions = false;
				bool shouldRun = false;
				std::string newDisturbanceType = "";

				void merge(const DisturbanceConditionResult& other) {
					hadRunConditions = hadRunConditions || other.hadRunConditions;
					shouldRun = shouldRun || other.shouldRun;
					if (newDisturbanceType == "") {
						newDisturbanceType = other.newDisturbanceType;
					}
				}
			};

			class IDisturbanceSubCondition {
			public:
				virtual ~IDisturbanceSubCondition() = default;

				virtual bool check() const = 0;
			};

			enum class DisturbanceConditionType {
				LessThan,
				EqualTo,
				AtLeast,
				Between,
				In,
				NotIn
			};

			struct DisturbanceHistoryCondition {
				std::string disturbanceType;
				int maxYearsAgo = 9999;
				DisturbanceConditionType ageComparison = DisturbanceConditionType::AtLeast;
				DynamicVar ageAtDisturbance = 0;
			};

			class DisturbanceCondition {
			public:
				DisturbanceCondition(
					const std::vector<std::string> disturbanceTypes,
					const std::vector<std::shared_ptr<IDisturbanceSubCondition>> matchConditions,
					const std::vector<std::shared_ptr<IDisturbanceSubCondition>> runConditions,
					const std::vector<std::shared_ptr<IDisturbanceSubCondition>> overrideConditions,
					const std::string& overrideDisturbanceType = "") : _disturbanceTypes(disturbanceTypes),
					_matchConditions(matchConditions), _runConditions(runConditions),
					_overrideConditions(overrideConditions), _overrideDisturbanceType(overrideDisturbanceType) { }

				bool isApplicable(const std::string& disturbanceType) {
					bool isMatchingDisturbance = false;
					for (const auto& matchDisturbanceType : _disturbanceTypes) {
						if (disturbanceType == matchDisturbanceType) {
							isMatchingDisturbance = true;
							break;
						}
					}

					if (!isMatchingDisturbance) {
						return false;
					}

					for (auto matchCondition : _matchConditions) {
						if (!matchCondition->check()) {
							return false;
						}
					}

					return true;
				}

				DisturbanceConditionResult check() {
					DisturbanceConditionResult result;

					if (_runConditions.size() > 0) {
						result.hadRunConditions = true;
					}

					for (auto runCondition : _runConditions) {
						if (runCondition->check()) {
							result.shouldRun = true;
							break;
						}
					}

					if (result.hadRunConditions && !result.shouldRun) {
						return result;
					}

					for (auto overrideCondition : _overrideConditions) {
						if (overrideCondition->check()) {
							result.newDisturbanceType = _overrideDisturbanceType;
							break;
						}
					}

					return result;
				}

			private:
				const std::vector<std::string> _disturbanceTypes;
				const std::vector<std::shared_ptr<IDisturbanceSubCondition>> _matchConditions;
				const std::vector<std::shared_ptr<IDisturbanceSubCondition>> _runConditions;
				const std::vector<std::shared_ptr<IDisturbanceSubCondition>> _overrideConditions;
				const std::string _overrideDisturbanceType;
			};

			class CompositeDisturbanceSubCondition : public IDisturbanceSubCondition {
			public:
				CompositeDisturbanceSubCondition(std::vector<std::shared_ptr<IDisturbanceSubCondition>> conditions) :
					_conditions(conditions) { }

				bool check() const override {
					for (const auto condition : _conditions) {
						if (!condition->check()) {
							return false;
						}
					}

					return true;
				}

			private:
				std::vector<std::shared_ptr<IDisturbanceSubCondition>> _conditions;
			};

			class VariableDisturbanceSubCondition : public IDisturbanceSubCondition {
			public:
				VariableDisturbanceSubCondition(
					const flint::IVariable* var, DisturbanceConditionType type, const DynamicVar& target,
					const std::string& propertyName = "") : _var(var), _property(propertyName),
					_type(type), _target(target) { }

				bool check() const override {
					const auto value = _property == "" ? _var->value() : _var->value()[_property];
					return _type == DisturbanceConditionType::LessThan ? value - _target < 0
						: _type == DisturbanceConditionType::EqualTo ? value == _target
						: _type == DisturbanceConditionType::AtLeast ? value - _target >= 0
						: _type == DisturbanceConditionType::Between ? value >= _target[0] && value <= _target[1]
						: _type == DisturbanceConditionType::In ? search(value)
						: _type == DisturbanceConditionType::NotIn ? !search(value)
						: false;
				}

			private:
				const std::string _property;
				const flint::IVariable* _var;
				const DisturbanceConditionType _type;
				const DynamicVar _target;

				bool search(DynamicVar value) const {
					for (const auto& targetValue : _target) {
						if (value == targetValue) {
							return true;
						}
					}

					return false;
				}
			};

			class PoolDisturbanceSubCondition : public IDisturbanceSubCondition {
			public:
				PoolDisturbanceSubCondition(
					std::vector<const flint::IPool*> pools, DisturbanceConditionType type, const DynamicVar& target)
					: _pools(pools), _type(type), _target(target) { }

				bool check() const override {
					double sum = 0.0;
					for (auto pool : _pools) {
						sum += pool->value();
					}

					return _type == DisturbanceConditionType::LessThan ? sum - _target < 0
						: _type == DisturbanceConditionType::EqualTo ? sum == _target
						: _type == DisturbanceConditionType::AtLeast ? sum - _target >= 0
						: _type == DisturbanceConditionType::Between ? sum >= _target[0] && sum <= _target[1]
						: false;
				}

			private:
				const std::vector<const flint::IPool*> _pools;
				const DisturbanceConditionType _type;
				const DynamicVar _target;
			};

			class DisturbanceSequenceSubCondition : public IDisturbanceSubCondition {
			public:
				DisturbanceSequenceSubCondition(
					flint::ITiming* timing,
					std::shared_ptr<std::deque<DisturbanceHistoryRecord>> disturbanceHistory,
					const std::vector<DisturbanceHistoryCondition>& sequence)
					: _timing(timing), _disturbanceHistory(disturbanceHistory), _sequence(sequence) { }

				bool check() const override {
					auto referenceYear = _timing->curStartDate().year();
					int historyIndex = 0;
					int lastHistoryIndex = _disturbanceHistory->size() - 1;
					for (auto i = 0; i < _sequence.size(); i++) {
						const auto& expected = _sequence[i];
						bool matchNone = expected.disturbanceType == "none";
						bool matchAny = expected.disturbanceType == "any";

						if (historyIndex > lastHistoryIndex) {
							return (matchNone && i == _sequence.size() - 1) ? true : false;
						}

						const auto& actual = _disturbanceHistory->operator[](historyIndex);

						if (!(matchNone || matchAny) && expected.disturbanceType != actual.disturbanceType) {
							return false;
						}

						bool withinMaxYears = expected.maxYearsAgo == -1 || referenceYear - actual.year <= expected.maxYearsAgo;
						if (matchNone && withinMaxYears) {
							return false;
						}
						else if (!matchNone && !withinMaxYears) {
							return false;
						}

						if (!(expected.ageComparison == DisturbanceConditionType::LessThan ? actual.ageAtDisturbance - expected.ageAtDisturbance < 0
							: expected.ageComparison == DisturbanceConditionType::EqualTo ? actual.ageAtDisturbance == expected.ageAtDisturbance
							: expected.ageComparison == DisturbanceConditionType::AtLeast ? actual.ageAtDisturbance - expected.ageAtDisturbance >= 0
							: expected.ageComparison == DisturbanceConditionType::Between ? actual.ageAtDisturbance >= expected.ageAtDisturbance[0] && actual.ageAtDisturbance <= expected.ageAtDisturbance[1]
							: false)) {

							return false;
						}

						if (!matchNone) {
							referenceYear = actual.year;
							historyIndex++;
						}
						else {
							// "Match none" condition passed, so the next sequence condition
							// gets checked against the same disturbance event relative to the
							// end of the "Match none" time period.
							if (expected.maxYearsAgo != -1) {
								referenceYear -= expected.maxYearsAgo;
							}
						}
					}

					return true;
				}

			private:
				flint::ITiming* _timing;
				std::shared_ptr<std::deque<DisturbanceHistoryRecord>> _disturbanceHistory;
				std::vector<DisturbanceHistoryCondition> _sequence;
			};

			class CBMDistEventRef {
			public:
				CBMDistEventRef() = default;

				explicit CBMDistEventRef(
					std::string& disturbanceType, int year, int transitionId,
					std::vector<std::shared_ptr<IDisturbanceSubCondition>> conditions,
					const DynamicObject& metadata) :
					_disturbanceType(disturbanceType), _year(year), _transitionRuleId(transitionId),
					_disturbanceConditions(conditions), _metadata(metadata) { }

				std::string disturbanceType() const { return _disturbanceType; }
				void setDisturbanceType(const std::string& disturbanceType) { _disturbanceType = disturbanceType; }
				int transitionRuleId() const { return _transitionRuleId; }
				int year() const { return _year; }
				const DynamicObject& metadata() { return _metadata; }

				bool checkConditions() {
					for (const auto condition : _disturbanceConditions) {
						if (!condition->check()) {
							return false;
						}
					}

					return true;
				}

			private:
				std::string _disturbanceType;
				int _transitionRuleId;
				int	_year;
				DynamicObject _metadata;
				std::vector<std::shared_ptr<IDisturbanceSubCondition>> _disturbanceConditions;
			};

			class CBMDistEventTransfer {
			public:
				CBMDistEventTransfer() = default;

				CBMDistEventTransfer(flint::ILandUnitDataWrapper& landUnitData, const DynamicObject& data) :
					_disturbanceMatrixId(data["disturbance_matrix_id"]),
					_sourcePool(landUnitData.getPool(data["source_pool_name"].convert<std::string>())),
					_destPool(landUnitData.getPool(data["dest_pool_name"].convert<std::string>())),
					_proportion(data["proportion"]) { }

				CBMDistEventTransfer(flint::ILandUnitDataWrapper& landUnitData, const std::string& sourcePool,
					const std::string& destPool, double proportion) :
					_disturbanceMatrixId(-1),
					_sourcePool(landUnitData.getPool(sourcePool)),
					_destPool(landUnitData.getPool(destPool)),
					_proportion(proportion) { }

				int disturbanceMatrixId() const { return _disturbanceMatrixId; }
				const flint::IPool* sourcePool() const { return _sourcePool; }
				const flint::IPool* destPool() const { return _destPool; }
				double proportion() const { return _proportion; }
				void setProportion(double proportion) { _proportion = proportion; }

			private:
				int _disturbanceMatrixId;
				const flint::IPool* _sourcePool;
				const flint::IPool* _destPool;
				double _proportion;
			};

			class CBMDisturbanceListener : public CBMModuleBase {
			public:
				CBMDisturbanceListener() : CBMModuleBase() {
					_disturbanceHistory = std::make_shared<std::deque<DisturbanceHistoryRecord>>();
				}

				virtual ~CBMDisturbanceListener() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

				virtual void doLocalDomainInit() override;
				virtual void doSystemShutdown() override;
				virtual void doTimingInit() override;
				virtual void doTimingStep() override;
				virtual void doDisturbanceEvent(DynamicVar) override;

			private:
				typedef std::vector<CBMDistEventTransfer> EventVector;
				typedef std::unordered_map<int, EventVector> EventMap;

				NotificationCenter* _notificationCenter;
				std::vector<std::string> _layerNames;
				std::vector<const flint::IVariable*> _layers;
				flint::IVariable* _landClass;
				flint::IVariable* _spu;
				flint::IVariable* _classifierSet;
				flint::IVariable* _age;
				EventMap _matrices;

				std::unordered_map<std::pair<int, std::string>, std::pair<int, int>> _peatlandDmAssociations;
				std::unordered_map<std::pair<std::string, int>, int> _dmAssociations;
				std::unordered_map<std::string, std::string> _landClassTransitions;
				std::map<int, std::vector<CBMDistEventRef>> _landUnitEvents;
				std::unordered_map<std::string, int> _distTypeCodes;
				std::unordered_map<int, std::string> _distTypeNames;
				std::unordered_set<std::string> _errorLayers;
				std::unordered_set<std::string> _classifierNames;
				std::unordered_map<std::string, int> _disturbanceOrder;

				bool _disturbanceConditionsInitialized = false;
				DynamicVar _conditionConfig;
				std::vector<DisturbanceCondition> _disturbanceConditions;
				std::shared_ptr<std::deque<DisturbanceHistoryRecord>> _disturbanceHistory;

				void fetchMatrices();
				void fetchDMAssociations();
				void fetchLandClassTransitions();
				void fetchDistTypeCodes();
				void fetchPeatlandDMAssociations();
				void fetchDisturbanceOrder();
				std::shared_ptr<IDisturbanceSubCondition> createSubCondition(const DynamicObject& config);
				std::string getDisturbanceTypeName(const DynamicObject& eventData);
				bool addLandUnitEvent(const DynamicVar& eventData);
				void fireCBMDisturbanceEvent(CBMDistEventRef& e);
				void firePeatlandDisturbanceEvent(CBMDistEventRef& e);
			};

		}
	}
} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMDISTURBANCELISTENER_H_
