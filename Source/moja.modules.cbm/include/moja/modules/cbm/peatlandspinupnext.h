#ifndef MOJA_MODULES_CBM_PLSPINUPNEXT_H_
#define MOJA_MODULES_CBM_PLSPINUPNEXT_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlanddecayparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"
#include "moja/modules/cbm/peatlandfireparameters.h"

namespace moja {
namespace modules {
namespace cbm {
	
	/*
	After regular spinup procedure, this class is called to quicly build up the peat DOM pools.
	This class is implemented based on R-code
	*/
	class CBM_API PeatlandSpinupNext : public CBMModuleBase {
	public:
		PeatlandSpinupNext() : CBMModuleBase() { }
		virtual ~PeatlandSpinupNext() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;	

		void doLocalDomainInit() override;
		void doTimingInit() override;	

	private:	
		const flint::IPool* _softwoodMerch;
		const flint::IPool* _softwoodOther;
		const flint::IPool* _softwoodFoliage;
		const flint::IPool* _softwoodCoarseRoots;
		const flint::IPool* _softwoodFineRoots;

		const flint::IPool* _hardwoodMerch;
		const flint::IPool* _hardwoodOther;
		const flint::IPool* _hardwoodFoliage;
		const flint::IPool* _hardwoodCoarseRoots;
		const flint::IPool* _hardwoodFineRoots;

		const flint::IPool* _softwoodStem;
		const flint::IPool* _hardwoodStem;

		const flint::IPool* _woodyFoliageLive;
		const flint::IPool* _woodyStemsBranchesLive;
		const flint::IPool* _woodyRootsLive;
		const flint::IPool* _sedgeFoliageLive;
		const flint::IPool* _sedgeRootsLive;
		const flint::IPool* _featherMossLive;
		const flint::IPool* _sphagnumMossLive;

		const flint::IPool* _woodyFoliageDead;
		const flint::IPool* _woodyFineDead;	
		const flint::IPool* _woodyRootsDead;
		const flint::IPool* _sedgeFoliageDead;
		const flint::IPool* _sedgeRootsDead;
		const flint::IPool* _feathermossDead;
		const flint::IPool* _acrotelm_o;
		const flint::IPool* _catotelm_a;
		const flint::IPool* _acrotelm_a;
		const flint::IPool* _catotelm_o;	

		const flint::IPool* _atmosphere;

		const flint::IVariable* _turnoverRates;

		double _softwoodFoliageFallRate{ 0 };
		double _hardwoodFoliageFallRate{ 0 };
		double _stemAnnualTurnOverRate{ 0 };
		double _softwoodBranchTurnOverRate{ 0 };
		double _hardwoodBranchTurnOverRate{ 0 };
		double _coarseRootTurnProp{ 0 };
		double _fineRootTurnProp{ 0 };

		int smallTreeOn{ 0 };
		double smallTreeFoliage{ 0 };
		double smallTreeFineRoot{ 0 };
		double smallTreeCoarseRoot{ 0 };
		double smallTreeOther{ 0 };
		double smallTreeStem{ 0 };

		int largeTreeOn{ 0 };
		double largeTreeFoliage{ 0 };
		double largeTreeFineRoot{ 0 };
		double largeTreeCoarseRoot{ 0 };
		double largeTreeMerchant{ 0 };
		double largeTreeOther{ 0 };

		double meanAnnualTemperature{ 0 };
		double fireReturnReciprocal{ 1 };

		// decay parameters associated to this peatland unit
		std::shared_ptr<PeatlandDecayParameters> decayParas;	

		// turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;	

		// the growth parameters associated to this peatland unit
		std::shared_ptr<PeatlandGrowthParameters> growthParas;

		// the peatland fire parameter
		std::shared_ptr<PeatlandFireParameters> fireParas;

		void getTreeTurnoverRate();

		void getAndUpdateParameter();

		void preparePeatlandSpinupSpeedup(int peatlandId);

		void populatePeatlandDeadPools();

		void getCurrentDeadPoolValues();

		void resetSlowPools();

		inline double modifyQ10(double baseQ10Para) {			
			return (pow(baseQ10Para, 0.1 * (meanAnnualTemperature - 10)));
		};		
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLDECAY_H_

