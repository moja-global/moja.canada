#ifndef MOJA_MODULES_CBM_PLTURNOVER_H_
#define MOJA_MODULES_CBM_PLTURNOVER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
namespace modules {
namespace cbm {
		
	class CBM_API PeatlandTurnoverModule : public flint::ModuleBase {
	public:
		PeatlandTurnoverModule() : ModuleBase() { }
		virtual ~PeatlandTurnoverModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;		

		void onLocalDomainInit() override;
		void onTimingInit() override;
		void onTimingStep() override;

	private:	
		flint::IPool::ConstPtr _atmosphere;
		flint::IPool::ConstPtr _woodyFoliageLive;
		flint::IPool::ConstPtr _woodyStemsBranchesLive;
		flint::IPool::ConstPtr _woodyRootsLive;
		flint::IPool::ConstPtr _sedgeFoliageLive;
		flint::IPool::ConstPtr _sedgeRootsLive;
		flint::IPool::ConstPtr _featherMossLive;
		flint::IPool::ConstPtr _sphagnumMossLive;
		flint::IPool::ConstPtr _woodyStemsBranchesDead;
		flint::IPool::ConstPtr _woodyFoliageDead;
		flint::IPool::ConstPtr _woodyRootsDead;
		flint::IPool::ConstPtr _sedgeFoliageDead;
		flint::IPool::ConstPtr _sedgeRootsDead;
		flint::IPool::ConstPtr _feathermossDead;
		flint::IPool::ConstPtr _acrotelm_o;
		flint::IPool::ConstPtr _catotelm_a;
		flint::IPool::ConstPtr _acrotelm_a;
		flint::IPool::ConstPtr _catotelm_o;
		
		//peatland age variable, peatland age may be very old
		flint::IVariable* _peatlandAge;

		// the turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;	

		// the growth parameters associated to this peatland unit
		std::shared_ptr<PeatlandGrowthParameters> growthParas;

		//current peatland pool value
		double woodyFoliageLive;
		double woodyStemsBranchesLive;
		double woodyRootsLive;
		double sedgeFoliageLive;
		double sedgeRootsLive;
		double featherMossLive;
		double sphagnumMossLive;

		void updatePeatlandLivePoolValue();
		void doLivePoolTurnover();
		void doWaterTableFlux();
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLTURNOVER_H_
