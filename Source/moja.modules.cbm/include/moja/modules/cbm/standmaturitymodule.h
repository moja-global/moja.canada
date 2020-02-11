#ifndef MOJA_MODULES_CBM_STANDMATURITYMODULE_H_
#define MOJA_MODULES_CBM_STANDMATURITYMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
namespace modules {
namespace cbm {

class StandMaturityModule : public CBMModuleBase {
public:
    StandMaturityModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory, std::shared_ptr<VolumeToBiomassCarbonGrowth> volumeToBioGrowth)
        : _gcFactory(gcFactory), _volumeToBioGrowth(volumeToBioGrowth) {};

    void subscribe(NotificationCenter& notificationCenter) override;

    flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

    void doLocalDomainInit() override;
    void doTimingInit() override;
    void doTimingEndStep() override;

private:
    double getCurrentFoliageValue();

	const flint::ILandUnitController* _landUnitController;

    const flint::IVariable* _age;
    const flint::IVariable* _gcId;
    const flint::IVariable* _spuId;
    
    flint::IVariable* _standMaturity;

    const flint::IPool* _swFoliage;
    const flint::IPool* _hwFoliage;

    mutable DynamicVar _value;
    mutable std::unordered_map<std::tuple<int, int>, double> _maxFoliageValues;

    std::shared_ptr<StandGrowthCurveFactory> _gcFactory;
    std::shared_ptr<VolumeToBiomassCarbonGrowth> _volumeToBioGrowth;
};

}}}

#endif // MOJA_MODULES_CBM_STANDMATURITYMODULE_H_