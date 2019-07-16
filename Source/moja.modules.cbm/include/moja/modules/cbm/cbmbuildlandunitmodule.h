#ifndef MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_
#define MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_

#include "moja/modules/cbm/cbmmodulebase.h"

namespace moja {
namespace modules {
namespace cbm {

    class CBMBuildLandUnitModule : public CBMModuleBase {
    public:
        CBMBuildLandUnitModule() : CBMModuleBase() {}
        virtual ~CBMBuildLandUnitModule() {};

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void doLocalDomainInit() override;
        void doPreTimingSequence() override;

    private:
		// Mask IN: a pixel is simulated if all mask variables have values.
		std::vector<std::string> _maskVarNames;
		std::vector<const flint::IVariable*> _maskVars;

        const flint::IVariable* _initialAge;
        const flint::IVariable* _initialCSet;
        const flint::IVariable* _initialHistoricLandClass;
        const flint::IVariable* _initialCurrentLandClass;

        flint::IVariable* _age;
        flint::IVariable* _buildWorked;
        flint::IVariable* _cset;
        flint::IVariable* _historicLandClass;
        flint::IVariable* _currentLandClass;
        flint::IVariable* _isForest;
    };

}}} // namespace moja::Modules::cbm
#endif // MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_
