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

		std::shared_ptr<StandGrowthCurve> createStandGrowthCurve(Int64 standGrowthCurveID);

		std::shared_ptr<VolumeToBiomassCarbonGrowth> getVolumeToBiomassGrowth() const { return _volumeToBioGrowth; }

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
		Int64 _standGrowthCurveID;

		std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;		
		
		void handleGrowthLoss(std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement, 
			std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement);

		std::shared_ptr<OvermatureDeclineLosses> getOvermatrueDeclineLosses(
			double merchCarbonChanges, double foliageCarbonChanges, double otherCarbonChanges,
			double coarseRootCarbonChanges, double fineRootCarbonChanges);

		void doHalfGrowth();
		void doTurnover();
		void updateBiomassPools();
		void updateBiomassPoolsAfterGrowth();
		void addbackBiomassTurnoverAmount();		

		void printPoolValuesAtStep(int age);
		void printTurnoverRate();

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