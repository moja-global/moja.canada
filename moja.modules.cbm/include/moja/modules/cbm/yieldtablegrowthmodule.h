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

		flint::IPool::ConstPtr _atmosphere;		

		flint::IVariable* _age;

		std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
		Int64 _standGrowthCurveID;
	};

}}}
#endif