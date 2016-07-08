#ifndef MOJA_MODULES_CBM_PEATLAND_DISTURBANCE_H_
#define MOJA_MODULES_CBM_PEATLAND_DISTURBANCE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    Response to the historical and last disturbance events in CBM spinup
    */
    class CBM_API PeatlandDisturbance : public moja::flint::ModuleBase {
    public:
        PeatlandDisturbance(){};
        virtual ~PeatlandDisturbance(){};		

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr) override;
        void onLocalDomainInit() override;
        void onTimingInit() override;
    private: 
        flint::IVariable* _spu;
        int _spuId;   

		std::unordered_map<std::pair<std::string, int>, int> _dmAssociations;
		void fetchDMAssociations();
    };
}}}
#endif