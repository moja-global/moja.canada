/**
 * @file
 * 
 * ******************/
#include "moja/modules/cbm/transitionruletransform.h"

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
     * Configuration function
     * 
     * Assign TransitionRuleTransform._landUnitController, TransitionRuleTransform._dataRepository as parameters &landUnitController, &dataRepository, 
     * TransitionRuleTransform._provider the result of getProvider() on "provider" in config, TransitionRuleTransform._csetVar, value of "classifier_set_var" of config in TransitionRuleTransform._landUnitController
     * 
     * @param config DynamicObject
     * @param landUnitController const flint::ILandUnitController&
     * @param dataRepository datarepository::DataRepository&
     * @return void
     * *******************/
    void TransitionRuleTransform::configure(
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
     * Assign TransitionRuleTransform._landUnitController as parameter &contoller
     * 
     * @param controller const flint::ILandUnitController&
     * @return void
     * **********************/
    void TransitionRuleTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    /**
     * Create variables classifierNames, a vector of strings and classifierValuesSql, a string \n
     * For each classifier in parameter classifierSet, append classifier.first to classifierNames, concatenate classifierValuesSql with the SQL query TransitionRuleTransform._matchSql, placeholder values
     * clasifier.first and classifier.second (gives the classifier value)
     * Assign a variable classifierNamesSql the join vector classifierNames, separated by "," \n
     * Return the SQL query TransitionRuleTransform._baseSql with placeholders classifierNamesSql and classifierValuesSql
     * 
     * @param classifierSet const DynamicObject&
     * @return string
     * **********************/
    const std::string TransitionRuleTransform::buildSql(const DynamicObject& classifierSet) const {
        std::vector<std::string> classifierNames;
        std::string classifierValuesSql = "";

        for (const auto& classifier : classifierSet) {
            const std::string& classifierName = classifier.first;
            classifierNames.push_back("'" + classifierName + "'");
            const std::string& classifierValue = classifier.second;
            classifierValuesSql += (boost::format(_matchSql)
                % classifierName % classifierValue).str();
        }

        std::string classifierNamesSql = boost::algorithm::join(classifierNames, ",");

        return (boost::format(_baseSql) % classifierNamesSql % classifierValuesSql).str();
    }

    /**
     * If value of TransitionRuleTransform._csetVar is empty assign TransitionRuleTransform._value as DynamicObject() and return TransitionRuleTransform._value \n
     * Extract DynamicObject from the value of TransitionRuleTransform._csetVar and store it in variable cset \n
     * Assign variable sql, the result of TransitionRuleTransform.buildSql() on argument cset \n
     * If sql has a null value in TransitionRuleTransform._cache, assign TransitionRuleTransform._value, a pointer to the result and return TransitionRuleTransform._value \n
     * Create a variable disturbanceTypeTransitions \n
     * For each row in the result of GetDataSet() on TransitionRuleTransform._provider with argument sql, 
     * using row["disturbance_type"] as the key in disturbanceTypeTransitions, map it to row["transition_id"]
     * Add (sql, disturbanceTypeTransitions) to TransitionRuleTransform._cache, assign _value as disturbanceTypeTransitions and return TransitionRuleTransform._value
     * 
     * @return const DynamicVar&
     ***************************/
    const DynamicVar& TransitionRuleTransform::value() const {
        const auto& csetVariableValue = _csetVar->value();
        if (csetVariableValue.isEmpty()) {
            _value = DynamicObject();
            return _value;
        }

        const auto& cset = csetVariableValue.extract<DynamicObject>();
        const auto sql = buildSql(cset);

        auto cachedValue = _cache->get(sql);
        if (!cachedValue.isNull()) {
            _value = *cachedValue;
            return _value;
        }

        DynamicObject disturbanceTypeTransitions;
        const auto& result = _provider->GetDataSet(sql);
        if (result.isVector()) {
            for (auto row : result.extract<std::vector<DynamicObject>>()) {
                std::string disturbanceType = row["disturbance_type"];
                int transitionId = row["transition_id"];
                disturbanceTypeTransitions[disturbanceType] = transitionId;
            }
        } else {
            std::string disturbanceType = result["disturbance_type"];
            int transitionId = result["transition_id"];
            disturbanceTypeTransitions[disturbanceType] = transitionId;
        }

        _cache->add(sql, disturbanceTypeTransitions);
        _value = disturbanceTypeTransitions;

        return _value;
    }

}}} // namespace moja::modules::cbm

