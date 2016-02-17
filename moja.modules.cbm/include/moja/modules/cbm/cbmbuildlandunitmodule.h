#ifndef MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_
#define MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_

#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {

    class CBMBuildLandUnitModule : public flint::ModuleBase {
    public:
        CBMBuildLandUnitModule() : ModuleBase() {}
        virtual ~CBMBuildLandUnitModule() {};

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
        void onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n) override;

    private:
        flint::IVariable* _buildWorked;
        const flint::IVariable* _initialAge;
        const flint::IVariable* _initialGCID;
        flint::IVariable* _gcid;
        const flint::IVariable* _cset;
        const flint::IVariable* _initialHistoricLandClass;
        const flint::IVariable* _initialCurrentLandClass;
        flint::IVariable* _historicLandClass;
        flint::IVariable* _currentLandClass;
    };

}}} // namespace moja::Modules::cbm
#endif // MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_
