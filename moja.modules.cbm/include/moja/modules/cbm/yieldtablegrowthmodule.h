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

		const flint::IPool* _atmosphere;		

		flint::IVariable* _age;

		//make sure it singleton, but can be referenced
		std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
		Int64 _standGrowthCurveID;
	};
}}}
#endif