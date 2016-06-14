#ifndef MOJA_MODULES_CBM_PEATLAND_PREPARE_H_
#define MOJA_MODULES_CBM_PEATLAND_PREPARE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    Response to the historical and last disturbance events in CBM spinup
    */
    
    class CBM_API PeatlandPrepareModule : public moja::flint::ModuleBase {
    public:
        PeatlandPrepareModule(){
			_isInitialPoolLoaded = false;					
		};

        virtual ~PeatlandPrepareModule(){};			

		const std::string fireEvent = "fire";

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;            
       
		void onLocalDomainInit() override;
        void onTimingInit() override;
    private:       
		flint::IPool::ConstPtr _acrotelm;
		flint::IPool::ConstPtr _catotelm;
		flint::IPool::ConstPtr _atmosphere;

		bool _isInitialPoolLoaded; 
		bool _isPeatland; 

		void loadPeatlandInitialPoolValues(const DynamicObject& data);
    };
}}}
#endif