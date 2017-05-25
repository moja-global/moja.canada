#ifndef MOJA_MODULES_CBM_PEATLAND_PREPARE_H_
#define MOJA_MODULES_CBM_PEATLAND_PREPARE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    Response to the historical and last disturbance events in CBM spinup
    */
    
    class CBM_API PeatlandPrepareModule : public CBMModuleBase {
    public:
        PeatlandPrepareModule() {
			_isInitialPoolLoaded = false;					
		};

        virtual ~PeatlandPrepareModule() {};

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;            
       
		void doLocalDomainInit() override;
        void doTimingInit() override;

    private:       
		flint::IPool::ConstPtr _acrotelm_o;
		flint::IPool::ConstPtr _catotelm_a;
		flint::IPool::ConstPtr _atmosphere;

		flint::IPool::ConstPtr _softwoodFoliage;
		flint::IPool::ConstPtr _hardwoodFoliage;
		flint::IPool::ConstPtr _softwoodOther;
		flint::IPool::ConstPtr _hardwoodOther;
		flint::IPool::ConstPtr _softwoodFineRoots;
		flint::IPool::ConstPtr _hardwoodFineRoots;
		flint::IPool::ConstPtr _woodyFoliageDead;
		flint::IPool::ConstPtr _woodyStemsBranchesDead;
		flint::IPool::ConstPtr _woodyRootsDead;
		bool _isInitialPoolLoaded; 
		bool _runPeatland; 

		void loadPeatlandInitialPoolValues(const DynamicObject& data);
		void transferCBMPoolToPeatland();
		double computeLongtermWTD(double dc, int peatlandID);
    };
}}}
#endif