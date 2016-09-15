#ifndef MOJA_MODULES_CBM_RECORD_H_
#define MOJA_MODULES_CBM_RECORD_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/types.h"

#include "moja/flint/record.h"

#include <Poco/Tuple.h>
#include <vector>

namespace moja {
namespace modules {
namespace cbm {

    // id, step, year, month, day, frac of step, years in step
    typedef Poco::Tuple<Int64, int, int, int, int, double, double> DateRow;
    class CBM_API DateRecord : public flint::Record<DateRow> {
    public:
        DateRecord(int step, int year, int month, int day,
                   double fracOfStep, double yearsInStep);

        ~DateRecord() {}
        
        bool operator==(const Record<DateRow>& other) override;
        size_t hash() override;
        DateRow asPersistable() const override;
        void merge(Record<DateRow>* other) override;
    
    private:
        size_t _hash = -1;
        int _step;
        int _year;
        int _month;
        int _day;
        double _fracOfStep;
        double _yearsInStep;
    };

    // id, classifier set id, date id, land class id, area
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, double> TemporalLocationRow;
    class CBM_API TemporalLocationRecord : public flint::Record<TemporalLocationRow> {
    public:
        TemporalLocationRecord(Int64 classifierSetId, Int64 dateId, Int64 landClassId, double area);
        ~TemporalLocationRecord() {}

        bool operator==(const Record<TemporalLocationRow>& other) override;
        size_t hash() override;
        TemporalLocationRow asPersistable() const override;
        void merge(Record<TemporalLocationRow>* other) override;

    private:
        size_t _hash = -1;
        Int64 _classifierSetId;
        Int64 _dateId;
        Int64 _landClassId;
        double _area;
    };

    // id, library type, library info id, module type, module id, module name, disturbance type
    typedef Poco::Tuple<Int64, int, int, int, int, std::string, int> ModuleInfoRow;
    class CBM_API ModuleInfoRecord : public flint::Record<ModuleInfoRow> {
    public:
        ModuleInfoRecord(int libType, int libInfoId,
                         int moduleType, int moduleId, std::string moduleName,
                         int disturbanceType);

        ~ModuleInfoRecord() {}

        bool operator==(const Record<ModuleInfoRow>& other) override;
        size_t hash() override;
        ModuleInfoRow asPersistable() const override;
        void merge(Record<ModuleInfoRow>* other) override;

    private:
        size_t _hash = -1;
        int _libType;
        int _libInfoId;
        int _moduleType;
        int _moduleId;
        std::string _moduleName;
        int _disturbanceType;
    };

    // id, pool name
    typedef Poco::Tuple<Int64, std::string> PoolInfoRow;
    class CBM_API PoolInfoRecord : public flint::Record<PoolInfoRow> {
    public:
        explicit PoolInfoRecord(std::string name);
        ~PoolInfoRecord() {}

        bool operator==(const Record<PoolInfoRow>& other) override;
        size_t hash() override;
        PoolInfoRow asPersistable() const override;
        void merge(Record<PoolInfoRow>* other) override;

    private:
        size_t _hash = -1;
        std::string _name;
    };

    // id, UNFCCC land class
    typedef Poco::Tuple<Int64, std::string> LandClassRow;
    class CBM_API LandClassRecord : public flint::Record<LandClassRow> {
    public:
        explicit LandClassRecord(std::string name);
        ~LandClassRecord() {}

        bool operator==(const Record<LandClassRow>& other) override;
        size_t hash() override;
        LandClassRow asPersistable() const override;
        void merge(Record<LandClassRow>* other) override;

    private:
        size_t _hash = -1;
        std::string _name;
    };

    // id, classifier values
    typedef  Poco::Tuple<Int64, std::vector<std::string>> ClassifierSetRow;
    class CBM_API ClassifierSetRecord : public flint::Record<ClassifierSetRow> {
    public:
        explicit ClassifierSetRecord(std::vector<std::string> classifierValues);
        ~ClassifierSetRecord() {}

        bool operator==(const Record<ClassifierSetRow>& other) override;
        size_t hash() override;
        ClassifierSetRow asPersistable() const override;
        void merge(Record<ClassifierSetRow>* other) override;
    
    private:
        size_t _hash = -1;
        std::vector<std::string> _classifierValues;
    };


    // id, locn id, module id, src pool id, dst pool id, flux value
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, Int64, double> FluxRow;
    class CBM_API FluxRecord : public flint::Record<FluxRow> {
    public:
        FluxRecord(Int64 locationId, Int64 moduleId, Int64 srcPoolId,
                   Int64 dstPoolId, double flux);

        ~FluxRecord() {}

        bool operator==(const Record<FluxRow>& other) override;
        size_t hash() override;
        FluxRow asPersistable() const override;
        void merge(Record<FluxRow>* other) override;

    private:
        size_t _hash = -1;
        Int64 _locationId;
        Int64 _moduleId;
        Int64 _srcPoolId;
        Int64 _dstPoolId;
        double _flux;
    };

    // id, classifier set id, pool id, pool value
    typedef Poco::Tuple<Int64, Int64, Int64, double> PoolRow;
    class CBM_API PoolRecord : public flint::Record<PoolRow> {
    public:
        PoolRecord(Int64 locationId, Int64 poolId, double value);
        ~PoolRecord() {}

        bool operator==(const Record<PoolRow>& other) override;
        size_t hash() override;
        PoolRow asPersistable() const override;
        void merge(Record<PoolRow>* other) override;

    private:
        size_t _hash = -1;
        Int64 _locationId;
        Int64 _poolId;
        double _value;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_RECORD_H_
