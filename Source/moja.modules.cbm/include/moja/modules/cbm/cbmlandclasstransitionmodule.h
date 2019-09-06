#ifndef MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_
#define MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_

#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/hash.h"
#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

    class CBMLandClassTransitionModule : public CBMModuleBase {
    public:
        CBMLandClassTransitionModule() : CBMModuleBase() {}
        virtual ~CBMLandClassTransitionModule() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() { return flint::ModuleTypes::Model; };

        virtual void doLocalDomainInit() override;
        virtual void doTimingInit() override;
        virtual void doTimingStep() override;

    private:
        flint::IVariable* _historicLandClass;
        flint::IVariable* _currentLandClass;
        flint::IVariable* _unfcccLandClass;
        flint::IVariable* _isForest;
        flint::IVariable* _lastPassDisturbance;
        flint::IVariable* _isDecaying;
        flint::IVariable* _lastPassDisturbanceTimeseries = nullptr;

        std::unordered_map<std::string, bool> _landClassForestStatus;
        std::unordered_map<std::string, int> _landClassElapsedTime;
        std::unordered_map<std::string, std::string> _landClassTransitions;
        std::string _lastCurrentLandClass;
        int _yearsSinceTransition = 0;

        void updateRemainingStatus(std::string landClass);
        void setUnfcccLandClass();
        void fetchLandClassTransitions();
        std::string getCreationDisturbance();
    };

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMLANDCLASSTRANSITIONMODULE_H_
