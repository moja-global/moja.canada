#include "moja/modules/cbm/dynamicgrowthcurvelookuptransform.h"

#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>

using moja::datarepository::IProviderRelationalInterface;

namespace moja {
namespace modules {
namespace cbm {

    void DynamicGrowthCurveLookupTransform::configure(
        DynamicObject config,
        const flint::ILandUnitController& landUnitController,
        datarepository::DataRepository& dataRepository) {

        _landUnitController = &landUnitController;
        _forestTypeFilter = config["forest_type"].convert<std::string>();
        _gcIdVar = _landUnitController->getVariable("growth_curve_id");
    }

    void DynamicGrowthCurveLookupTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    const DynamicVar& DynamicGrowthCurveLookupTransform::value() const {
        const auto& gcIdValue = _gcIdVar->value();
        if (gcIdValue.isEmpty()) {
            _value = DynamicVar();
            return _value;
        }
        
        int gcId = gcIdValue;
        if (_gcCache->find(gcId) == _gcCache->end()) {
            _value = DynamicVar();
            return _value;
        }

        if (_gcCache->operator[](gcId).find(_forestTypeFilter) == _gcCache->operator[](gcId).end()) {
            _value = DynamicVar();
            return _value;
        }

        return _gcCache->operator[](gcId)[_forestTypeFilter];
    }

}}} // namespace moja::modules::cbm

