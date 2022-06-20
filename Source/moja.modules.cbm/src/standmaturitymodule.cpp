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

    /**
     * Subscribe to the signals LocalDomainInit, TimingInit and TimingEndStep
     * 
     * @param notificationCenter NotificationCenter&
     * @return void
     *********************/
    void StandMaturityModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &StandMaturityModule::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit, &StandMaturityModule::onTimingInit, *this);
        notificationCenter.subscribe(signals::TimingEndStep, &StandMaturityModule::onTimingEndStep, *this);
    }

    /**
     * Assign StandMaturityModule._standMaturity, StandMaturityModule._age, StandMaturityModule._gcId, StandMaturityModule._spuId, 
     * StandMaturityModule._swFoilage, StandMaturityModule._hwFoilage values of variables "stand_maturity", "age", "growth_curve_id", "spatial_unit_id", "SoftwoodFoliage", "HardwoodFoliage"
     * in _landUnitData
     * 
     * @return void
     *******************/
    void StandMaturityModule::doLocalDomainInit() {
        _standMaturity = _landUnitData->getVariable("stand_maturity");
        _age = _landUnitData->getVariable("age");
        _gcId = _landUnitData->getVariable("growth_curve_id");
        _spuId = _landUnitData->getVariable("spatial_unit_id");
        _swFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _hwFoliage = _landUnitData->getPool("HardwoodFoliage");
    }

    /**
     * Set the value of StandMaturityModule._standMaturity to 0.0
     * 
     * @return void
     * *******************/
    void StandMaturityModule::doTimingInit() {
        _standMaturity->set_value(0.0);
    }

    /**
     * If the value of StandMaturityModule._gcId or StandMaturityModule._spuId or StandMaturityModule._age is empty, set the value of StandMaturityModule._standMaturity to 0.0 and return, else assign the values to variables standGrowthCurveId, spuId, ageVar \n
     * If isBiomassCarbonCurveAvailable() with arguments standGrowthCurveId, spuId on StandMaturityModule._volumeToBioGrowth is false, call createStandGrowthCurve() with arguments standGrowthCurveId, spuId, *_landUnitData on  StandMaturityModule._gcFactory to create the stand growth curve, 
     * invoke generateBiomassCarbonCurve() on StandMaturityModule._volumeToBioGrowth to process and convert yield volume to carbon curves \n
     * Assign variable maxFoilage, the value corresponding to the tuple key (standGrowthCurveId, spuId) in StandMaturityModule._maxFoliageValues, if found. \n
     * Else, get the foilageCurve from getFoliageCarbonCurve() on StandMaturityModule._volumeToBioGrowth with arguments standGrowthCurveId, spuId. Assign maxFoilage the maximum of the iterator maxFoilage, if not empty, else 0
     * Set the value of the key (standGrowthCurveId, spuId) in StandMaturityModule._maxFoliageValues to maxFoilage \n
     * Set the value of StandMaturityModule._standMaturity to 0 if maxFoilage is 0, else to StandMaturityModule.getCurrentFoliageValue() / maxFoilage
     * 
     * @return void
     ***************************/
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

    /**
     * Return the sum of the values of StandMaturityModule._swFoliage and StandMaturityModule._hwFoliage
     * 
     * @return double
     * *******************/
    double StandMaturityModule::getCurrentFoliageValue() {
        double swFoliage = _swFoliage->value();
        double hwFoliage = _hwFoliage->value();
        double foliage = swFoliage + hwFoliage;
        
        return foliage;
    }

}}} // namespace moja::modules::cbm

