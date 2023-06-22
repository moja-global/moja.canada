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
				const flint::IPool* _atmosphere = nullptr;
				const flint::IPool* _woodyFoliageLive = nullptr;
				const flint::IPool* _woodyStemsBranchesLive = nullptr;
				const flint::IPool* _woodyRootsLive = nullptr;
				const flint::IPool* _sedgeFoliageLive = nullptr;
				const flint::IPool* _sedgeRootsLive = nullptr;
				const flint::IPool* _featherMossLive = nullptr;
				const flint::IPool* _sphagnumMossLive = nullptr;
				flint::IVariable* _regenDelay = nullptr;
				flint::IVariable* _spinupMossOnly = nullptr;

				flint::IVariable* _midSeaonFoliageTurnover = nullptr;
				flint::IVariable* _midSeaonStemBranchTurnover = nullptr;

				// peatland woody layer shrub age
				flint::IVariable* _shrubAge = nullptr;

				// peatland moss age
				flint::IVariable* _mossAge = nullptr;

				// the growth parameters associated to this peatland unit
				std::shared_ptr<PeatlandGrowthParameters> growthParas;

				// the turnover parameters associated to this peatland unit
				std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;

				// the peatland growth curve, and store it
				std::shared_ptr<PeatlandGrowthcurve> growthCurve;

				bool _runPeatland{ false };
				int _peatlandId{ -1 };

				//current peatland pool value
				double woodyFoliageLive{ 0 };
				double woodyStemsBranchesLive{ 0 };
				double woodyRootsLive{ 0 };
				double sedgeFoliageLive{ 0 };
				double sedgeRootsLive{ 0 };
				double featherMossLive{ 0 };
				double sphagnumMossLive{ 0 };

				void updateParameters();
				void updateLivePool();
				void doNormalGrowth(int shrubAge, int mossAge);
				void doMidseasonGrowth(int shrubAge);
			};
		}
	}
} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLGROWTH_H_
