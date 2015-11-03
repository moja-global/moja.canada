#ifndef MOJA_MODULES_CBM_RECORD_H_
#define MOJA_MODULES_CBM_RECORD_H_

#include <unordered_map>

#include <Poco/Tuple.h>

#include "moja/modules/cbm/_modules.cbm_exports.h"
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

        virtual bool operator==(const Record<TPersistable>& other) = 0;
        virtual size_t hash() = 0;
        virtual TPersistable asPersistable() const = 0;
        virtual void merge(Record<TPersistable>* other) = 0;

        Int64 getId() { return _id; }
    };

    // id, step, substep, year, month, day, frac of step, years in step
    typedef  Poco::Tuple<Int64, int, int, int, int, int, double, double> DateRow;
    class CBM_API DateRecord : public Record<DateRow> {
    public:
        DateRecord(int step, int substep,
                   int year, int month, int day,
                   double fracOfStep, double yearsInStep);

        ~DateRecord() {}
        
        bool operator==(const Record<DateRow>& other) override;
        size_t hash() override;
        DateRow asPersistable() const override;
        void merge(Record<DateRow>* other) override;
    
    private:
        int _step;
        int _substep;
        int _year;
        int _month;
        int _day;
        double _fracOfStep;
        double _yearsInStep;
    };

    // id, classifier set id, area
    typedef Poco::Tuple<Int64, Int64, Int64, double> LocationRow;
    class CBM_API LocationRecord : public Record<LocationRow> {
    public:
        LocationRecord(Int64 landUnitId, Int64 classifierSetId, double area);
        ~LocationRecord() {}

        bool operator==(const Record<LocationRow>& other) override;
        size_t hash() override;
        LocationRow asPersistable() const override;
        void merge(Record<LocationRow>* other) override;

    private:
        Int64 _landUnitId;
        Int64 _classifierSetId;
        double _area;
    };

    // id, library type, library info id, module type, module id, module name, disturbance type
    typedef  Poco::Tuple<Int64, int, int, int, int, std::string, int> ModuleInfoRow;
    class CBM_API ModuleInfoRecord : public Record<ModuleInfoRow> {
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
        int _libType;
        int _libInfoId;
        int _moduleType;
        int _moduleId;
        std::string _moduleName;
        int _disturbanceType;
    };

    // id, pool name
    typedef  Poco::Tuple<Int64, std::string> PoolInfoRow;
    class CBM_API PoolInfoRecord : public Record<PoolInfoRow> {
    public:
        PoolInfoRecord(std::string name);
        ~PoolInfoRecord() {}

        bool operator==(const Record<PoolInfoRow>& other) override;
        size_t hash() override;
        PoolInfoRow asPersistable() const override;
        void merge(Record<PoolInfoRow>* other) override;

    private:
        std::string _name;
    };

    // id, classifier values
    typedef  Poco::Tuple<Int64, std::vector<std::string>> ClassifierSetRow;
    class CBM_API ClassifierSetRecord : public Record<ClassifierSetRow> {
    public:
        ClassifierSetRecord(std::vector<std::string> classifierValues);
        ~ClassifierSetRecord() {}

        bool operator==(const Record<ClassifierSetRow>& other) override;
        size_t hash() override;
        ClassifierSetRow asPersistable() const override;
        void merge(Record<ClassifierSetRow>* other) override;
    
    private:
        std::vector<std::string> _classifierValues;
    };


    // id, date id, locn id, module id, src pool id, dst pool id, flux value
    typedef  Poco::Tuple<Int64, Int64, Int64, Int64, Int64, Int64, double> FluxRow;
    class CBM_API FluxRecord : public Record<FluxRow> {
    public:
        FluxRecord(Int64 dateId, Int64 locationId, Int64 moduleId,
                   Int64 srcPoolId, Int64 dstPoolId, double flux);

        ~FluxRecord() {}

        bool operator==(const Record<FluxRow>& other) override;
        size_t hash() override;
        FluxRow asPersistable() const override;
        void merge(Record<FluxRow>* other) override;

    private:
        Int64 _dateId;
        Int64 _locationId;
        Int64 _moduleId;
        Int64 _srcPoolId;
        Int64 _dstPoolId;
        double _flux;
    };

    // id, date id, classifier set id, pool id, pool value
    typedef  Poco::Tuple<Int64, Int64, Int64, Int64, double> PoolRow;
    class CBM_API PoolRecord : public Record<PoolRow> {
    public:
        PoolRecord(Int64 dateId, Int64 locationId, Int64 poolId, double value);
        ~PoolRecord() {}

        bool operator==(const Record<PoolRow>& other) override;
        size_t hash() override;
        PoolRow asPersistable() const override;
        void merge(Record<PoolRow>* other) override;

    private:
        Int64 _dateId;
        Int64 _locationId;
        Int64 _poolId;
        double _value;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_RECORD_H_
