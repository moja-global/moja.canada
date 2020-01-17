#ifndef MOJA_MODULES_CBM_STANDMATURITYTRANSFORM_H_
#define MOJA_MODULES_CBM_STANDMATURITYTRANSFORM_H_

#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"
#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

#include <Poco/LRUCache.h>
#include <Poco/ThreadLocal.h>

namespace moja {
namespace modules {
namespace cbm {

class StandMaturityTransform : public flint::ITransform {
public:
    StandMaturityTransform(std::shared_ptr<VolumeToBiomassCarbonGrowth> volumeToBioGrowth) :
        _volumeToBioGrowth(volumeToBioGrowth) {}

	void configure(DynamicObject config,
		const flint::ILandUnitController& landUnitController,
		datarepository::DataRepository& dataRepository) override;

	void controllerChanged(const flint::ILandUnitController& controller) override;
	const DynamicVar& value() const override;

private:
	const flint::ILandUnitController* _landUnitController;
	datarepository::DataRepository* _dataRepository;

    const flint::IVariable* _age;
    const flint::IVariable* _gcId;
    const flint::IVariable* _spuId;

    mutable DynamicVar _value;

    std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
};

}}}

#endif // MOJA_MODULES_CBM_STANDMATURITYTRANSFORM_H_