#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <pqxx/strconv>

#include "moja/modules/cbm/flatrecord.h"
#include "moja/hash.h"

namespace moja {
namespace modules {
namespace cbm {
    
    const std::string FlatRecordHelper::BuildClassifierNamesString(const std::vector<std::string>& classifierNames, const std::string& suffix) {
        std::string classifierStr = "";
        bool firstItem = true;
        for (const auto& name : classifierNames) {
            std::string valueStr = firstItem ? "" : ",";
            firstItem = false;
            valueStr += (boost::format("\"%1%%2%\"") % name % suffix).str();
            classifierStr += valueStr;
        }

        return classifierStr;
    }

    const std::string FlatRecordHelper::BuildClassifierValueString(const std::vector<Poco::Nullable<std::string>>& classifierValues) {
        std::string classifierStr = "";
        bool firstItem = true;
        for (const auto& value : classifierValues) {
            std::string valueStr = firstItem ? "" : ",";
            firstItem = false;
            if (!value.isNull()) {
                valueStr += (boost::format("\"%1%\"") % value).str();
            }

            classifierStr += valueStr;
        }

        return classifierStr;
    }

    // -- FlatFluxRecord
    FlatFluxRecord::FlatFluxRecord(
        int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
        const std::string& ageClass, const std::vector<Poco::Nullable<std::string>>& previousClassifierValues,
        const std::string& previousLandClass, const std::string& previousAgeClass, const Poco::Nullable<std::string>& disturbanceType,
        const Poco::Nullable<int>& disturbanceCode, const std::string& srcPool, const std::string& dstPool, double flux
    ) : _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass),
        _previousClassifierValues(previousClassifierValues), _previousLandClass(previousLandClass),
        _previousAgeClass(previousAgeClass), _disturbanceType(disturbanceType), _disturbanceCode(disturbanceCode),
        _srcPool(srcPool), _dstPool(dstPool), _flux(flux) { }

    bool FlatFluxRecord::operator==(const FlatFluxRecord& other) const {
        bool isEqual = _year == other._year
			&& _landClass == other._landClass
            && _ageClass == other._ageClass
			&& _previousLandClass == other._previousLandClass
			&& _previousAgeClass == other._previousAgeClass
            && _disturbanceType.value("") == other._disturbanceType.value("")
            && _disturbanceCode.value(-1) == other._disturbanceCode.value(-1)
            && _srcPool == other._srcPool
            && _dstPool == other._dstPool;

        if (!isEqual) {
            return false;
        }

        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i].value("") != other._classifierValues[i].value("")) {
                return false;
            }
        }

        for (int i = 0; i < other._previousClassifierValues.size(); i++) {
            if (_previousClassifierValues[i].value("") != other._previousClassifierValues[i].value("")) {
                return false;
            }
        }

        return true;
    }

    size_t FlatFluxRecord::hash() const {
        if (_hash == -1) {
            size_t hash = 0;
            hash = moja::hash::hash_combine(hash, _disturbanceType.value(""), _disturbanceCode.value());
            for (const auto& classifier : _classifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            for (const auto& classifier : _previousClassifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            _hash = moja::hash::hash_combine(
                hash, _year, _landClass, _ageClass, _previousLandClass, _previousAgeClass, _srcPool, _dstPool);
        }

        return _hash;
    }

    std::string FlatFluxRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames, "_previous");

        return (boost::format("year,%1%,unfccc_land_class,age_range,%2%,unfccc_land_class_previous,age_range_previous,disturbance_type,disturbance_code,from_pool,to_pool,flux_tc\n")
            % classifierStr % previousClassifierStr).str();
    }

    std::string FlatFluxRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierValueString(_previousClassifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%,%6%,%7%,\"%8%\",%9%,%10%,%11%,%12%\n")
            % _year % classifierStr % _landClass % _ageClass % previousClassifierStr % _previousLandClass
            % _previousAgeClass % _disturbanceType % _disturbanceCode % _srcPool % _dstPool % _flux).str();
    }

    std::vector<std::optional<std::string>> FlatFluxRecord::asVector() const {
        std::vector<std::optional<std::string>> row;
        row.push_back(pqxx::to_string(_year));
        for (const auto& value : _classifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_landClass);
        row.push_back(_ageClass);
        for (const auto& value : _previousClassifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_previousLandClass);
        row.push_back(_previousAgeClass);
        row.push_back(_disturbanceType.isNull() ? std::optional<std::string>(std::nullopt) : pqxx::to_string(_disturbanceType.value()));
        row.push_back(_disturbanceCode.isNull() ? std::optional<std::string>(std::nullopt) : pqxx::to_string(_disturbanceCode.value()));
        row.push_back(_srcPool);
        row.push_back(_dstPool);
        row.push_back(pqxx::to_string(_flux));

        return row;
    }

    void FlatFluxRecord::merge(const FlatFluxRecord& other) {
        _flux += other._flux;
    }
    // --

	// -- FlatPoolRecord
    FlatPoolRecord::FlatPoolRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues,
                                   const std::string& landClass, const std::string& ageClass, const std::string& pool, double value)
        : _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass),
          _pool(pool), _value(value) { }

    bool FlatPoolRecord::operator==(const FlatPoolRecord& other) const {
        bool isEqual = _year == other._year
            && _landClass == other._landClass
            && _ageClass == other._ageClass
            && _pool == other._pool;

        if (!isEqual) {
            return false;
        }

        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i].value("") != other._classifierValues[i].value("")) {
                return false;
            }
        }

        return true;
    }

    size_t FlatPoolRecord::hash() const {
        if (_hash == -1) {
            size_t hash = 0;
            for (const auto& classifier : _classifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            _hash = moja::hash::hash_combine(hash, _year, _landClass, _ageClass, _pool);
        }

        return _hash;
    }

    std::string FlatPoolRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,unfccc_land_class,age_range,pool,pool_tc\n") % classifierStr).str();
    }

    std::string FlatPoolRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%,%6%\n")
            % _year % classifierStr % _landClass % _ageClass % _pool % _value).str();
    }

    std::vector<std::optional<std::string>> FlatPoolRecord::asVector() const {
        std::vector<std::optional<std::string>> row;
        row.push_back(pqxx::to_string(_year));
        for (const auto& value : _classifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_landClass);
        row.push_back(_ageClass);
        row.push_back(_pool);
        row.push_back(pqxx::to_string(_value));

        return row;
    }

    void FlatPoolRecord::merge(const FlatPoolRecord& other) {
        _value += other._value;
    }
    // --

	// -- FlatErrorRecord
    FlatErrorRecord::FlatErrorRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues,
                                     const std::string& module, const std::string& error, double area)
		: _year(year), _classifierValues(classifierValues), _module(module), _error(error), _area(area) { }

	bool FlatErrorRecord::operator==(const FlatErrorRecord& other) const {
        bool isEqual = _year == other._year
            && _module == other._module
            && _error == other._error;

        if (!isEqual) {
            return false;
        }

        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i].value("") != other._classifierValues[i].value("")) {
                return false;
            }
        }

        return true;
    }

	size_t FlatErrorRecord::hash() const {
        if (_hash == -1) {
            size_t hash = 0;
            for (const auto& classifier : _classifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            _hash = moja::hash::hash_combine(hash, _year, _module, _error);
        }

        return _hash;
    }

    std::string FlatErrorRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,module,error,area\n") % classifierStr).str();
    }

    std::string FlatErrorRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto errorStr = _error;
        boost::replace_all(errorStr, "\"", "'");

        return (boost::format("%1%,%2%,%3%,\"%4%\",%5%\n")
            % _year % classifierStr % _module % errorStr % _area).str();
    }
    
    std::vector<std::optional<std::string>> FlatErrorRecord::asVector() const {
        std::vector<std::optional<std::string>> row;
        row.push_back(pqxx::to_string(_year));
        for (const auto& value : _classifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_module);
        row.push_back(_error);
        row.push_back(pqxx::to_string(_area));

        return row;
    }

    void FlatErrorRecord::merge(const FlatErrorRecord& other) {
        _area += other._area;
    }
    // --

	// -- FlatAgeAreaRecord
    FlatAgeAreaRecord::FlatAgeAreaRecord(int year, std::vector<Poco::Nullable<std::string>>& classifierValues,
                                         std::string& landClass, std::string& ageClass, double area)
		: _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass), _area(area) { }

	bool FlatAgeAreaRecord::operator==(const FlatAgeAreaRecord& other) const {
        bool isEqual = _year == other._year
            && _landClass == other._landClass
            && _ageClass == other._ageClass;

        if (!isEqual) {
            return false;
        }

        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i].value("") != other._classifierValues[i].value("")) {
                return false;
            }
        }

        return true;
    }

	size_t FlatAgeAreaRecord::hash() const {
        if (_hash == -1) {
            size_t hash = 0;
            for (const auto& classifier : _classifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            _hash = moja::hash::hash_combine(hash, _year, _landClass, _ageClass);
        }

        return _hash;
    }

    std::string FlatAgeAreaRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,unfccc_land_class,age_range,area\n") % classifierStr).str();
    }

    std::string FlatAgeAreaRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%\n") % _year % classifierStr % _landClass % _ageClass % _area).str();
    }

    std::vector<std::optional<std::string>> FlatAgeAreaRecord::asVector() const {
        std::vector<std::optional<std::string>> row;
        row.push_back(pqxx::to_string(_year));
        for (const auto& value : _classifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_landClass);
        row.push_back(_ageClass);
        row.push_back(pqxx::to_string(_area));

        return row;
    }

    void FlatAgeAreaRecord::merge(const FlatAgeAreaRecord& other) {
		_area += other._area;
	}
    // --

    // -- FlatDisturbanceRecord
    FlatDisturbanceRecord::FlatDisturbanceRecord(
        int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
        const std::string& ageClass, const std::vector<Poco::Nullable<std::string>>& previousClassifierValues,
        const std::string& previousLandClass, const std::string& previousAgeClass,
        const std::string& disturbanceType, int disturbanceCode, double area
    ) : _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass),
        _previousClassifierValues(previousClassifierValues), _previousLandClass(previousLandClass),
        _previousAgeClass(previousAgeClass), _disturbanceType(disturbanceType), _disturbanceCode(disturbanceCode),
        _area(area) { }

    bool FlatDisturbanceRecord::operator==(const FlatDisturbanceRecord& other) const {
        bool isEqual = _year == other._year
            && _landClass == other._landClass
            && _ageClass == other._ageClass
            && _previousLandClass == other._previousLandClass
            && _previousAgeClass == other._previousAgeClass
            && _disturbanceType == other._disturbanceType
            && _disturbanceCode == other._disturbanceCode;

        if (!isEqual) {
            return false;
        }

        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i].value("") != other._classifierValues[i].value("")) {
                return false;
            }
        }

        for (int i = 0; i < other._previousClassifierValues.size(); i++) {
            if (_previousClassifierValues[i].value("") != other._previousClassifierValues[i].value("")) {
                return false;
            }
        }

        return true;
    }

    size_t FlatDisturbanceRecord::hash() const {
        if (_hash == -1) {
            size_t hash = 0;
            for (const auto& classifier : _classifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            for (const auto& classifier : _previousClassifierValues) {
                hash = moja::hash::hash_combine(hash, classifier.value(""));
            }

            _hash = moja::hash::hash_combine(
                hash, _year, _landClass, _ageClass, _previousLandClass, _previousAgeClass,
                _disturbanceType, _disturbanceCode);
        }

        return _hash;
    }

    std::string FlatDisturbanceRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames, "_previous");

        return (boost::format("year,%1%,unfccc_land_class,age_range,%2%,unfccc_land_class_previous,age_range_previous,disturbance_type,disturbance_code,area\n")
            % classifierStr % previousClassifierStr).str();
    }

    std::string FlatDisturbanceRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierValueString(_previousClassifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%,%6%,%7%,\"%8%\",%9%,%10%\n")
            % _year % classifierStr % _landClass % _ageClass % previousClassifierStr % _previousLandClass
            % _previousAgeClass % _disturbanceType % _disturbanceCode % _area).str();
    }

    std::vector<std::optional<std::string>> FlatDisturbanceRecord::asVector() const {
        std::vector<std::optional<std::string>> row;
        row.push_back(pqxx::to_string(_year));
        for (const auto& value : _classifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_landClass);
        row.push_back(_ageClass);
        for (const auto& value : _previousClassifierValues) {
            row.push_back(value.isNull() ? std::optional<std::string>(std::nullopt) : value.value());
        }

        row.push_back(_previousLandClass);
        row.push_back(_previousAgeClass);
        row.push_back(_disturbanceType);
        row.push_back(pqxx::to_string(_disturbanceCode));
        row.push_back(pqxx::to_string(_area));

        return row;
    }

    void FlatDisturbanceRecord::merge(const FlatDisturbanceRecord& other) {
        _area += other._area;
    }
    // --

}

}} // namespace moja::modules::cbm