#ifndef MOJA_MODULES_CBM_PLSPINUPNEXT_H_
#define MOJA_MODULES_CBM_PLSPINUPNEXT_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlanddecayparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"
#include "moja/modules/cbm/peatlandfireparameters.h"
#include "moja/modules/cbm/peatlands.h"

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
		//void doTimingInit() override;	
		void doPrePostDisturbanceEvent() override;

	private:	
		const flint::IPool* _softwoodMerch;
		const flint::IPool* _softwoodOther;
		const flint::IPool* _softwoodFoliage;
		const flint::IPool* _softwoodCoarseRoots;
		const flint::IPool* _softwoodFineRoots;
		const flint::IPool* _softwoodStemSnag;	
		const flint::IPool* _softwoodBranchSnag;

		const flint::IPool* _hardwoodMerch;
		const flint::IPool* _hardwoodOther;
		const flint::IPool* _hardwoodFoliage;
		const flint::IPool* _hardwoodCoarseRoots;
		const flint::IPool* _hardwoodFineRoots;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;

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
		const flint::IPool* _woodyCoarseDead;
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
		double _stemSnagTurnoverRate{ 0 };
		double _softwoodBranchTurnOverRate{ 0 };
		double _hardwoodBranchTurnOverRate{ 0 };
		double _coarseRootTurnProp{ 0 };
		double _fineRootTurnProp{ 0 };
		double _otherToBranchSnagSplit{ 0 };
		double _branchSnagTurnoverRate{ 0 };
		

		int smallTreeOn{ 0 };
		double smallTreeFoliageRemoval{ 0 };
		double smallTreeFineRootRemoval{ 0 };
		double smallTreeCoarseRootRemoval{ 0 };
		double smallTreeOtherRemovalToWFD{ 0 };
		double smallTreeBranchSnagRemoval{ 0 };
		double smallTreeStemSnagRemoval{ 0 };

		int largeTreeOn{ 0 };
		double largeTreeFoliageRemoval{ 0 };
		double largeTreeFineRootRemoval{ 0 };
		double largeTreeCoarseRootRemoval{ 0 };		
		double largeTreeOtherRemovalToWFD{ 0 };
		double largeTreeBranchSnagRemoval{ 0 };
		double largeTreeStemSnagRemoval{ 0 };

		double meanAnnualTemperature{ 0 };
		double f_r{ 1 };
		double f_fr{ 1 };

		// decay parameters associated to this peatland unit
		std::shared_ptr<PeatlandDecayParameters> decayParas;	

		// turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;	

		// the growth parameters associated to this peatland unit
		std::shared_ptr<PeatlandGrowthParameters> growthParas;

		// the peatland fire parameter
		std::shared_ptr<PeatlandFireParameters> fireParas;

		void getTreeTurnoverRate(Peatlands peatlandId);

		void getAndUpdateParameter();

		void getNonOpenPeatlandRemovals(Peatlands peatlandId);

		void populatePeatlandDeadPoolsV1();

		void populatePeatlandDeadPoolsV2();

		void populatePeatlandDeadPoolsV3();

		void getCurrentDeadPoolValues();

		void resetSlowPools();

		inline double modifyQ10(double baseQ10Para) {			
			return (pow(baseQ10Para, 0.1 * (meanAnnualTemperature - 10)));
		};		
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLDECAY_H_

