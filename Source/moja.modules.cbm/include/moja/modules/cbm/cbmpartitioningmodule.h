#ifndef MOJA_MODULES_CBM_PARTITIONING_MODULE_H_
#define MOJA_MODULES_CBM_PARTITIONING_MODULE_H_

#include <list>

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

namespace moja {
namespace modules {
namespace cbm {

    class IRecoveryRule;

    class CBM_API CBMPartitioningModule : public CBMModuleBase {
    public:
        CBMPartitioningModule() : CBMModuleBase() {};
        virtual ~CBMPartitioningModule() {};			

        void subscribe(NotificationCenter& notificationCenter) override;            
       
		void doLocalDomainInit() override;
        void doTimingInit() override;
        void doDisturbanceEvent(DynamicVar e) override;
		void doTimingStep() override;

    private:
        std::list<std::shared_ptr<IRecoveryRule>> _activeRecoveryRules;
        std::list<std::shared_ptr<IRecoveryRule>> _pendingRecoveryRules;
        double _cumulativeMortality = 0.0;
        int _spuId;
        std::map<std::string, std::map<int, double>> _disturbanceMortality;
        std::map<std::string, std::string> _disturbanceCategories;
        std::map<std::string, std::map<int, std::tuple<std::string, DynamicVar>>> _initialRecoveryRules;
        std::map<std::string, std::map<int, std::tuple<std::string, DynamicVar>>> _recoveryRules;
        
        std::vector<const flint::IPool*> _agBiomassPools;
        std::vector<const flint::IPool*> _totalBiomassPools;
        std::vector<const flint::IPool*> _totalEcoPools;

        flint::IVariable* _partition;
        flint::IVariable* _spinupParameters;
        flint::IVariable* _spu;

        void fetchRecoveryRules();

        std::shared_ptr<IRecoveryRule> createInitialRecoveryRule(std::string disturbanceType);
        std::shared_ptr<IRecoveryRule> createRecoveryRule(std::string disturbanceType);

        void doSmallDisturbanceEvent(std::string disturbanceType, double mortality);
        void doLargeDisturbanceEvent(std::string disturbanceType);
    };
}}}
#endif