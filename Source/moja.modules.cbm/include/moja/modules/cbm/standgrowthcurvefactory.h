#ifndef MOJA_MODULES_CBM_MOSSPARAMETERS_H_
#define MOJA_MODULES_CBM_MOSSPARAMETERS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/modules/cbm/standgrowthcurve.h"

#include <Poco/LRUCache.h>

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// Singleton factory class to create a stand growth curve.
	/// This object will be instantiated in module factory, and be 
	/// injected to other objects that requires the stand growth factory.
	/// </summary>	
	class CBM_API StandGrowthCurveFactory {

	public:
		StandGrowthCurveFactory();
		virtual ~StandGrowthCurveFactory() = default;

		Poco::SharedPtr<StandGrowthCurve> createStandGrowthCurve(Int64 standGrowthCurveID, Int64 spuID, flint::ILandUnitDataWrapper& landUnitData);		
		Poco::SharedPtr<StandGrowthCurve> getStandGrowthCurve(Int64 growthCurveID);

	private:
		void addStandGrowthCurve(Int64 standGrowthCurveID, StandGrowthCurve& standGrowthCurve);

		// For each stand growth curve, the yield volume is not changed by SPU
		// just create a thread safe lookup map by stand growth curve ID.
		Poco::LRUCache<Int64, StandGrowthCurve>& getCache() {
			thread_local Poco::LRUCache<Int64, StandGrowthCurve> cache;
			return cache;
		}
	};
}}}
#endif
