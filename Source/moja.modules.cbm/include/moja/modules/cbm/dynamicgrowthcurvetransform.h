#ifndef MOJA_MODULES_CBM_DYNAMICGROWTHCURVETRANSFORM_H_
#define MOJA_MODULES_CBM_DYNAMICGROWTHCURVETRANSFORM_H_

#include "moja/datarepository/iproviderrelationalinterface.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"

#include <Poco/LRUCache.h>
#include <Poco/ThreadLocal.h>

#include <atomic>
#include <map>

namespace moja {
namespace modules {
namespace cbm {

/**
 * Generates a merchantable volume curve dynamically from Ung et al (2009) equation 8,
 * using mean annual precipitation, growing degree days, and species-specific coefficients.
 */
class DynamicGrowthCurveTransform : public flint::ITransform {
public:
    DynamicGrowthCurveTransform(
        std::shared_ptr<std::map<std::tuple<std::string, double, double>, DynamicVar>> gcIdCache,
        std::shared_ptr<std::map<int, std::map<std::string, DynamicVar>>> gcCache,
        std::shared_ptr<Poco::Mutex> cacheLock,
        std::shared_ptr<std::atomic<int>> nextGcId
    ) : _gcIdCache(gcIdCache), _gcCache(gcCache), _cacheLock(cacheLock), _nextGcId(nextGcId) { }

	void configure(DynamicObject config,
		const flint::ILandUnitController& landUnitController,
		datarepository::DataRepository& dataRepository) override;

	void controllerChanged(const flint::ILandUnitController& controller) override;
	const DynamicVar& value() const override;

private:
    mutable int _maxAge = 300;
    mutable int _incrementLength = 10;
    mutable bool _debug = false;

	const flint::ILandUnitController* _landUnitController;
	mutable const flint::IVariable* _csetVar;
    mutable const flint::IVariable* _precipitationVar;
    mutable const flint::IVariable* _growingDaysVar;
    mutable const flint::IVariable* _volToBioVar;
    mutable const flint::IVariable* _growthAndYieldParamsVar;
    mutable DynamicVar _value;

    // Key: <species, mean annual precipitation, growing days>
    std::shared_ptr<std::map<std::tuple<std::string, double, double>, DynamicVar>> _gcIdCache;
    std::shared_ptr<std::map<int, std::map<std::string, DynamicVar>>> _gcCache;
    std::shared_ptr<Poco::Mutex> _cacheLock;
    std::shared_ptr<std::atomic<int>> _nextGcId;
};

}}}

#endif // MOJA_MODULES_CBM_DYNAMICGROWTHCURVETRANSFORM_H_