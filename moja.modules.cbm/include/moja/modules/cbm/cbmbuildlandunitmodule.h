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

        void onLocalDomainInit() override;
        void onPreTimingSequence() override;

    private:
        const flint::IVariable* _initialAge;
        const flint::IVariable* _initialCSet;
        const flint::IVariable* _initialHistoricLandClass;
        const flint::IVariable* _initialCurrentLandClass;
        flint::IVariable* _buildWorked;
        flint::IVariable* _cset;
        flint::IVariable* _historicLandClass;
        flint::IVariable* _currentLandClass;
    };

}}} // namespace moja::Modules::cbm
#endif // MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_
