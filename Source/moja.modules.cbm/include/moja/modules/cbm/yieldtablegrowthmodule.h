#ifndef MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_
#define MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API YieldTableGrowthModule : public CBMModuleBase {
	public:
		YieldTableGrowthModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory)
			: _gcFactory(gcFactory) {};

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

		const flint::IPool* _aboveGroundVeryFastSoil;
		const flint::IPool* _aboveGroundFastSoil;
		const flint::IPool* _belowGroundVeryFastSoil;
		const flint::IPool* _belowGroundFastSoil;
		const flint::IPool* _softwoodStemSnag;
		const flint::IPool* _softwoodBranchSnag;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;
		const flint::IPool* _mediumSoil;
        const flint::IPool* _atmosphere;	

		flint::IVariable* _age;
        flint::IVariable* _gcId;
        flint::IVariable* _spuId;
        flint::IVariable* _turnoverRates;
        flint::IVariable* _regenDelay;
        flint::IVariable* _spinupMossOnly;
        flint::IVariable* _isForest;
        flint::IVariable* _isDecaying;
		flint::IVariable* _growthMultipliers;
			
		bool _growthMultipliersEnabled;
        bool _smootherEnabled = true;

		Int64 _standGrowthCurveID;
        Int64 _standSPUID;

        std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
		std::shared_ptr<StandGrowthCurveFactory> _gcFactory;
		
		void getIncrements();
		void initPeatland();
		void doHalfGrowth() const;
        void doTurnover() const;
        void updateBiomassPools();
        void doMidSeasonGrowth() const;
        bool shouldRun() const;
		bool _skipForPeatland;

		// biomass and snag turnover rate/parameters
		double _softwoodFoliageFallRate;
		double _hardwoodFoliageFallRate;
		double _stemAnnualTurnOverRate;
		double _softwoodBranchTurnOverRate;
		double _hardwoodBranchTurnOverRate;
		double _otherToBranchSnagSplit;
		double _stemSnagTurnoverRate;
		double _branchSnagTurnoverRate;
		double _coarseRootSplit;
		double _coarseRootTurnProp;
		double _fineRootAGSplit;
		double _fineRootTurnProp;

        // record of the biomass carbon growth increment
        double swm;
        double swo;
        double swf;
        double hwm;
        double hwo;
        double hwf;
        double swcr;
        double swfr;
        double hwcr;
        double hwfr;

        // record of the current biomass and snag pool value
        double standSoftwoodMerch;
        double standSoftwoodOther;
        double standSoftwoodFoliage;
        double standSWCoarseRootsCarbon;
        double standSWFineRootsCarbon;
        double standHardwoodMerch;
        double standHardwoodOther;
        double standHardwoodFoliage;
        double standHWCoarseRootsCarbon;
        double standHWFineRootsCarbon;
        double softwoodStemSnag;
        double softwoodBranchSnag;
        double hardwoodStemSnag;
        double hardwoodBranchSnag;
    };

}}}
#endif
