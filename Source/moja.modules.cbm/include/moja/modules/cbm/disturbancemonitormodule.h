#ifndef MOJA_MODULES_CBM_DISTURBANCEMONITOR_H_
#define MOJA_MODULES_CBM_DISTURBANCEMONITOR_H_

#include "moja/modules/cbm/cbmmodulebase.h"

namespace moja {
namespace modules {
namespace cbm {
	
	class DisturbanceMonitorModule : public CBMModuleBase {
	public:
		DisturbanceMonitorModule() : CBMModuleBase() {}
		virtual ~DisturbanceMonitorModule() = default;

		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

        virtual void doLocalDomainInit() override;
        virtual void doTimingInit() override;
        virtual void doOutputStep() override;
		virtual void doDisturbanceEvent(DynamicVar) override;

	private:
        bool _moduleEnabled = true;
		flint::IVariable* _currentDisturbance;
	};

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_DISTURBANCEMONITOR_H_
