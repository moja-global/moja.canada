#ifndef MOJA_MODULES_CBM_PEATLAND_DISTURBANCE_H_
#define MOJA_MODULES_CBM_PEATLAND_DISTURBANCE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/peatlandfireparameters.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    Response to the historical and last disturbance events in CBM spinup
    */
    class CBM_API PeatlandDisturbanceModule : public moja::flint::ModuleBase {
    public:
        PeatlandDisturbanceModule(){			
			_sourcePools = { 
				"WoodyStemsBranchesLive",
				"WoodyFoliageLive",
				"WoodyRootsLive",
				"SedgeFoliageLive",
				"SedgeRootsLive",
				"SphagnumMossLive",
				"FeatherMossLive",				
				"WoodyStemsBranchesDead",
				"WoodyFoliageDead",
				"WoodyRootsDead",
				"SedgeFoliageDead",
				"SedgeRootsDead",
				"FeathermossDead",
				"Acrotelm" 
			};			
		};

        virtual ~PeatlandDisturbanceModule(){};		

		const std::string fireEvent = "fire";
		const std::string CO2 = "CO2";
		const std::string CO = "CO";
		const std::string CH4 = "CH4";

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void onDisturbanceEvent(Dynamic) override;
        void onLocalDomainInit() override;
        void onTimingInit() override;
    private: 
        flint::IVariable* _spu;
        int _spuId;   
	
		bool _isPeatland;

		std::shared_ptr<PeatlandFireParameters> _fireParameter;
		std::vector<std::string> _sourcePools;		
    };
}}}
#endif