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
		bool _isInitialPoolLoaded; 
		bool _runPeatland; 

		void loadPeatlandInitialPoolValues(const DynamicObject& data);
		void transferCBMPoolToPeatland();
		double computeLongtermWTD(double dc, int peatlandID);
    };
}}}
#endif