#ifndef MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_
#define MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_

#include "moja/flint/modulebase.h"
#include "moja/hash.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

    class TransitionRule {
    public:
        TransitionRule() {}
        TransitionRule(int id, int resetAge, int regenDelay)
            : _id(id), _resetAge(resetAge), _regenDelay(regenDelay) { }
        
        int id() { return _id; }
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
        int _resetAge;
        int _regenDelay;
        std::unordered_map<std::string, std::string> _classifiers;
    };
    
    class CBMTransitionRulesModule : public flint::ModuleBase {
    public:
        CBMTransitionRulesModule() : ModuleBase() {}
        virtual ~CBMTransitionRulesModule() = default;

        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

        virtual void onDisturbanceEvent(const Dynamic) override;
        virtual void onLocalDomainInit() override;

    private:
        flint::IVariable* _age;
        flint::IVariable* _cset;
        flint::IVariable* _regenDelay;
        std::unordered_map<int, TransitionRule> _transitions;
    };

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMTRANSITIONRULESMODULE_H_