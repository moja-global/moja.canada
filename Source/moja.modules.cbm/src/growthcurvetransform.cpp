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

    const DynamicVar& GrowthCurveTransform::value() const {
        const auto& csetVariableValue = _csetVar->value();
        if (csetVariableValue.isEmpty()) {
            _value = -1;
            return _value;
        }

        std::vector<std::string> classifierNames;
        std::string classifierValuesSql = "";

        const auto& cset = csetVariableValue.extract<DynamicObject>();
        for (const auto& classifier : cset) {
            const std::string& classifierName = classifier.first;
            classifierNames.push_back("'" + classifierName + "'");
            const std::string& classifierValue = classifier.second;
            classifierValuesSql += (boost::format(matchSql)
                % classifierName % classifierValue).str();
        }

        std::string classifierNamesSql = boost::algorithm::join(classifierNames, ",");
        std::string sql = (boost::format(baseSql)
            % classifierNamesSql % classifierValuesSql).str();

        const auto& gc = _provider->GetDataSet(sql);
        if (gc.size() > 0) {
            auto& gcRows = gc.extract<const std::vector<DynamicObject>>();
            _value = gcRows.at(0)["growth_curve_id"];
        } else {
            MOJA_LOG_DEBUG << "Error getting growth curve for query: " << sql;
            _value = -1;
        }

        return _value;
    }

}}} // namespace moja::modules::cbm
