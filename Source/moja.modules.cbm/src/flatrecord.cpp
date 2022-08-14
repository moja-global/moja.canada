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
    
    /**
     * Return the concatentation of all the classifier names and suffix separated by a comma (,) \n
     * For each classifier name add the suffix to the end of the name.
     * 
     * @param classifierNames vector<std::string>&
     * @param suffix std::string
     * @return string
     */
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

    /**
     * Return the concatentation of all the classifier values separated by a comma (,)
     * 
     * @param classifierValues vector<Poco::Nullable<std::string>>&
     * @return string
     */
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
    /**
     * Constructor for FlatFluxRecord
     * 
     * @param year int
     * @param classifierValues vector<Poco::Nullable<std::string>>&
     * @param landClass std::string
     * @param ageClass std::string
     * @param previousClassifierValues vector<Poco::Nullable<std::string>>&
     * @param previousLandClass std::string
     * @param previousAgeClass std::string
     * @param disturbanceType Poco::Nullable<std::string>
     * @param disturbanceCode Poco::Nullable<int>
     * @param srcPool std::string
     * @param dstPool std::string
     * @param flux double
     */
    FlatFluxRecord::FlatFluxRecord(
        int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
        const std::string& ageClass, const std::vector<Poco::Nullable<std::string>>& previousClassifierValues,
        const std::string& previousLandClass, const std::string& previousAgeClass, const Poco::Nullable<std::string>& disturbanceType,
        const Poco::Nullable<int>& disturbanceCode, const std::string& srcPool, const std::string& dstPool, double flux
    ) : _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass),
        _previousClassifierValues(previousClassifierValues), _previousLandClass(previousLandClass),
        _previousAgeClass(previousAgeClass), _disturbanceType(disturbanceType), _disturbanceCode(disturbanceCode),
        _srcPool(srcPool), _dstPool(dstPool), _flux(flux) { }

     /**
     * Return if the members of the current FlatFluxRecord are equal to the members of the object other of FlatFluxRecord
     * 
     * @param other FlatFluxRecord&
     * @return bool
     */
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

    /**
     * Compute and return the value of FlatFluxRecord._hash if it is -1, and not already computed 
     * 
     * @return size_t
     */
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

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierNamesString() passing parameter classifierNames 
     * as an argument, the previous classifier string using FlatRecordHelper.BuildClassifierNamesString() passing classifierNames and
     * "_previous" as arguments \n
     * Return the header with the values of the current and previous classifier strings
     * 
     * @param classifierNames vector<std::string>&
     * @return string
     */
    std::string FlatFluxRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames, "_previous");

        return (boost::format("year,%1%,unfccc_land_class,age_range,%2%,unfccc_land_class_previous,age_range_previous,disturbance_type,disturbance_code,from_pool,to_pool,flux_tc\n")
            % classifierStr % previousClassifierStr).str();
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierValueString() passing FlatFluxRecord._classifierValues
     * as an argument, the previous classifier string using FlatRecordHelper.BuildClassifierValueString() passing FlatFluxRecord._classifierValues
     * as an argument \n
     * Return the concatentation of FlatFluxRecord._year, current classifier string, FlatFluxRecord._landClass, _ageClass, previous classifier string, FlatFluxRecord._previousLandClass,
     * FlatFluxRecord._previousAgeClass, FlatFluxRecord._disturbanceType, FlatFluxRecord._disturbanceCode, FlatFluxRecord._srcPool, FlatFluxRecord._dstPool, and FlatFluxRecord._flux
     * 
     * @return string
     */
    std::string FlatFluxRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierValueString(_previousClassifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%,%6%,%7%,\"%8%\",%9%,%10%,%11%,%12%\n")
            % _year % classifierStr % _landClass % _ageClass % previousClassifierStr % _previousLandClass
            % _previousAgeClass % _disturbanceType % _disturbanceCode % _srcPool % _dstPool % _flux).str();
    }

    /**
     * Merge the value of other._flux into FlatFluxRecord._flux
     * 
     * @param other FlatFluxRecord&
     * @return void
     */
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
    /**
     * Constructor of FlatPoolRecord 
     * 
     * @param year int
     * @param classifierValues vector<double>&
     * @param landClass string&
     * @param ageClass string&
     * @param pool string&
     * @param flux double
     * 
     */
    FlatPoolRecord::FlatPoolRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues,
                                   const std::string& landClass, const std::string& ageClass, const std::string& pool, double value)
        : _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass),
          _pool(pool), _value(value) { }

     /**
     * Return if the members of the current FlatPoolRecord are equal to the members of the object other of FlatPoolRecord
     * 
     * @param other FlatPoolRecord&
     * @return bool
     */
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

    /**
     * Compute and return the value of FlatPoolRecord._hash if it is -1, and not already computed 
     * 
     * @return size_t
     */
    size_t FlatPoolRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass, _pool);
        }

        return _hash;
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierNamesString() passing parameter classifierNames 
     * as an argument \n
     * Return the header with the values of the current classifier string
     * 
     * @param classifierNames vector<std::string>&
     * @return string
     */
    std::string FlatPoolRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,unfccc_land_class,age_range,pool,pool_tc\n") % classifierStr).str();
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierValueString passing FlatPoolRecord._classifierValues
     * as an argument \n
     * Return the concatenation of FlatPoolRecord._year, current classifier string, FlatPoolRecord._landClass, FlatPoolRecord._ageClass, FlatPoolRecord._pool, and FlatPoolRecord._value
     * 
     * @param classifierNames vector<std::string>&
     * @return string
     */
    std::string FlatPoolRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%,%6%\n")
            % _year % classifierStr % _landClass % _ageClass % _pool % _value).str();
    }

    /**
     * Merge the value of other._value into FlatPoolRecord._value
     * 
     * @param other FlatPoolRecord&
     * @return void
     */
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
    /**
     * Constructor for FlatErrorRecord
     * 
     * @param year int
     * @param classifierValues vector<Poco::Nullable<std::string>>&
     * @param module string&
     * @param error string&
     * @param area double
     */
    FlatErrorRecord::FlatErrorRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues,
                                     const std::string& module, const std::string& error, double area)
		: _year(year), _classifierValues(classifierValues), _module(module), _error(error), _area(area) { }

    /**
     * Return if the members of the current FlatErrorRecord are equal to the members of the object other of FlatErrorRecord
     * 
     * @param other FlatAgeAreaRecord&
     * @return bool
     */
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

    /**
     * Compute and return the value of FlatErrorRecord._hash if it is -1, and not already computed 
     * 
     * @return size_t
     */
	size_t FlatErrorRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                _year, _module, _error);
        }

        return _hash;
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierNamesString() passing parameter classifierNames 
     * as an argument \n
     * Return the header with the values of the current classifier string
     * 
     * @param classifierNames vector<std::string>&
     * @return string
     */
    std::string FlatErrorRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,module,error,area\n") % classifierStr).str();
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierValueString() passing FlatErrorRecord._classifierValues
     * as an argument \n
     * Return the concatentation of FlatErrorRecord._year, current classifier string, FlatErrorRecord._module, FlatErrorRecord._errorStr, and FlatErrorRecord._area
     * 
     * @return string
     */
    std::string FlatErrorRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto errorStr = _error;
        boost::replace_all(errorStr, "\"", "'");

        return (boost::format("%1%,%2%,%3%,\"%4%\",%5%\n")
            % _year % classifierStr % _module % errorStr % _area).str();
    }
    

    /**
     * Merge the value of other._area into FlatErrorRecord._area
     * 
     * @param other FlatErrorRecord&
     * @return void
     */
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
    /**
     * Constructor for FlatAgeAreaRecord
     * 
     * @param year int
     * @param classifierValues vector<Poco::Nullable<std::string>>&
     * @param landClass string
     * @param ageClass string
     * @param area double
     */
    FlatAgeAreaRecord::FlatAgeAreaRecord(int year, std::vector<Poco::Nullable<std::string>>& classifierValues,
                                         std::string& landClass, std::string& ageClass, double area)
		: _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass), _area(area) { }

    /**
     * Return if the members of the current FlatAgeAreaRecord are equal to the members of the object other of FlatAgeAreaRecord
     * 
     * @param other FlatAgeAreaRecord&
     * @return bool
     */
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

    /**
     * Compute and return the value of FlatAgeAreaRecord._hash if it is -1, and not already computed 
     * 
     * @return size_t
     */
	size_t FlatAgeAreaRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass);
        }

        return _hash;
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierNamesString() passing parameter classifierNames 
     * as an argument \n
     * Return the header with the values of the current classifier string
     * 
     * @param classifierNames vector<std::string>&
     * @return string
     */
    std::string FlatAgeAreaRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);

        return (boost::format("year,%1%,unfccc_land_class,age_range,area\n") % classifierStr).str();
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierValueString() passing FlatFluxRecord._classifierValues
     * as an argument \n
     * Return the concatentation of FlatAgeAreaRecord._year, current classifier string, FlatAgeAreaRecord._landClass, FlatAgeAreaRecord._ageClass, and FlatAgeAreaRecord._area
     * 
     * @return string
     */
    std::string FlatAgeAreaRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%\n") % _year % classifierStr % _landClass % _ageClass % _area).str();
    }


     /**
     * Merge the value of other._area into FlatAgeAreaRecord._area
     * 
     * @param other FlatAgeAreaRecord&
     * @return void
     */
	void FlatAgeAreaRecord::merge(const FlatAgeAreaRecord& other) {
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
    /**
     * Constructor for FlatDisturbanceRecord
     * 
     * @param year int
     * @param classifierValues vector<Poco::Nullable<std::string>>&
     * @param landClass string&
     * @param ageClass string&
     * @param previousClassifierValues vector<Poco::Nullable<std::string>>&
     * @param previousLandClass string&
     * @param previousAgeClass string&
     * @param disturbanceType string&
     * @param disturbanceCode int
     * @param area double
     */
    FlatDisturbanceRecord::FlatDisturbanceRecord(
        int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
        const std::string& ageClass, const std::vector<Poco::Nullable<std::string>>& previousClassifierValues,
        const std::string& previousLandClass, const std::string& previousAgeClass,
        const std::string& disturbanceType, int disturbanceCode, double area
    ) : _year(year), _classifierValues(classifierValues), _landClass(landClass), _ageClass(ageClass),
        _previousClassifierValues(previousClassifierValues), _previousLandClass(previousLandClass),
        _previousAgeClass(previousAgeClass), _disturbanceType(disturbanceType), _disturbanceCode(disturbanceCode),
        _area(area) { }


    /**
     * Return if the members of the current FlatDisturbanceRecord are equal to the members of the object other of FlatDisturbanceRecord
     * 
     * @param other FlatDisturbanceRecord&
     * @return bool
     */
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

    /**
     * Compute and return the value of FlatDisturbanceRecord._hash if it is -1, and not already computed 
     * 
     * @return size_t
     */
    size_t FlatDisturbanceRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_range(_classifierValues.begin(), _classifierValues.end(), 0, moja::Hash()),
                moja::hash::hash_range(_previousClassifierValues.begin(), _previousClassifierValues.end(), 0, moja::Hash()),
                _year, _landClass, _ageClass, _previousLandClass, _previousAgeClass, _disturbanceType, _disturbanceCode);
        }

        return _hash;
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierNamesString() passing parameter classifierNames 
     * as an argument, the previous classifier string using FlatRecordHelper.BuildClassifierNamesString() passing classifierNames and
     * "_previous" as arguments \n
     * Return the header with the values of the current and previous classifier strings
     * 
     * @param classifierNames vector<std::string>&
     * @return string
     */
    std::string FlatDisturbanceRecord::header(const std::vector<std::string>& classifierNames) const {
        auto classifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierNamesString(classifierNames, "_previous");

        return (boost::format("year,%1%,unfccc_land_class,age_range,%2%,unfccc_land_class_previous,age_range_previous,disturbance_type,disturbance_code,area\n")
            % classifierStr % previousClassifierStr).str();
    }

    /**
     * Get the current classifier string using FlatRecordHelper.BuildClassifierValueString() passing FlatFluxRecord._classifierValues
     * as an argument, the previous classifier string using FlatRecordHelper.BuildClassifierValueString() passing FlatFluxRecord._classifierValues
     * as an argument \n
     * Return the concatentation of FlatDisturbanceRecord._year, current classifier string, FlatDisturbanceRecord._landClass, _ageClass, previous classifier string, FlatDisturbanceRecord._previousLandClass,
     * FlatDisturbanceRecord._previousAgeClass, FlatDisturbanceRecord._disturbanceType, FlatDisturbanceRecord._disturbanceCode, FlatDisturbanceRecord._srcPool, FlatDisturbanceRecord._dstPool, and FlatDisturbanceRecord._flux
     * 
     * @return string
     */
    std::string FlatDisturbanceRecord::asPersistable() const {
        auto classifierStr = FlatRecordHelper::BuildClassifierValueString(_classifierValues);
        auto previousClassifierStr = FlatRecordHelper::BuildClassifierValueString(_previousClassifierValues);

        return (boost::format("%1%,%2%,%3%,%4%,%5%,%6%,%7%,\"%8%\",%9%,%10%\n")
            % _year % classifierStr % _landClass % _ageClass % previousClassifierStr % _previousLandClass
            % _previousAgeClass % _disturbanceType % _disturbanceCode % _area).str();
    }

    /**
     * Merge the value of other._area into FlatDisturbanceRecord._area
     * 
     * @param other FlatDisturbanceRecord&
     * @return void
     */
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
