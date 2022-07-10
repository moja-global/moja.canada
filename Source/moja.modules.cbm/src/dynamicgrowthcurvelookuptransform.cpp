
/**
 * @file 
 * 
 * ************************/
#include "moja/modules/cbm/dynamicgrowthcurvelookuptransform.h"

#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>

using moja::datarepository::IProviderRelationalInterface;

namespace moja {
namespace modules {
namespace cbm {

    /**
     * Configuration function
     * 
     * Assign DynamicGrowthCurveLookupTransform._landUnitController as parameter &landUnitController, \n
     * DynamicGrowthCurveLookupTransform._forestTypeFilter as variable "forest_type" in parameter config, \n
     * DynamicGrowthCurveLookupTransform._gcIdVar as variable "growth_curve_id" in DynamicGrowthCurveLookupTransform._landUnitController
     * 
     * @param DynamicObject config
     * @param flint::ILandUnitController& landUnitController
     * @param  datarepository::DataRepository& dataRepository
     * @return void
     * *********************/
    void DynamicGrowthCurveLookupTransform::configure(
        DynamicObject config,
        const flint::ILandUnitController& landUnitController,
        datarepository::DataRepository& dataRepository) {

        _landUnitController = &landUnitController;
        _forestTypeFilter = config["forest_type"].convert<std::string>();
        _gcIdVar = _landUnitController->getVariable("growth_curve_id");
    }

    /**
     * Set DynamicGrowthCurveLookupTransform._landUnitController as &controller
     * 
     * @param flint::ILandUnitController& controller
     * @return void
     * *************************/
    void DynamicGrowthCurveLookupTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    /**
     * If DynamicGrowthCurveLookupTransform._gcIdVar is empty, \n
     * DynamicGrowthCurveLookupTransform._gcCache does not contain DynamicGrowthCurveLookupTransform._gcIdVar \n 
     * or DynamicGrowthCurveLookupTransform._gcCache does not contain DynamicGrowthCurveLookupTransform._forestTypeFilter, \n
     * assign DynamicGrowthCurveLookupTransform._value to DynamicVar() and return \n
     * Else return the value of DynamicGrowthCurveLookupTransform._forestTypeFilter in DynamicGrowthCurveLookupTransform._gcCache 
     * 
     * @return void
     * *************/
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

