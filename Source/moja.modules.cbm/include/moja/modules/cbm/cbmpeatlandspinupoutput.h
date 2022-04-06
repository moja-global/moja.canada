#ifndef MOJA_MODULES_CBM_PEATLAND_SPINUP_OUTPUT_H_
#define MOJA_MODULES_CBM_PEATLAND_SPINUP_OUTPUT_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include <fstream>

namespace moja {
namespace modules {
namespace cbm {

    /*
   Record the pool values during peatland spinup phase
    */    
    class CBM_API CBMPeatlandSpinupOutput : public CBMModuleBase {
    public:
		CBMPeatlandSpinupOutput() {};
        virtual ~CBMPeatlandSpinupOutput() {};
		
		
        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;            
       
		void doLocalDomainInit() override;
		void doTimingInit() override;					
		void doLocalDomainShutdown() override;	
		void doTimingStep() override;
		void doDisturbanceEvent(DynamicVar) override;
		void doPrePostDisturbanceEvent() override;

    private:      
		flint::IVariable* _peatland_spinup_rotation;
		flint::IVariable* _stand_age;
		flint::IVariable* _tree_age;
		flint::IVariable* _shrub_age;		

		bool _runPeatland{ false };
		bool _isInitialPoolLoaded{ false };		
		bool _isForestPeatland{ false };
		bool _isTreedPeatland{ false };	
		bool _isOutputLog{ false };

		std::ofstream timeStepOutputFile;

		int _peatlandId{ -1 };
		int _fireReturnIntervalValue{ -1 };
		bool _isSpinupFileCreated{ false };	

		std::string getTimeStamp();
		void outputPoolValues();	
		std::string fileNameFixed;
    };
}}}
#endif