#ifndef MOJA_MODULES_CBM_FLATRECORD_H_
#define MOJA_MODULES_CBM_FLATRECORD_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/types.h"
#include "moja/flint/record.h"

#include <vector>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API FlatRecordHelper {
    public:
        static const std::string BuildClassifierNamesString(const std::vector<std::string>& classifierNames, const std::string& suffix = "");
        static const std::string BuildClassifierValueString(const std::vector<Poco::Nullable<std::string>>& classifierValues);
    };

    class CBM_API FlatFluxRecord {
    public:
        FlatFluxRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
                       const std::string& ageClass, const std::vector<Poco::Nullable<std::string>>& previousClassifierValues,
                       const std::string& previousLandClass, const std::string& previousAgeClass,
                       const Poco::Nullable<std::string>& disturbanceType, const Poco::Nullable<int>& disturbanceCode,
                       const std::string& srcPool, const std::string& dstPool, double flux);

        ~FlatFluxRecord() {}

        bool operator==(const FlatFluxRecord& other) const;
        size_t hash() const;
        std::string header(const std::vector<std::string>& classifierNames) const;
        std::string asPersistable() const;
        void merge(const FlatFluxRecord& other);
        void setId(Int64 id) { _id = id; }
        Int64 getId() const { return _id; }

    private:
        mutable size_t _hash = -1;
        Int64 _id;

        // Data
        int _year;
        std::vector<Poco::Nullable<std::string>> _classifierValues;
        std::string _landClass;
        std::string _ageClass;
        std::vector<Poco::Nullable<std::string>> _previousClassifierValues;
        std::string _previousLandClass;
        std::string _previousAgeClass;
        Poco::Nullable<std::string> _disturbanceType;
        Poco::Nullable<int> _disturbanceCode;
        std::string _srcPool;
        std::string _dstPool;
        double _flux;
    };

    class CBM_API FlatPoolRecord {
    public:
        FlatPoolRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
                       const std::string& ageClass, const std::string& pool, double value);

        ~FlatPoolRecord() {}

        bool operator==(const FlatPoolRecord& other) const;
        size_t hash() const;
        std::string header(const std::vector<std::string>& classifierNames) const;
        std::string asPersistable() const;
        void merge(const FlatPoolRecord& other);
        void setId(Int64 id) { _id = id; }
        Int64 getId() const { return _id; }

    private:
        mutable size_t _hash = -1;
        Int64 _id;

        // Data
        int _year;
        std::vector<Poco::Nullable<std::string>> _classifierValues;
        std::string _landClass;
        std::string _ageClass;
        std::string _pool;
        double _value;
    };

    class CBM_API FlatErrorRecord {
    public:
        FlatErrorRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues,
                        const std::string& module, const std::string& error, double area);

        ~FlatErrorRecord() {};

        bool operator==(const FlatErrorRecord& other) const;
        size_t hash() const;
        std::string header(const std::vector<std::string>& classifierNames) const;
        std::string asPersistable() const;
        void merge(const FlatErrorRecord& other);
        void setId(Int64 id) { _id = id; }
        Int64 getId() const { return _id; }

    private:
        mutable size_t _hash = -1;
        Int64 _id;

        // Data
        int _year;
        std::vector<Poco::Nullable<std::string>> _classifierValues;
        std::string _module;
        std::string _error;
        double _area;
    };

    class CBM_API FlatAgeAreaRecord {
    public:
        FlatAgeAreaRecord(int year, std::vector<Poco::Nullable<std::string>>& classifierValues,
                          std::string& landClass, std::string& ageClass, double area);

        ~FlatAgeAreaRecord() {}

        bool operator==(const FlatAgeAreaRecord& other) const;
        size_t hash() const;
        std::string header(const std::vector<std::string>& classifierNames) const;
        std::string asPersistable() const;
        void merge(const FlatAgeAreaRecord& other);
        void setId(Int64 id) { _id = id; }
        Int64 getId() const { return _id; }

        int getYear() const { return _year; }
        const std::vector<Poco::Nullable<std::string>>& getClassifierValues() const { return _classifierValues; }
        const std::string& getLandClass() const { return _landClass; }
        const std::string& getAgeClass() const { return _ageClass; }

    private:
        mutable size_t _hash = -1;
        Int64 _id;

        // Data
        int _year;
        std::vector<Poco::Nullable<std::string>> _classifierValues;
        std::string _landClass;
        std::string _ageClass;
        double _area;
    };

    class CBM_API FlatDisturbanceRecord {
    public:
        FlatDisturbanceRecord(int year, const std::vector<Poco::Nullable<std::string>>& classifierValues, const std::string& landClass,
                              const std::string& ageClass, const std::vector<Poco::Nullable<std::string>>& previousClassifierValues,
                              const std::string& previousLandClass, const std::string& previousAgeClass,
                              const std::string& disturbanceType, int disturbanceCode, double area);

        ~FlatDisturbanceRecord() {}

        bool operator==(const FlatDisturbanceRecord& other) const;
        size_t hash() const;
        std::string header(const std::vector<std::string>& classifierNames) const;
        std::string asPersistable() const;
        void merge(const FlatDisturbanceRecord& other);
        void setId(Int64 id) { _id = id; }
        Int64 getId() const { return _id; }

    private:
        mutable size_t _hash = -1;
        Int64 _id;

        // Data
        int _year;
        std::vector<Poco::Nullable<std::string>> _classifierValues;
        std::string _landClass;
        std::string _ageClass;
        std::vector<Poco::Nullable<std::string>> _previousClassifierValues;
        std::string _previousLandClass;
        std::string _previousAgeClass;
        std::string _disturbanceType;
        int _disturbanceCode;
        double _area;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_FLATRECORD_H_
