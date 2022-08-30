
/**
 * @file
 * Generates a merchantable volume curve dynamically from Ung et al (2009) equation 8,
 * using mean annual precipitation, growing degree days, and species-specific coefficients.
 *******************/
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

    /**
     * Configuration function
     * 
     * Assign DynamicGrowthCurveTransform._landUnitController as parameter &landUnitController, \n
     * Assign values of variables "classifier_set", "mean_annual_precipitation", "mean_annual_growing_days", "volume_to_biomass_parameters", "growth_and_yield_parameters" in DynamicGrowthCurveTransform._landUnitController \n
     * to DynamicGrowthCurveTransform._csetVar, DynamicGrowthCurveTransform._precipitationVar, DynamicGrowthCurveTransform._growingDaysVar, DynamicGrowthCurveTransform._volToBioVar, DynamicGrowthCurveTransform._growthAndYieldParamsVar \n
     * If parameter config contains "age", "increment_length", "debugging_enabled", assign it to DynamicGrowthCurveTransform._maxAge, DynamicGrowthCurveTransform._incrementLength, DynamicGrowthCurveTransform._debug 
     * 
     * @param config DynamicObject 
     * @param landUnitController flint::ILandUnitController&
     * @param dataRepository datarepository::DataRepository&
     * @return void
     * **************************/
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

    /**
     * Assign DynamicGrowthCurveTransform._landUnitController as paramter &controller
     * 
     * @param controller flint::ILandUnitController&flint::ILandUnitController&
     * @return void
     * ****************/
    void DynamicGrowthCurveTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    /**
     * If the value of DynamicGrowthCurveTransform._csetVar or DynamicGrowthCurveTransform._precipitationVar or DynamicGrowthCurveTransform._growingDaysVar is empty, assign DynamicGrowthCurveTransform._value as 
     * an object of Poco::Dynamic::Var, and return DynamicGrowthCurveTransform._value \n
     * Assign a variable key, to the tuple, species (value of "Species" in DynamicGrowthCurveTransform._csetVar),  precipitation (value of DynamicGrowthCurveTransform._precipitationVar) and growingDays (value of DynamicGrowthCurveTransform._growingDaysVar) \n
     * If key exists in DynamicGrowthCurveTransform._gcIdCache return it \n
     * Apply Poco::Mutex::ScopedLock, generate a new Growth Curvce and check again in case of a race condition \n
     * If value of _growthAndYieldParamsVar is empty, add variable key, -1 to DynamicGrowthCurveTransform._gcIdCache, assign DynamicGrowthCurveTransform._value as 
     * an object of Poco::Dynamic::Var, and return DynamicGrowthCurveTransform._value \n
     * 
     * Return value of variable key in DynamicGrowthCurveTransform._gcIdCache
     * 
     * @return DynamicVar& 
     * *********************/
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

