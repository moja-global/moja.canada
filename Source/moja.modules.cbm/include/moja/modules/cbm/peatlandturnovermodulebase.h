#ifndef MOJA_MODULES_CBM_PLTURNOVER_BASE_H_
#define MOJA_MODULES_CBM_PLTURNOVER_BASE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API PeatlandTurnoverModuleBase : public CBMModuleBase {
			public:
				PeatlandTurnoverModuleBase() : CBMModuleBase() { }
				virtual ~PeatlandTurnoverModuleBase() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

			protected:
				const flint::IPool* _atmosphere = nullptr;
				const flint::IPool* _woodyFoliageLive = nullptr;;
				const flint::IPool* _woodyStemsBranchesLive = nullptr;;
				const flint::IPool* _woodyRootsLive = nullptr;;
				const flint::IPool* _sedgeFoliageLive = nullptr;;
				const flint::IPool* _sedgeRootsLive = nullptr;;
				const flint::IPool* _featherMossLive = nullptr;;
				const flint::IPool* _sphagnumMossLive = nullptr;;
				const flint::IPool* _woodyFineDead = nullptr;;
				const flint::IPool* _woodyCoarseDead = nullptr;;
				const flint::IPool* _woodyFoliageDead = nullptr;;
				const flint::IPool* _woodyRootsDead = nullptr;;
				const flint::IPool* _sedgeFoliageDead = nullptr;;
				const flint::IPool* _sedgeRootsDead = nullptr;;
				const flint::IPool* _feathermossDead = nullptr;;
				const flint::IPool* _acrotelm_o = nullptr;;
				const flint::IPool* _catotelm_a = nullptr;;
				const flint::IPool* _acrotelm_a = nullptr;;
				const flint::IPool* _catotelm_o = nullptr;;

				flint::IVariable* _regenDelay = nullptr;

				//peatland shrub age variable, which may be very old
				flint::IVariable* _shrubAge = nullptr;;

				// the turnover parameters associated to this peatland unit
				std::shared_ptr<PeatlandTurnoverParameters> turnoverParas = nullptr;;

				// the growth parameters associated to this peatland unit
				std::shared_ptr<PeatlandGrowthParameters> growthParas = nullptr;;

				DynamicObject baseWTDParameters;

				//current peatland pool value
				double woodyFoliageLive{ 0 };
				double woodyStemsBranchesLive{ 0 };
				double woodyRootsLive{ 0 };
				double sedgeFoliageLive{ 0 };
				double sedgeRootsLive{ 0 };
				double featherMossLive{ 0 };
				double sphagnumMossLive{ 0 };

				bool _runPeatland{ false };
				int _peatlandId{ -1 };

				void updatePeatlandLivePoolValue();
				void doLivePoolTurnover();

				double computeWaterTableDepth(double dc, int peatlandID);
				double computeCarbonTransfers(double previousAwtd, double currentAwtd, double a, double b);
			};
		}
	}
} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLTURNOVER_H_
