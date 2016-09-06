#ifndef MOJA_MODULES_CBM_MOSS_DISTURBANCE_H_
#define MOJA_MODULES_CBM_MOSS_DISTURBANCE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    Moss module to response to the fire disturbance events in CBM
    */
    class CBM_API MossDisturbanceModule : public moja::flint::ModuleBase {
    public:
        MossDisturbanceModule(){	
			_isMoss = false;

			_sourcePools = { 
				"FeatherMossLive",
				"SphagnumMossLive",
				"FeatherMossFast",
				"SphagnumMossFast" 
			};	
				
			_destPools = { 
				"CO2",
				"CH4",
				"CO",
				"FeatherMossSlow",
				"SphagnumMossSlow" 
			};			
		};

        virtual ~MossDisturbanceModule(){};			

		const std::string fireEvent = "fire";

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void onDisturbanceEvent(Dynamic) override;
        void onLocalDomainInit() override;
        void onTimingInit() override;
    private:    	
		bool _isMoss;

		std::vector<std::string> _sourcePools;	
		std::vector<std::string> _destPools;	
		std::vector<double> _transferRates;

		void recordMossTransfers(const DynamicObject& data);
    };
}}}
#endif