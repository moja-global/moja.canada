#ifndef MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_
#define MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/turnoverrates.h"
#include "moja/modules/cbm/peatlands.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API YieldTableGrowthModule : public CBMModuleBase {
	public:
		YieldTableGrowthModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory, std::shared_ptr<VolumeToBiomassCarbonGrowth> volumeToBioGrowth)
			: _gcFactory(gcFactory), _volumeToBioGrowth(volumeToBioGrowth) {};

		virtual ~YieldTableGrowthModule() {};	

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

        void doLocalDomainInit() override;
        void doTimingInit() override;
        void doTimingStep() override;	

        void getYieldCurve();
        std::shared_ptr<StandGrowthCurve> createStandGrowthCurve(
            Int64 standGrowthCurveID, Int64 spuID) const;

    private:
        const flint::IPool* _softwoodMerch = nullptr;
        const flint::IPool* _softwoodOther = nullptr;
        const flint::IPool* _softwoodFoliage = nullptr;
        const flint::IPool* _softwoodCoarseRoots = nullptr;
        const flint::IPool* _softwoodFineRoots = nullptr;

		const flint::IPool* _hardwoodMerch = nullptr;
		const flint::IPool* _hardwoodOther = nullptr;
		const flint::IPool* _hardwoodFoliage = nullptr;
		const flint::IPool* _hardwoodCoarseRoots = nullptr;
		const flint::IPool* _hardwoodFineRoots = nullptr;

		const flint::IPool* _aboveGroundVeryFastSoil = nullptr;
		const flint::IPool* _aboveGroundFastSoil = nullptr;
		const flint::IPool* _belowGroundVeryFastSoil = nullptr;
		const flint::IPool* _belowGroundFastSoil = nullptr;
		const flint::IPool* _softwoodStemSnag = nullptr;
		const flint::IPool* _softwoodBranchSnag = nullptr;
		const flint::IPool* _hardwoodStemSnag = nullptr;
		const flint::IPool* _hardwoodBranchSnag = nullptr;
		const flint::IPool* _mediumSoil = nullptr;
        const flint::IPool* _atmosphere = nullptr;	

		const flint::IPool* _woodyFoliageDead = nullptr;
		const flint::IPool* _woodyFineDead = nullptr;
		const flint::IPool* _woodyCoarseDead = nullptr;
		const flint::IPool* _woodyRootsDead = nullptr;		

		flint::IVariable* _age = nullptr;
        flint::IVariable* _gcId = nullptr;
        flint::IVariable* _spuId = nullptr;
        flint::IVariable* _turnoverRates = nullptr;
        flint::IVariable* _regenDelay = nullptr;
        flint::IVariable* _spinupMossOnly = nullptr;
        flint::IVariable* _isForest = nullptr;
        flint::IVariable* _isDecaying = nullptr;
		flint::IVariable* _growthMultipliers = nullptr;
		flint::IVariable* _output_removal = nullptr;
			
		bool _growthMultipliersEnabled = true;
        bool _smootherEnabled = true;
        bool _debuggingEnabled = false;
        std::string _debuggingOutputPath = ".";

		Int64 _standGrowthCurveID{ -1 };
		Int64 _standSPUID{ -1 };

        std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth = nullptr;
		std::shared_ptr<StandGrowthCurveFactory> _gcFactory = nullptr;
		
		void getIncrements();
        void getTurnoverRates();
		void initPeatland();
		void doHalfGrowth() const;	
        void doTurnover() const;		
        void updateBiomassPools();
        void doMidSeasonGrowth() const;
        bool shouldRun() const;

		void switchTurnover() const;
		void doPeatlandTurnover() const;
		void switchHalfGrowth() const;
		void doPeatlandHalfGrowth() const;

		bool _skipForPeatland{ false };
		bool _runForForestedPeatland{ false };

		// biomass and snag turnover rate/parameters
        std::unordered_map<std::tuple<Int64, Int64>, std::shared_ptr<TurnoverRates>> _cachedTurnoverRates;
        std::shared_ptr<TurnoverRates> _currentTurnoverRates;

        // record of the biomass carbon growth increment
        double swm = 0;
        double swo = 0;
        double swf = 0;
        double hwm = 0;
        double hwo = 0;
        double hwf = 0;
        double swcr = 0;
        double swfr = 0;
        double hwcr = 0;
        double hwfr = 0;

        // record of the current biomass and snag pool value
        double standSoftwoodMerch = 0;
        double standSoftwoodOther = 0;
        double standSoftwoodFoliage = 0;
        double standSWCoarseRootsCarbon = 0;
        double standSWFineRootsCarbon = 0;
        double standHardwoodMerch = 0;
        double standHardwoodOther = 0;
        double standHardwoodFoliage = 0;
        double standHWCoarseRootsCarbon = 0;
        double standHWFineRootsCarbon = 0;
        double softwoodStemSnag = 0;
        double softwoodBranchSnag = 0;
        double hardwoodStemSnag = 0;
        double hardwoodBranchSnag = 0;

		void printRemovals(int standAge,
			double standFoliageRemoval,
			double standStemSnagRemoval,
			double standBranchSnagRemoval,
			double standOtherRemovalToWFD,
			double standCoarseRootsRemoval,
			double standFineRootsRemoval,
			double standOtherRemovalToBranchSnag) const;
    };
}}}
#endif