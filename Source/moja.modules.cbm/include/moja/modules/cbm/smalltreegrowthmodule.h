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

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API SmallTreeGrowthModule : public CBMModuleBase {
	public:
		SmallTreeGrowthModule(){};
		virtual ~SmallTreeGrowthModule() {};

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

        void doLocalDomainInit() override;
        void doTimingInit() override;
        void doTimingStep() override;	

        void getYieldCurve();
    private:
		const flint::IPool* _atmosphere;

		//softwood small tree growth curve component
		std::shared_ptr<SmallTreeGrowthCurve> _smallTreeGrowthSW;
        const flint::IPool* _softwoodStem;
        const flint::IPool* _softwoodOther;
        const flint::IPool* _softwoodFoliage;
        const flint::IPool* _softwoodCoarseRoots;
        const flint::IPool* _softwoodFineRoots;
		const flint::IPool* _softwoodStemSnag;
		const flint::IPool* _softwoodBranchSnag;

		//hardwood small tree growth curve compoment
		std::shared_ptr<SmallTreeGrowthCurve> _smallTreeGrowthHW;
		const flint::IPool* _hardwoodStem;
		const flint::IPool* _hardwoodOther;
		const flint::IPool* _hardwoodFoliage;
		const flint::IPool* _hardwoodCoarseRoots;
		const flint::IPool* _hardwoodFineRoots;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;	
      
		const flint::IPool* _woodyFoliageDead;
		const flint::IPool* _woodyFineDead;
		const flint::IPool* _woodyRootsDead;
		const flint::IVariable* _turnoverRates;

		flint::IVariable* _smalltree_age;
        flint::IVariable* _gcId;       
       
        flint::IVariable* _regenDelay;
        flint::IVariable* _spinupMossOnly;
        flint::IVariable* _isForest;
        flint::IVariable* _isDecaying;	
		
		void getIncrements();	
		void doHalfGrowth() const;
		void doPeatlandTurnover() const;
        void updateBiomassPools();
        void doMidSeasonGrowth() const;
        bool shouldRun();	
			
		bool _treedPeatland {false};
		bool _shouldRun {false};

		// biomass and snag turnover rate/parameters
		double _softwoodFoliageFallRate{ 0 };
		double _hardwoodFoliageFallRate{ 0 };
		double _stemAnnualTurnOverRate{ 0 };
		double _softwoodBranchTurnOverRate{ 0 };
		double _hardwoodBranchTurnOverRate{ 0 };
		double _otherToBranchSnagSplit{ 0 };
		double _stemSnagTurnoverRate{ 0 };
		double _branchSnagTurnoverRate{ 0 };
		double _coarseRootSplit{ 0 };
		double _coarseRootTurnProp{ 0 };
		double _fineRootAGSplit{ 0 };
		double _fineRootTurnProp{ 0 };

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

		void printRemovals(int age, double standSoftwoodStem, double standSoftwoodFoliage, double standSoftwoodOther, double standSWCoarseRootsCarbon, double standSWFineRootsCarbon);
    };
}}}
#endif