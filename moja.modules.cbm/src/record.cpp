#include <boost/functional/hash.hpp>

#include "moja/modules/cbm/record.h"

namespace moja {
namespace modules {
namespace cbm {

    // -- DateRecord
    DateRecord::DateRecord(int step, int substep,
                           int year, int month, int day,
                           double fracOfStep, double yearsInStep)
        : _step(step), _substep(substep), _year(year), _month(month),
          _day(day), _fracOfStep(fracOfStep), _yearsInStep(yearsInStep) { }

    size_t DateRecord::hash() {
        size_t seed = 0;
        boost::hash_combine(seed, _step);
        boost::hash_combine(seed, _substep);
        return seed;
    }

    DateRow DateRecord::asPersistable() {
        return DateRow{ _id, _step, _substep, _year, _month, _day, _fracOfStep,
                        _yearsInStep };
    }

    void DateRecord::merge(Record<DateRow>* other) { }
    // --

    // -- LocationRecord
    LocationRecord::LocationRecord(Int64 classifierSetId, double area)
        : _classifierSetId(classifierSetId), _area(area) { }

    size_t LocationRecord::hash() {
        size_t seed = 0;
        boost::hash_combine(seed, _classifierSetId);
        return seed;
    }

    LocationRow LocationRecord::asPersistable() {
        return LocationRow{ _id, _classifierSetId, _area };
    }

    void LocationRecord::merge(Record<LocationRow>* other) {
        auto otherRow = other->asPersistable();
        _area += otherRow.get<2>();
    }
    // --

    // -- ModuleInfoRecord
    ModuleInfoRecord::ModuleInfoRecord(
        int libType, int libInfoId,
        int moduleType, int moduleId, std::string moduleName,
        int disturbanceType)
        : _libType(libType), _libInfoId(libInfoId),
          _moduleType(moduleType), _moduleId(moduleId), _moduleName(moduleName),
          _disturbanceType(disturbanceType) { }

    size_t ModuleInfoRecord::hash() {
        size_t seed = 0;
        boost::hash_combine(seed, _moduleName);
        boost::hash_combine(seed, _disturbanceType);
        return seed;
    }

    ModuleInfoRow ModuleInfoRecord::asPersistable() {
        return ModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId,
                              _moduleName, _disturbanceType };
    }

    void ModuleInfoRecord::merge(Record<ModuleInfoRow>* other) { }
    // --

    // -- PoolInfoRecord
    PoolInfoRecord::PoolInfoRecord(std::string name) : _name(name) { }

    size_t PoolInfoRecord::hash() {
        size_t seed = 0;
        boost::hash_combine(seed, _name);
        return seed;
    }

    PoolInfoRow PoolInfoRecord::asPersistable() {
        return PoolInfoRow{ _id, _name };
    }

    void PoolInfoRecord::merge(Record<PoolInfoRow>* other) { }
    // --

    // -- ClassifierSetRecord
    ClassifierSetRecord::ClassifierSetRecord(std::vector<std::string> classifierValues)
        : _classifierValues(classifierValues) { }

    size_t ClassifierSetRecord::hash() {
        size_t seed = 0;
        for (auto value : _classifierValues) {
            boost::hash_combine(seed, value);
        }

        return seed;
    }

    ClassifierSetRow ClassifierSetRecord::asPersistable() {
        return ClassifierSetRow{ _id, _classifierValues };
    }

    void ClassifierSetRecord::merge(Record<ClassifierSetRow>* other) { }
    // --

    // -- FluxRecord
    FluxRecord::FluxRecord(Int64 dateId, Int64 locationId, Int64 moduleId,
                           Int64 srcPoolId, Int64 dstPoolId, double flux)
        : _dateId(dateId), _locationId(locationId), _moduleId(moduleId),
          _srcPoolId(srcPoolId), _dstPoolId(dstPoolId), _flux(flux) { }

    size_t FluxRecord::hash() {
        size_t seed = 0;
        boost::hash_combine(seed, _dateId);
        boost::hash_combine(seed, _locationId);
        boost::hash_combine(seed, _moduleId);
        boost::hash_combine(seed, _srcPoolId);
        boost::hash_combine(seed, _dstPoolId);
        return seed;
    }

    FluxRow FluxRecord::asPersistable() {
        return FluxRow{
            _id, _dateId, _locationId, _moduleId, _srcPoolId, _dstPoolId, _flux
        };
    }

    void FluxRecord::merge(Record<FluxRow>* other) {
        auto otherRow = other->asPersistable();
        _flux += otherRow.get<6>();
    }
    // --

    // -- PoolRecord
    PoolRecord::PoolRecord(Int64 dateId, Int64 locationId, Int64 poolId, double value)
        : _dateId(dateId), _locationId(locationId), _poolId(poolId), _value(value) { }

    size_t PoolRecord::hash() {
        size_t seed = 0;
        boost::hash_combine(seed, _dateId);
        boost::hash_combine(seed, _locationId);
        boost::hash_combine(seed, _poolId);
        return seed;
    }

    PoolRow PoolRecord::asPersistable() {
        return PoolRow{ _id, _dateId, _locationId, _poolId, _value };
    }

    void PoolRecord::merge(Record<PoolRow>* other) {
        auto otherRow = other->asPersistable();
        _value += otherRow.get<4>();
    }
    // --

}}} // namespace moja::modules::cbm
