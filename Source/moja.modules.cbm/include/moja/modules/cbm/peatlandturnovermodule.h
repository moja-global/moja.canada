#ifndef MOJA_MODULES_CBM_PLTURNOVER_H_
#define MOJA_MODULES_CBM_PLTURNOVER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
namespace modules {
namespace cbm {
		
	class CBM_API PeatlandTurnoverModule : public CBMModuleBase {
	public:
		PeatlandTurnoverModule() : CBMModuleBase() { }
		virtual ~PeatlandTurnoverModule() = default;

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
		const flint::IPool* _woodyFineDead;
		const flint::IPool* _woodyCoarseDead;
		const flint::IPool* _woodyFoliageDead;
		const flint::IPool* _woodyRootsDead;
		const flint::IPool* _sedgeFoliageDead;
		const flint::IPool* _sedgeRootsDead;
		const flint::IPool* _feathermossDead;
		const flint::IPool* _acrotelm_o;
		const flint::IPool* _catotelm_a;
		const flint::IPool* _acrotelm_a;
		const flint::IPool* _catotelm_o;	

		//peatland shrub age variable, which may be very old
		flint::IVariable* _shrubAge;
		
		// the turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;	

		// the growth parameters associated to this peatland unit
		std::shared_ptr<PeatlandGrowthParameters> growthParas;

		//current peatland pool value
		double woodyFoliageLive{ 0 };
		double woodyStemsBranchesLive{ 0 };
		double woodyRootsLive{ 0 };
		double sedgeFoliageLive{ 0 };
		double sedgeRootsLive{ 0 };
		double featherMossLive{ 0 };
		double sphagnumMossLive{ 0 };
		bool _runPeatland{ false };

		double computeCarbonTransfers(double previousAwtd, double currentAwtd, double a, double b);

		void updatePeatlandLivePoolValue();
		void doLivePoolTurnover();
		void doWaterTableFlux();		
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLTURNOVER_H_
