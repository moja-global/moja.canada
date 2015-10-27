#ifndef MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_
#define MOJA_MODULES_CBM_YIELDTABLEGROWTHMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurve.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API YieldTableGrowthModule : public moja::flint::ModuleBase {
	public:
		YieldTableGrowthModule() {};
		virtual ~YieldTableGrowthModule() {};	

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes ModuleType() { return flint::ModuleTypes::Model; };

		void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
		void onTimingStep(const flint::TimingStepNotification::Ptr& n) override;	
		void onTimingInit(const flint::TimingInitNotification::Ptr&) override;
		void onTimingPreEndStep(const flint::TimingPreEndStepNotification::Ptr& n) override;

		std::shared_ptr<StandGrowthCurve> createStandGrowthCurve(Int64 standGrowthCurveID);

		std::shared_ptr<VolumeToBiomassCarbonGrowth> getVolumeToBiomassGrowth() const { return _volumeToBioGrowth; }

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

		const flint::ConstPtr _aboveGroundVeryFastSoil;
		const flint::ConstPtr _aboveGroundFastSoil;
		const flint::ConstPtr _belowGroundVeryFastSoil;
		const flint::ConstPtr _belowGroundFastSoil;
		const flint::ConstPtr _softwoodStemSnag;
		const flint::ConstPtr _softwoodBranchSnag;
		const flint::ConstPtr _hardwoodStemSnag;
		const flint::ConstPtr _hardwoodBranchSnag;
		const flint::ConstPtr _mediumSoil;
		const flint::ConstPtr _atmosphere;		

		flint::IVariable* _age;
		Int64 _standGrowthCurveID;

		std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
		
		void handleGrowthLoss(std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement, std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement);
		std::shared_ptr<OvermatureDeclineLosses> getOvermatrueDeclineLosses(
			double merchCarbonChanges, double foliageCarbonChanges, double otherCarbonChanges,
			double coarseRootCarbonChanges, double fineRootCarbonChanges);

		void doGrowth();
		void doTurnover();
		void updateBiomassPools();
		void updateBiomassPoolsAfterGrowth();
		void addbackBiomassTurnoverAmount();
		void updateBioPoolsAfterGrowthLoss(std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement, std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement);	
		void printPoolValuesAtStep(int age);

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