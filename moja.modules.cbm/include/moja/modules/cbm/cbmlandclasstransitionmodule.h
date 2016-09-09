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

        flint::ModuleTypes moduleType() { return flint::ModuleTypes::Model; };

        virtual void onLocalDomainInit() override;
        virtual void onTimingInit() override;
        virtual void onTimingStep() override;

    private:
        flint::IVariable* _historicLandClass;
        flint::IVariable* _currentLandClass;
        flint::IVariable* _unfcccLandClass;
        flint::IVariable* _isForest;
        std::unordered_map<std::string, bool> _landClassForestStatus;
        std::string _lastCurrentLandClass;

        void setUnfcccLandClass();
    };

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_
