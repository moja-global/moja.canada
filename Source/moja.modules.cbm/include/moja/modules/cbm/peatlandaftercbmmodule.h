#ifndef MOJA_MODULES_CBM_PEATLAND_AFTERCBM_H_
#define MOJA_MODULES_CBM_PEATLAND_AFTERCBM_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    After CBM simulation on a landunit, prepare the landunit to simulate 
    peatland simulation.
	(1) transfer carbon from some of CBM pools to peatland pools
	(2) should be called after finishing regular CBM simulation.
    */
    
    class CBM_API PeatlandAfterCBMModule : public CBMModuleBase {
    public:
        PeatlandAfterCBMModule() {};
        virtual ~PeatlandAfterCBMModule() {};			

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;            
       
		void doLocalDomainInit() override;
		void doTimingStep() override;

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

		/*
		For forested peatland, transfer som of the cbm pool values to peatland pool.
		*/
		void transferCBMPoolToPeatland();
    };
}}}
#endif