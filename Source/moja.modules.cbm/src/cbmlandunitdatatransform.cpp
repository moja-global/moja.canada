#include "moja/modules/cbm/cbmlandunitdatatransform.h"

#include <moja/flint/ivariable.h>

#include <moja/datarepository/datarepository.h>

namespace moja {
namespace modules {
namespace cbm {

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

    void CBMLandUnitDataTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
        _varToUse = _landUnitController->getVariable(_varName);
    };

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

