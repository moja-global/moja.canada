#ifndef MOJA_MODULES_CBM_RECORD_H_
#define MOJA_MODULES_CBM_RECORD_H_

#include <unordered_map>

#include <Poco/Tuple.h>

#include "moja/types.h"

namespace moja {
namespace modules {
namespace cbm {

    template<class TPersistable>
    class Record {
    template<class T> friend class RecordAccumulator;
    protected:
        Int64 _id = -1;
        void setId(Int64 id) { _id = id; }
    
    public:
        virtual ~Record() = default;

        virtual size_t hash() = 0;
        virtual TPersistable asPersistable() = 0;
        virtual void merge(Record<TPersistable>* other) = 0;

        Int64 getId() { return _id; }
    };

    // id, step, substep, year, month, day, frac of step, years in step
    typedef Poco::Tuple<Int64, int, int, int, int, int, double, double> DateRow;
    class DateRecord : public Record<DateRow> {
    public:
        DateRecord(int step, int substep,
                   int year, int month, int day,
                   double fracOfStep, double yearsInStep);

        ~DateRecord() {}
        
        size_t hash();
        DateRow asPersistable();
        void merge(Record<DateRow>* other);
    
    private:
        int _step;
        int _substep;
        int _year;
        int _month;
        int _day;
        double _fracOfStep;
        double _yearsInStep;
    };

    // id, local domain id, land unit id
    typedef Poco::Tuple<Int64, int, Int64> LocationRow;
    class LocationRecord : public Record<LocationRow> {
    public:
        LocationRecord(int localDomainId, Int64 landUnitId);
        ~LocationRecord() {}

        size_t hash();
        LocationRow asPersistable();
        void merge(Record<LocationRow>* other);

    private:
        int _localDomainId;
        Int64 _landUnitId;
    };

    // id, library type, library info id, module type, module id, module name, disturbance type
    typedef Poco::Tuple<Int64, int, int, int, int, std::string, int> ModuleInfoRow;
    class ModuleInfoRecord : public Record<ModuleInfoRow> {
    public:
        ModuleInfoRecord(int libType, int libInfoId,
                         int moduleType, int moduleId, std::string moduleName,
                         int disturbanceType);

        ~ModuleInfoRecord() {}

        size_t hash();
        ModuleInfoRow asPersistable();
        void merge(Record<ModuleInfoRow>* other);

    private:
        int _libType;
        int _libInfoId;
        int _moduleType;
        int _moduleId;
        std::string _moduleName;
        int _disturbanceType;
    };

    // id, pool name
    typedef Poco::Tuple<Int64, std::string> PoolInfoRow;
    class PoolInfoRecord : public Record<PoolInfoRow> {
    public:
        PoolInfoRecord(std::string name);
        ~PoolInfoRecord() {}

        size_t hash();
        PoolInfoRow asPersistable();
        void merge(Record<PoolInfoRow>* other);

    private:
        std::string _name;
    };

    // id, classifier values
    typedef Poco::Tuple<Int64, std::vector<std::string>> ClassifierSetRow;
    class ClassifierSetRecord : public Record<ClassifierSetRow> {
    public:
        ClassifierSetRecord(std::vector<std::string> classifierValues);
        ~ClassifierSetRecord() {}

        size_t hash();
        ClassifierSetRow asPersistable();
        void merge(Record<ClassifierSetRow>* other);
    
    private:
        std::vector<std::string> _classifierValues;
    };


    // id, date id, locn id, module id, src pool id, dst pool id, item count, area sum, flux value
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, Int64, Int64, Int64, double, double> FluxRow;
    class FluxRecord : public Record<FluxRow> {
    public:
        FluxRecord(Int64 dateId, Int64 classifierSetId, Int64 moduleId,
                   Int64 srcPoolId, Int64 dstPoolId,
                   Int64 count, double area, double flux);

        ~FluxRecord() {}

        size_t hash();
        FluxRow asPersistable();
        void merge(Record<FluxRow>* other);

    private:
        Int64 _dateId;
        Int64 _classifierSetId;
        Int64 _moduleId;
        Int64 _srcPoolId;
        Int64 _dstPoolId;
        Int64 _count;
        double _area;
        double _flux;
    };

    // id, date id, classifier set id, pool id, pool value
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, double> PoolRow;
    class PoolRecord : public Record<PoolRow> {
    public:
        PoolRecord(Int64 dateId, Int64 classifierSetId, Int64 poolId, double value);
        ~PoolRecord() {}

        size_t hash();
        PoolRow asPersistable();
        void merge(Record<PoolRow>* other);

    private:
        Int64 _dateId;
        Int64 _classifierSetId;
        Int64 _poolId;
        double _value;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_RECORD_H_
