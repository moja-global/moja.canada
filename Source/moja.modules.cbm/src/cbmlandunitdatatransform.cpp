/**
* @file
*/
#include "moja/modules/cbm/cbmlandunitdatatransform.h"

#include <moja/flint/ivariable.h>

#include <moja/datarepository/datarepository.h>

namespace moja {
namespace modules {
namespace cbm {

    /**
    * Configuration Function.
    *
    * Assign CBMLandUnitDataTransform._landUnitController as parameter landUnitController \n,
    * CBMLandUnitDataTransform._dataRepository as dataRepository, CBMLandUnitDataTransform._provider from _dataRepository, \n
    * CBMLandUnitDataTransform._varName as value of "variable" in parameter config, \n 
    * CBMLandUnitDataTransform._varToUse the value of CBMLandUnitDataTransform._varName in CBMLandUnitDataTransform._landUnitController
    * 
    * @param config DynamicObject
    * @param landUnitController ILandUnitController&
    * @param dataRepository DataRepository&
    * @return void
    * ************************/
    void CBMLandUnitDataTransform::configure(
        DynamicObject config,
        const flint::ILandUnitController& landUnitController,
        datarepository::DataRepository& dataRepository) {

        _landUnitController = &landUnitController;
        _dataRepository = &dataRepository;

        std::string providerName = config["provider"];
        _provider = std::static_pointer_cast<datarepository::IProviderRelationalInterface>(
            _dataRepository->getProvider(providerName));

        std::string varName = config["variable"];
        _varName = varName;
        _varToUse = _landUnitController->getVariable(_varName);
    }

    /**
    * Assign CBMLandUnitDataTransform.__landUnitController, \n
    * CBMLandUnitDataTransform._varToUse the value of CBMLandUnitDataTransform._varName in CBMLandUnitDataTransform._landUnitController
    * 
    * @param controller ILandUnitController
    * @return void
    * ************************/
    void CBMLandUnitDataTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
        _varToUse = _landUnitController->getVariable(_varName);
    };

    /**
     * For each row in CBMLandUnitDataTransform._varToUse, set the values of "spatial_unit_id", "landUnitArea", "age", "growth_curve_id",
     * "admin_boundary", "eco_boundary", "climate_time_series_id" in each row to CBMLandUnitDataTransform._resultsObject \n
     * Assign CBMLandUnitDataTransform._resultsObject to CBMLandUnitDataTransform._results and return
     * 
     * @return DynamicVar&
     */
    const DynamicVar& CBMLandUnitDataTransform::value() const {
        const auto& table = _varToUse->value();
        for (const auto& row : table) {
            _resultsObject["spatial_unit_id"] = row["spatial_unit_id"];
            _resultsObject["landUnitArea"] = row["landUnitArea"];
            _resultsObject["age"] = row["age"];
            _resultsObject["growth_curve_id"] = row["growth_curve_id"];
            _resultsObject["admin_boundary"] = row["admin_boundary"];
            _resultsObject["eco_boundary"] = row["eco_boundary"];
            _resultsObject["climate_time_series_id"] = row["climate_time_series_id"];
        }

        _results = _resultsObject;
        return _results;
    }

}}} // namespace moja::Modules::SLEEK

