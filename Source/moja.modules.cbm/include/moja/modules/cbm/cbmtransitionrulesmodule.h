#ifndef MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_
#define MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_

#include "moja/flint/modulebase.h"
#include "moja/hash.h"
#include "moja/modules/cbm/cbmmodulebase.h"

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
        CBMTransitionRulesModule() : CBMModuleBase() {}
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

        int findTransitionRule(const std::string& disturbanceType);
    };

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_