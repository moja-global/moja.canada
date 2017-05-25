#ifndef MOJA_MODULES_CBM_PLGROWTH_H_
#define MOJA_MODULES_CBM_PLGROWTH_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlandgrowthparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthcurve.h"

namespace moja {
namespace modules {
namespace cbm {
		
	class CBM_API PeatlandGrowthModule : public CBMModuleBase {
	public:
		PeatlandGrowthModule() : CBMModuleBase() { }
		virtual ~PeatlandGrowthModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;		

		void doLocalDomainInit() override;
		void doTimingInit() override;
		void doTimingStep() override;

	private:		
		flint::IPool::ConstPtr _atmosphere;
		flint::IPool::ConstPtr _woodyFoliageLive;
		flint::IPool::ConstPtr _woodyStemsBranchesLive;
		flint::IPool::ConstPtr _woodyRootsLive;
		flint::IPool::ConstPtr _sedgeFoliageLive;
		flint::IPool::ConstPtr _sedgeRootsLive;
		flint::IPool::ConstPtr _featherMossLive;
		flint::IPool::ConstPtr _sphagnumMossLive;
		
		//peatland age variable, peatland age may be very old
		flint::IVariable* _peatlandAge;

		// the growth parameters associated to this peatland unit
		std::shared_ptr<PeatlandGrowthParameters> growthParas;

		// the turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;

		// the peatland growth curve, and store it
		std::shared_ptr<PeatlandGrowthcurve> growthCurve;
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLGROWTH_H_
