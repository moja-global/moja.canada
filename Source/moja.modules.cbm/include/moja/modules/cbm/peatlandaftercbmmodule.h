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
		const flint::IPool* _acrotelm_o;
		const flint::IPool* _catotelm_a;
		const flint::IPool* _atmosphere;

		const flint::IPool* _softwoodFoliage;
		const flint::IPool* _hardwoodFoliage;

		const flint::IPool* _softwoodOther;
		const flint::IPool* _hardwoodOther;

		const flint::IPool* _softwoodFineRoots;
		const flint::IPool* _hardwoodFineRoots;

		const flint::IPool* _woodyFoliageDead;
		const flint::IPool* _woodyStemsBranchesDead;
		const flint::IPool* _woodyRootsDead;	

		/*
		For forested peatland, transfer som of the cbm pool values to peatland pool.
		*/
		void transferCBMPoolToPeatland();
    };
}}}
#endif