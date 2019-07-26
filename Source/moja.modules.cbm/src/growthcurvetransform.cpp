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

    void GrowthCurveTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    const std::string GrowthCurveTransform::buildSql(const DynamicObject& classifierSet) const {
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

    const std::string GrowthCurveTransform::buildDebuggingInfo(const DynamicObject& classifierSet) const {
        std::vector<std::string> debugInfo;
        for (const auto& classifier : classifierSet) {
            const std::string& classifierName = classifier.first;
            const std::string& classifierValue = classifier.second;
            debugInfo.push_back(classifierName + ": " + classifierValue);
        }

        return boost::algorithm::join(debugInfo, ", ");
    }

    const DynamicVar& GrowthCurveTransform::value() const {
        const auto& csetVariableValue = _csetVar->value();
        if (csetVariableValue.isEmpty()) {
            _value = DynamicVar();
            return _value;
        }

        const auto& cset = csetVariableValue.extract<DynamicObject>();
        const auto sql = buildSql(cset);

        auto cachedValue = _cache->get(sql);
        if (!cachedValue.isNull()) {
            return *cachedValue;
        }

        DynamicVar value;
        const auto& gc = _provider->GetDataSet(sql);
        if (gc.size() > 0) {
            auto& gcRows = gc.extract<const std::vector<DynamicObject>>();
            int gcId = gcRows.at(0)["growth_curve_id"];
            value = gcId;
        } else {
            MOJA_LOG_DEBUG << "Error getting growth curve for classifier set: "
                           << buildDebuggingInfo(cset);
        }

        _cache->add(sql, value);

        return *_cache->get(sql);
    }

}}} // namespace moja::modules::cbm

