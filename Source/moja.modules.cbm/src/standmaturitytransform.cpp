#include "moja/modules/cbm/standmaturitytransform.h"

#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>
#include <moja/logging.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void StandMaturityTransform::configure(
        DynamicObject config,
        const flint::ILandUnitController& landUnitController,
        datarepository::DataRepository& dataRepository) {

        _landUnitController = &landUnitController;
        _dataRepository = &dataRepository;

        _age = _landUnitController->getVariable("age");
        _gcId = _landUnitController->getVariable("growth_curve_id");
        _spuId = _landUnitController->getVariable("spatial_unit_id");
    }

    void StandMaturityTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    const DynamicVar& StandMaturityTransform::value() const {
        // Get the stand growth curve ID associated to the pixel/svo.
        const auto& gcid = _gcId->value();
        auto standGrowthCurveId = gcid.isEmpty() ? -1 : gcid.convert<Int64>();

        int spuId = _spuId->value();
        int age = _age->value();

        double maturity = _volumeToBioGrowth->getMaturityAtAge(standGrowthCurveId, spuId, age);
        _value = maturity;

        return _value;
    }

}}} // namespace moja::modules::cbm

