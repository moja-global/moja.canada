#include "moja/modules/cbm/standmaturitymodule.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void StandMaturityModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &StandMaturityModule::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit, &StandMaturityModule::onTimingInit, *this);
        notificationCenter.subscribe(signals::TimingEndStep, &StandMaturityModule::onTimingEndStep, *this);
    }

    void StandMaturityModule::doLocalDomainInit() {
        _standMaturity = _landUnitData->getVariable("stand_maturity");
        _age = _landUnitData->getVariable("age");
        _gcId = _landUnitData->getVariable("growth_curve_id");
        _spuId = _landUnitData->getVariable("spatial_unit_id");
        _swFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _hwFoliage = _landUnitData->getPool("HardwoodFoliage");
    }

    void StandMaturityModule::doTimingInit() {
        _standMaturity->set_value(0.0);
    }

    void StandMaturityModule::doTimingEndStep() {
        // Get the stand growth curve ID associated to the pixel/svo.
        const auto& gcid = _gcId->value();
        auto standGrowthCurveId = gcid.isEmpty() ? -1 : gcid.convert<Int64>();

        const auto& spuIdVar = _spuId->value();
        auto spuId = spuIdVar.isEmpty() ? -1 : spuIdVar.convert<Int64>();

        const auto& ageVar = _age->value();
        auto age = ageVar.isEmpty() ? -1 : ageVar.convert<Int64>();

        if (standGrowthCurveId == -1 || spuId == -1 || age == -1) {
            _standMaturity->set_value(0.0);
            return;
        }
        
        if (!_volumeToBioGrowth->isBiomassCarbonCurveAvailable(standGrowthCurveId, spuId)) {
            // Call the stand growth curve factory to create the stand growth curve.
            auto standGrowthCurve = _gcFactory->createStandGrowthCurve(
                standGrowthCurveId, spuId, *_landUnitData);

            // Process and convert yield volume to carbon curves.
            _volumeToBioGrowth->generateBiomassCarbonCurve(*standGrowthCurve);
        }

        double maxFoliage = 0.0;
        auto key = std::make_tuple(standGrowthCurveId, spuId);
        const auto& cachedMaxFoliageValue = _maxFoliageValues.find(key);
        if (cachedMaxFoliageValue != _maxFoliageValues.end()) {
            maxFoliage = cachedMaxFoliageValue->second;
        } else {
            auto foliageCurve = _volumeToBioGrowth->getFoliageCarbonCurve(standGrowthCurveId, spuId);
            maxFoliage = foliageCurve.empty() ? 0.0 : *std::max_element(foliageCurve.begin(), foliageCurve.end());
            _maxFoliageValues[key] = maxFoliage;
        }

        double currentFoliageValue = getCurrentFoliageValue();
        double maturity = maxFoliage > 0 ? currentFoliageValue / maxFoliage : 0;

        _standMaturity->set_value(maturity);
    }

    double StandMaturityModule::getCurrentFoliageValue() {
        double swFoliage = _swFoliage->value();
        double hwFoliage = _hwFoliage->value();
        double foliage = swFoliage + hwFoliage;
        
        return foliage;
    }

}}} // namespace moja::modules::cbm

