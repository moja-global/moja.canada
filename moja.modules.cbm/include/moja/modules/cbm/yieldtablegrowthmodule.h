#ifndef MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_
#define MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API YieldTableGrowthModule : public moja::flint::ModuleBase {
	public:
		YieldTableGrowthModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory)
			: _gcFactory(gcFactory) {};

		virtual ~YieldTableGrowthModule() {};	

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

        void onLocalDomainInit() override;
        void onTimingInit() override;
        void onTimingStep() override;	

        void getYieldCurve();
        std::shared_ptr<StandGrowthCurve> createStandGrowthCurve(
            Int64 standGrowthCurveID, Int64 spuID) const;

    private:
        flint::IPool::ConstPtr _softwoodMerch;
        flint::IPool::ConstPtr _softwoodOther;
        flint::IPool::ConstPtr _softwoodFoliage;
        flint::IPool::ConstPtr _softwoodCoarseRoots;
        flint::IPool::ConstPtr _softwoodFineRoots;

		flint::IPool::ConstPtr _hardwoodMerch;
		flint::IPool::ConstPtr _hardwoodOther;
		flint::IPool::ConstPtr _hardwoodFoliage;
		flint::IPool::ConstPtr _hardwoodCoarseRoots;
		flint::IPool::ConstPtr _hardwoodFineRoots;

		flint::IPool::ConstPtr _aboveGroundVeryFastSoil;
		flint::IPool::ConstPtr _aboveGroundFastSoil;
		flint::IPool::ConstPtr _belowGroundVeryFastSoil;
		flint::IPool::ConstPtr _belowGroundFastSoil;
		flint::IPool::ConstPtr _softwoodStemSnag;
		flint::IPool::ConstPtr _softwoodBranchSnag;
		flint::IPool::ConstPtr _hardwoodStemSnag;
		flint::IPool::ConstPtr _hardwoodBranchSnag;
		flint::IPool::ConstPtr _mediumSoil;
        flint::IPool::ConstPtr _atmosphere;	

		flint::IVariable* _age;
        flint::IVariable* _gcId;
        flint::IVariable* _spuId;
        flint::IVariable* _turnoverRates;
        flint::IVariable* _regenDelay;

		Int64 _standGrowthCurveID;
        Int64 _standSPUID;

        std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
		std::shared_ptr<StandGrowthCurveFactory> _gcFactory;
		
        void doHalfGrowth() const;
        void doTurnover() const;
        void updateBiomassPools();
        void doMidSeasonGrowth() const;

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
