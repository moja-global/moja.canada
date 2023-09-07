#include "moja/modules/cbm/growthcurvetransform.h"

#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>
#include <moja/logging.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>


using moja::datarepository::IProviderRelationalInterface;

namespace moja {
namespace modules {
namespace cbm {

    /**
     * Assign GrowthCurveTransform._landUnitController, GrowthCurveTransform._dataRepository as parameters &landUnitController, &dataRepository \n
     * GrowthCurveTransform._provider, GrowthCurveTransform._csetVar value of "provider", "classifier_set_var" in parameter config
     * 
     * @param config DynamicObject
     * @param landUnitController const flint::ILandUnitController&
     * @param dataRepository datarepository::DataRepository&
     * @return void 
     ***********************************/
    void GrowthCurveTransform::configure(
        DynamicObject config,
        const flint::ILandUnitController& landUnitController,
        datarepository::DataRepository& dataRepository) {

        _landUnitController = &landUnitController;
        _dataRepository = &dataRepository;

        std::string providerName = config["provider"];
        _provider = std::static_pointer_cast<IProviderRelationalInterface>(
                _dataRepository->getProvider(providerName));

        auto csetVarName = config["classifier_set_var"].convert<std::string>();
        _csetVar = _landUnitController->getVariable(csetVarName);
    }

    /**
     * Assign GrowthCurveTransform._landUnitController as parameter &controller
     * 
     * @param controller const flint::ILandUnitController&
     * @return void
     * **********************/
    void GrowthCurveTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    /**
     * Replace the placeholder values in SQL queries GrowthCurveTransform._baseSql, GrowthCurveTransform._matchSql
     * 
     * Create variables classifierNames, a vector of strings and classifierValuesSql, a string \n
     * For each classifier in parameter classifierSet, append classifier.first to classifierNames, concatenate classifierValuesSql with the SQL query GrowthCurveTransform._matchSql, placeholder values
     * clasifier.first and classifier.second (gives the classifier value)
     * Assign a variable classifierNamesSql the join vector classifierNames, separated by "," \n
     * Return the SQL query GrowthCurveTransform._baseSql with placeholders classifierNamesSql and classifierValuesSql
     * 
     * @param classifierSet const DynamicObject&
     * @return string
     * **********************/
    const std::string GrowthCurveTransform::buildSql(const DynamicObject& classifierSet) const {
        std::vector<std::string> classifierNames;
        std::string classifierValuesSql = "";

        for (const auto& classifier : classifierSet) {
            const std::string& classifierName = classifier.first;
            classifierNames.push_back("'" + classifierName + "'");

            if (!classifier.second.isEmpty()) {
                const std::string& classifierValue = classifier.second;
                classifierValuesSql += (boost::format(_matchSql)
                    % classifierName % classifierValue).str();
            } else {
                classifierValuesSql += (boost::format(_matchSql)
                    % classifierName % "NULL").str();
            }
        }

        std::string classifierNamesSql = boost::algorithm::join(classifierNames, ",");

        return (boost::format(_baseSql) % classifierNamesSql % classifierValuesSql).str();
    }

    /**
     * Return all the classifier names and values in parameter classifierSet, in the format classifierName : classifierValue, separated by a "," 
     * 
     * @param classifierSet const DynamicObject&
     * @return string
     * ************************/
    const std::string GrowthCurveTransform::buildDebuggingInfo(const DynamicObject& classifierSet) const {
        std::vector<std::string> debugInfo;
        for (const auto& classifier : classifierSet) {
            const std::string& classifierName = classifier.first;

            if (!classifier.second.isEmpty()) {
                const std::string& classifierValue = classifier.second;
                debugInfo.push_back(classifierName + ": " + classifierValue);
            } else {
                debugInfo.push_back(classifierName + ": NULL");
            }
        }

        return boost::algorithm::join(debugInfo, ", ");
    }

    /**
     * If value of GrowthCurveTransform._csetVar is empty assign GrowthCurveTransform._value as DynamicObject() and return GrowthCurveTransform._value \n
     * Extract DynamicObject from the value of GrowthCurveTransform._csetVar and store it in variable cset \n
     * Assign variable sql, the result of GrowthCurveTransform.buildSql() on argument cset \n
     * If sql has a null value in GrowthCurveTransform._cache, return a pointer to the result \n
     * If the result of GetDataSet() on GrowthCurveTransform._provider with argument sql has a size > 0, 
     * add sql, and "growth_curve_id" at index 0 of the result to GrowthCurveTransform._cache, \n
     * else add sql, and a variable of type DynamicVar to GrowthCurveTransform._cache, indicate an error and invoke GrowthCurveTransform.buildDebuggingInfo() with argument cset \n
     * Return a pointer to the value of sql in GrowthCurveTransform._cache
     * 
     * @return const DynamicVar&
     ***************************/
    const DynamicVar& GrowthCurveTransform::value() const {
        const auto& csetVariableValue = _csetVar->value();
        if (csetVariableValue.isEmpty()) {
            _value = DynamicVar();
            return _value;
        }

        const auto& cset = csetVariableValue.extract<DynamicObject>();
        
        GCCacheKey key;
        for (const auto& classifier : cset) {
            if (!classifier.second.isEmpty()) {
                const std::string& classifierValue = classifier.second;
                key.add(classifierValue);
            }
        }

        auto cachedValue = _cache->get(key);
        if (!cachedValue.isNull()) {
            return *cachedValue;
        }

        DynamicVar value;
        const auto sql = buildSql(cset);
        const auto& gc = _provider->GetDataSet(sql);
        if (gc.size() > 0) {
            auto& gcRows = gc.extract<const std::vector<DynamicObject>>();
            int gcId = gcRows.at(0)["growth_curve_id"];
            value = gcId;
        } else {
            MOJA_LOG_DEBUG << "Error getting growth curve for classifier set: "
                           << buildDebuggingInfo(cset);
        }

        _cache->add(key, value);

        return *_cache->get(key);
    }

}}} // namespace moja::modules::cbm

