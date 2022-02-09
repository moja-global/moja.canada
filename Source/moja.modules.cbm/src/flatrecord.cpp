#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

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
            && _disturbanceType == other._disturbanceType
            && _disturbanceCode == other._disturbanceCode
            && _srcPool == other._srcPool
            && _dstPool == other._dstPool;

        if (!isEqual) {
            return false;
        }

        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i] != other._classifierValues[i]) {
                return false;
            }
        }

        for (int i = 0; i < other._previousClassifierValues.size(); i++) {
            if (_previousClassifierValues[i] != other._previousClassifierValues[i]) {
                return false;
            }
        }

        return true;
    }

    size_t FlatFluxRecord::hash() const {
        if (_hash == -1) {
            size_t hash = 0;
            if (!_disturbanceType.isNull()) {
                hash = moja::hash::hash_combine(hash, _disturbanceType.value());
            }

            if (!_disturbanceCode.isNull()) {
                hash = moja::hash::hash_combine(hash, _disturbanceCode.value());
            }

            _hash = moja::hash::hash_combine(
                hash,
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                moja::hash::hash_range(_previousClassifierValues.begin(), _previousClassifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass, _previousLandClass, _previousAgeClass, _srcPool, _dstPool);
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
            if (_classifierValues[i] != other._classifierValues[i]) {
                return false;
            }
        }

        return true;
    }

    size_t FlatPoolRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass, _pool);
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
            if (_classifierValues[i] != other._classifierValues[i]) {
                return false;
            }
        }

        return true;
    }

	size_t FlatErrorRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                _year, _module, _error);
        }

        return _hash;
    }

    std::string FlatErrorRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,module,error,area\n") % classifierStr).str();
    }

    std::string FlatErrorRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto errorStr = (boost::format("\"%1%\"") % _error).str();

        return (boost::format("%1%,%2%,%3%,%4%,%5%\n")
            % _year % classifierStr % _module % errorStr % _area).str();
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
            if (_classifierValues[i] != other._classifierValues[i]) {
                return false;
            }
        }

        return true;
    }

	size_t FlatAgeAreaRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass);
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
            if (_classifierValues[i] != other._classifierValues[i]) {
                return false;
            }
        }

        for (int i = 0; i < other._previousClassifierValues.size(); i++) {
            if (_previousClassifierValues[i] != other._previousClassifierValues[i]) {
                return false;
            }
        }

        return true;
    }

    size_t FlatDisturbanceRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                moja::hash::hash_range(_previousClassifierValues.begin(), _previousClassifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass, _previousLandClass, _previousAgeClass, _disturbanceType, _disturbanceCode);
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

    void FlatDisturbanceRecord::merge(const FlatDisturbanceRecord& other) {
        _area += other._area;
    }
    // --

}

}} // namespace moja::modules::cbm
