#include "moja/modules/cbm/dynamicgrowthcurvetransform.h"

#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>
#include <moja/logging.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

#include <math.h>

using moja::datarepository::IProviderRelationalInterface;

namespace moja {
namespace modules {
namespace cbm {

    void DynamicGrowthCurveTransform::configure(
        DynamicObject config,
        const flint::ILandUnitController& landUnitController,
        datarepository::DataRepository& dataRepository) {

        _landUnitController = &landUnitController;
        _csetVar = _landUnitController->getVariable("classifier_set");
        _precipitationVar = _landUnitController->getVariable("mean_annual_precipitation");
        _growingDaysVar = _landUnitController->getVariable("mean_annual_growing_days");
        _volToBioVar = _landUnitController->getVariable("volume_to_biomass_parameters");
        _growthAndYieldParamsVar = _landUnitController->getVariable("growth_and_yield_parameters");

        if (config.contains("max_age")) {
            _maxAge = config["max_age"];
        }

        if (config.contains("increment_length")) {
            _incrementLength = config["increment_length"];
        }

        if (config.contains("debugging_enabled")) {
            _debug = config["debugging_enabled"];
        }
    }

    void DynamicGrowthCurveTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    const DynamicVar& DynamicGrowthCurveTransform::value() const {
        const auto& csetVariableValue = _csetVar->value();
        if (csetVariableValue.isEmpty()) {
            _value = DynamicVar();
            return _value;
        }

        const auto& precipitationVariableValue = _precipitationVar->value();
        if (precipitationVariableValue.isEmpty()) {
            _value = DynamicVar();
            return _value;
        }

        const auto& growingDaysVariableValue = _growingDaysVar->value();
        if (growingDaysVariableValue.isEmpty()) {
            _value = DynamicVar();
            return _value;
        }

        const auto& cset = csetVariableValue.extract<DynamicObject>();
        double precipitation = precipitationVariableValue;
        double growingDays = growingDaysVariableValue;
        std::string species = cset["Species"].convert<std::string>();

        auto key = std::make_tuple(species, precipitation, growingDays);
        auto cachedValue = _gcIdCache->find(key);
        if (cachedValue != _gcIdCache->end()) {
            return cachedValue->second;
        }

        // Lock for generating a new GC and check again in case of a race.
        Poco::Mutex::ScopedLock lock(*_cacheLock);
        cachedValue = _gcIdCache->find(key);
        if (cachedValue != _gcIdCache->end()) {
            return cachedValue->second;
        }

        std::vector<double> b;
        const auto& growthAndYieldParams = _growthAndYieldParamsVar->value();
        if (growthAndYieldParams.isEmpty()) {
            _gcIdCache->try_emplace(key, -1);
            _value = DynamicVar();
            return _value;
        }

        DynamicVar gcId = (*_nextGcId)++;
        _gcIdCache->try_emplace(key, gcId);

        const auto& growthAndYieldParamsValue = growthAndYieldParams.extract<DynamicObject>();
        for (const auto& bParam : { "b0", "b1", "b2", "b3", "b4", "b5" }) {
            b.push_back(growthAndYieldParamsValue[bParam]);
        }

        double correctionFactor = growthAndYieldParamsValue["c"];

        const auto& vol2bio = _volToBioVar->value().extract<DynamicObject>();
        std::string forestType = vol2bio["forest_type"].convert<std::string>();
        std::vector<DynamicObject> growthCurve;
        for (int i = 0; i <= _maxAge / _incrementLength; i++) {
            int age = i * _incrementLength;
            double vol = std::exp(
                    b[0]
                + b[1] * growingDays
                + b[2] * precipitation
                + (b[3] + b[4] * growingDays + b[5] * precipitation) / age
            ) * correctionFactor;

            DynamicObject ageVol;
            ageVol["age"] = age;
            ageVol["merchantable_volume"] = vol;
            
            growthCurve.push_back(ageVol);
        }

        (*_gcCache)[gcId][forestType] = growthCurve;

        if (_debug) {
            std::string debugLine = ((boost::format("GC:%1%,%2%,%3%") % species % growingDays % precipitation)).str();
            auto gc = _gcCache->at(gcId);
            const auto& forestTypeCurve = gc[forestType].extract<std::vector<DynamicObject>>();
            for (const auto& ageVol : forestTypeCurve) {
                debugLine += (boost::format(",%1%") % ageVol["merchantable_volume"].extract<double>()).str();
            }

            MOJA_LOG_DEBUG << debugLine;
        }

        return _gcIdCache->at(key);
    }

}}} // namespace moja::modules::cbm

