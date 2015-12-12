#include "moja/modules/cbm/growthcurvetransform.h"
#include "moja/logging.h"
#include "moja/datarepository/iproviderinterface.h"

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

#include <limits>

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

        _csetVarName = config["classifier_set_var"].convert<std::string>();
    }

    void GrowthCurveTransform::controllerChanged(const flint::ILandUnitController& controller) {
        _landUnitController = &controller;
    };

    const Dynamic& GrowthCurveTransform::value() const {
        _csetVar = _landUnitController->getVariable(_csetVarName);
        const auto& csetVariableValue = _csetVar->value();
        if (csetVariableValue.isEmpty()) {
            _value = -1;
            return _value;
        }

        std::vector<std::string> classifierNames;
        std::string classifierValuesSql = "";

       const auto& cset = csetVariableValue.extract<std::vector<DynamicObject>>();
        for (const auto& classifier : cset) {
            std::string classifierName = classifier["classifier_name"].convert<std::string>();
            classifierNames.push_back("'" + classifierName + "'");

            if (classifier["classifier_value"].isInteger()) {
                auto classifierValue = classifier["classifier_value"].convert<int>();
                classifierValuesSql += (boost::format(matchSql)
                    % classifierName % classifierValue).str();
            }
            else {
                auto classifierValue = classifier["classifier_value"].convert<std::string>();
                classifierValuesSql += (boost::format(matchSql)
                    % classifierName % classifierValue).str();
            }
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

