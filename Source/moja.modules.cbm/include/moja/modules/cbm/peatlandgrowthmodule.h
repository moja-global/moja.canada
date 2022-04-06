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
				const flint::IPool* _atmosphere;
				const flint::IPool* _woodyFoliageLive;
				const flint::IPool* _woodyStemsBranchesLive;
				const flint::IPool* _woodyRootsLive;
				const flint::IPool* _sedgeFoliageLive;
				const flint::IPool* _sedgeRootsLive;
				const flint::IPool* _featherMossLive;
				const flint::IPool* _sphagnumMossLive;
				flint::IVariable* _regenDelay;
				flint::IVariable* _spinupMossOnly;

				// peatland woody layer shrub age
				flint::IVariable* _shrubAge;

				// the growth parameters associated to this peatland unit
				std::shared_ptr<PeatlandGrowthParameters> growthParas;

				// the turnover parameters associated to this peatland unit
				std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;

				// the peatland growth curve, and store it
				std::shared_ptr<PeatlandGrowthcurve> growthCurve;

				bool _runPeatland{ false };
				int _peatlandId{ -1 };
			};
		}
	}
} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLGROWTH_H_
