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

    void TransitionRuleTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

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

