#ifndef MOJA_MODULES_CBM_SMALLTREEROWTHMODULE_H_
#define MOJA_MODULES_CBM_SMALLTREEGROWTHMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/smalltreegrowthcurve.h"
#include "moja/modules/cbm/turnoverrates.h"
#include "moja/modules/cbm/peatlands.h"

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API SmallTreeGrowthModule : public CBMModuleBase {
			public:
				SmallTreeGrowthModule() {};
				virtual ~SmallTreeGrowthModule() {};

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;

				void getYieldCurve();
			private:
				const flint::IPool* _atmosphere{ nullptr };

				//softwood small tree growth curve component
				std::shared_ptr<SmallTreeGrowthCurve> _smallTreeGrowthSW = nullptr;
				const flint::IPool* _softwoodStem{ nullptr };
				const flint::IPool* _softwoodOther{ nullptr };
				const flint::IPool* _softwoodFoliage{ nullptr };
				const flint::IPool* _softwoodCoarseRoots{ nullptr };
				const flint::IPool* _softwoodFineRoots{ nullptr };
				const flint::IPool* _softwoodStemSnag{ nullptr };
				const flint::IPool* _softwoodBranchSnag{ nullptr };

				//hardwood small tree growth curve compoment
				std::shared_ptr<SmallTreeGrowthCurve> _smallTreeGrowthHW{ nullptr };
				const flint::IPool* _hardwoodStem{ nullptr };
				const flint::IPool* _hardwoodOther{ nullptr };
				const flint::IPool* _hardwoodFoliage{ nullptr };
				const flint::IPool* _hardwoodCoarseRoots{ nullptr };
				const flint::IPool* _hardwoodFineRoots{ nullptr };
				const flint::IPool* _hardwoodStemSnag{ nullptr };
				const flint::IPool* _hardwoodBranchSnag{ nullptr };

				const flint::IPool* _woodyFoliageDead{ nullptr };
				const flint::IPool* _woodyFineDead{ nullptr };
				const flint::IPool* _woodyRootsDead{ nullptr };

				flint::IVariable* _turnoverRates{ nullptr };
				flint::IVariable* _spuId{ nullptr };
				flint::IVariable* _smalltreeAge{ nullptr };
				flint::IVariable* _regenDelay{ nullptr };
				flint::IVariable* _spinupMossOnly{ nullptr };
				flint::IVariable* _isForest{ nullptr };
				flint::IVariable* _isDecaying{ nullptr };
				flint::IVariable* _outputRemoval{ nullptr };
				flint::IVariable* _ecoBoundary{ nullptr };
				flint::IVariable* _blackSpruceGCID{ nullptr };
				flint::IVariable* _smallTreeGCParameters{ nullptr };
				flint::IVariable* _appliedGrowthCurveID{ nullptr };

				void getIncrements();
				void doHalfGrowth() const;
				void doPeatlandTurnover() const;
				void updateBiomassPools();
				void doMidSeasonGrowth() const;
				bool shouldRun();

				bool _shouldRun{ false };
				int _peatlandId{ -1 };

				// record of the biomass carbon growth increment
				double sws{ 0 }; // stem wood
				double swo{ 0 };
				double swf{ 0 };
				double hws{ 0 }; // stem wood
				double hwo{ 0 };
				double hwf{ 0 };
				double swcr{ 0 };
				double swfr{ 0 };
				double hwcr{ 0 };
				double hwfr{ 0 };

				// record of the current biomass and snag pool value
				double standSoftwoodStem{ 0 };
				double standSoftwoodOther{ 0 };
				double standSoftwoodFoliage{ 0 };
				double standSWCoarseRootsCarbon{ 0 };
				double standSWFineRootsCarbon{ 0 };
				double standHardwoodStem{ 0 };
				double standHardwoodOther{ 0 };
				double standHardwoodFoliage{ 0 };
				double standHWCoarseRootsCarbon{ 0 };
				double standHWFineRootsCarbon{ 0 };
				double standSoftwoodStemSnag{ 0 };
				double standSoftwoodBranchSnag{ 0 };
				double standHardwoodStemSnag{ 0 };
				double standHardwoodBranchSnag{ 0 };

				// biomass and snag turnover rate/parameters
				std::unordered_map<std::tuple<Int64, Int64>, std::shared_ptr<TurnoverRates>> _cachedTurnoverRates;
				std::shared_ptr<TurnoverRates> _currentTurnoverRates;

				void getTurnoverRates(int smalltreeGCID, int spuID);

				void printRemovals(int standSmallTreeAge,
					double smallTreeFoliageRemoval,
					double smallTreeStemSnagRemoval,
					double smallTreeBranchSnagRemoval,
					double smallTreeOtherRemovalToWFD,
					double smallTreeCoarseRootRemoval,
					double smallTreeFineRootRemoval,
					double smallTreeOtherToBranchSnag,
					double smallTreeStemRemoval);
    };
}}}
#endif