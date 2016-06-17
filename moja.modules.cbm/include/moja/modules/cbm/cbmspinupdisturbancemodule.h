#ifndef MOJA_MODULES_CBM_CBM_SPINUPDISTURBANCEMODULE_H_
#define MOJA_MODULES_CBM_CBM_SPINUPDISTURBANCEMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    Response to the historical and last disturbance events in CBM spinup
    */
    class CBM_API CBMSpinupDisturbanceModule : public moja::flint::ModuleBase {
    public:
        CBMSpinupDisturbanceModule(){};
        virtual ~CBMSpinupDisturbanceModule(){};		

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr) override;
        void onLocalDomainInit() override;
        void onTimingInit() override;

		virtual void onPostDisturbanceEvent() override;
    private:	
        typedef std::vector<CBMDistEventTransfer::Ptr> EventVector;
        typedef std::unordered_map<int, EventVector> EventMap;

        flint::IVariable* _spu;
        int _spuId;
        EventMap _matrices;
        std::unordered_map<std::pair<std::string, int>, int> _dmAssociations;

        void fetchMatrices();
        void fetchDMAssociations();
    };
}}}
#endif