#ifndef MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_
#define MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_

#include "moja/flint/modulebase.h"
#include "moja/hash.h"
#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

    class CBMLandClassTransitionModule : public flint::ModuleBase {
    public:
        CBMLandClassTransitionModule() : ModuleBase() {}
        virtual ~CBMLandClassTransitionModule() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes ModuleType() { return flint::ModuleTypes::Model; };

        virtual void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr&) override;
        virtual void onTimingInit(const flint::TimingInitNotification::Ptr&) override;
        virtual void onTimingStep(const flint::TimingStepNotification::Ptr&) override;

    private:
        flint::IVariable* _landClass;
        flint::IVariable* _gcId;
        std::unordered_map<std::string, bool> _landClassForestStatus;
        std::string _previousLandClass;
    };

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_
