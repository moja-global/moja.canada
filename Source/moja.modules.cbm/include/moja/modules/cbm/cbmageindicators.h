#ifndef MOJA_MODULES_CBM_AGE_INDICATORS_H_
#define MOJA_MODULES_CBM_AGE_INDICATORS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/ageclasshelper.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
	Compute and update forest landunit age class information
    */
    class CBM_API CBMAgeIndicators : public CBMModuleBase {
    public:
        CBMAgeIndicators() {};
        virtual ~CBMAgeIndicators() {};			

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;            
       
		void doLocalDomainInit() override;
		void doTimingStep() override;

    private:       
        AgeClassHelper _ageClassHelper;
    };
}}}
#endif