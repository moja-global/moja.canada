#ifndef MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_
#define MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_

#include "moja/flint/modulebase.h"
#include "moja/hash.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

#include <unordered_map>

namespace moja {
	namespace modules {
		namespace cbm {

			enum class AgeResetType {
				Absolute,
				Relative,
				Yield
			};

			class TransitionRule {
			public:
				TransitionRule() {}
				TransitionRule(const DynamicObject& data);
				TransitionRule(int id, int resetAge, int regenDelay)
					: _id(id), _resetAge(resetAge), _regenDelay(regenDelay) { }

				int id() { return _id; }
				AgeResetType resetType() { return _resetType; }
				int resetAge() { return _resetAge; }
				int regenDelay() { return _regenDelay; }

				const std::unordered_map<std::string, std::string> classifiers() const {
					return _classifiers;
				}

				void addClassifier(std::string name, std::string value) {
					_classifiers[name] = value;
				}

			private:
				int _id;
				AgeResetType _resetType;
				int _resetAge;
				int _regenDelay;
				std::unordered_map<std::string, std::string> _classifiers;
			};

			class CBMTransitionRulesModule : public CBMModuleBase {
			public:
				CBMTransitionRulesModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory, std::shared_ptr<VolumeToBiomassCarbonGrowth> volumeToBioGrowth)
					: CBMModuleBase(), _gcFactory(gcFactory), _volumeToBioGrowth(volumeToBioGrowth) {};

				virtual ~CBMTransitionRulesModule() = default;

				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

				virtual void doDisturbanceEvent(DynamicVar) override;
				virtual void doLocalDomainInit() override;
				virtual void doTimingInit() override;
				virtual void doTimingShutdown() override;

			private:
				flint::IVariable* _age;
				flint::IVariable* _cset;
				flint::IVariable* _regenDelay;
				flint::IVariable* _transitionRuleMatches;
				std::unordered_map<int, TransitionRule> _transitions;
				bool _allowMatchingRules = false;
				std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
				std::shared_ptr<StandGrowthCurveFactory> _gcFactory;
				const flint::IPool* _softwoodMerch;
				const flint::IPool* _softwoodOther;
				const flint::IPool* _softwoodFoliage;
				const flint::IPool* _softwoodCoarseRoots;
				const flint::IPool* _softwoodFineRoots;
				const flint::IPool* _hardwoodMerch;
				const flint::IPool* _hardwoodOther;
				const flint::IPool* _hardwoodFoliage;
				const flint::IPool* _hardwoodCoarseRoots;
				const flint::IPool* _hardwoodFineRoots;
				flint::IVariable* _gcId;
				flint::IVariable* _spuId;
				Int64 _standSpuId;

				int findTransitionRule(const std::string& disturbanceType);
				int findYieldCurveAge();
				double calculateBiomass();
			};

		}
	}
} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_