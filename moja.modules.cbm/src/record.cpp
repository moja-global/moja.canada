#include <boost/functional/hash.hpp>

#include "moja/modules/cbm/record.h"
#include "moja/hash.h"

namespace moja {
namespace modules {
namespace cbm {

    // -- DateRecord
    DateRecord::DateRecord(int step, int substep,
                           int year, int month, int day,
                           double fracOfStep, double yearsInStep)
        : _step(step), _substep(substep), _year(year), _month(month),
          _day(day), _fracOfStep(fracOfStep), _yearsInStep(yearsInStep) { }

    bool DateRecord::operator==(const Record<DateRow>& other) {
        auto otherRow = other.asPersistable();
        return _step == otherRow.get<1>() && _substep == otherRow.get<2>();
    }

    size_t DateRecord::hash() {
        return moja::hash::hash_combine(_step, _substep);
    }

    DateRow DateRecord::asPersistable() const {
        return DateRow{ _id, _step, _substep, _year, _month, _day, _fracOfStep, _yearsInStep };
    }

    void DateRecord::merge(Record<DateRow>* other) { }
    // --

    // -- LocationRecord
    LocationRecord::LocationRecord(Int64 classifierSetId, double area)
        : _classifierSetId(classifierSetId), _area(area) { }

    bool LocationRecord::operator==(const Record<LocationRow>& other) {
        auto otherRow = other.asPersistable();
        return _classifierSetId == otherRow.get<1>();
    }

    size_t LocationRecord::hash() {
        return moja::hash::hash_combine(_classifierSetId);
    }

    LocationRow LocationRecord::asPersistable() const {
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

    bool ModuleInfoRecord::operator==(const Record<ModuleInfoRow>& other) {
        auto otherRow = other.asPersistable();
        return _moduleName == otherRow.get<5>() && _disturbanceType == otherRow.get<6>();
    }

    size_t ModuleInfoRecord::hash() {
        return moja::hash::hash_combine(_moduleName, _disturbanceType);
    }

    ModuleInfoRow ModuleInfoRecord::asPersistable() const {
        return ModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId,
                              _moduleName, _disturbanceType };
    }

    void ModuleInfoRecord::merge(Record<ModuleInfoRow>* other) { }
    // --

    // -- PoolInfoRecord
    PoolInfoRecord::PoolInfoRecord(std::string name) : _name(name) { }

    bool PoolInfoRecord::operator==(const Record<PoolInfoRow>& other) {
        auto otherRow = other.asPersistable();
        return _name == otherRow.get<1>();
    }

    size_t PoolInfoRecord::hash() {
        return moja::hash::hash_combine(_name);
    }

    PoolInfoRow PoolInfoRecord::asPersistable() const {
        return PoolInfoRow{ _id, _name };
    }

    void PoolInfoRecord::merge(Record<PoolInfoRow>* other) { }
    // --

    // -- ClassifierSetRecord
    ClassifierSetRecord::ClassifierSetRecord(std::vector<std::string> classifierValues)
        : _classifierValues(classifierValues) { }

    bool ClassifierSetRecord::operator==(const Record<ClassifierSetRow>& other) {
        auto otherValues = other.asPersistable().get<1>();
        for (int i = 0; i < otherValues.size(); i++) {
            if (_classifierValues[i] != otherValues[i]) {
                return false;
            }
        }

        return true;
    }

    size_t ClassifierSetRecord::hash() {
        return moja::hash::hash_range(_classifierValues.begin(),
                                      _classifierValues.end(),
                                      0, moja::Hash());
    }

    ClassifierSetRow ClassifierSetRecord::asPersistable() const {
        return ClassifierSetRow{ _id, _classifierValues };
    }

    void ClassifierSetRecord::merge(Record<ClassifierSetRow>* other) { }
    // --

    // -- FluxRecord
    FluxRecord::FluxRecord(Int64 dateId, Int64 locationId, Int64 moduleId,
                           Int64 srcPoolId, Int64 dstPoolId, double flux)
        : _dateId(dateId), _locationId(locationId), _moduleId(moduleId),
          _srcPoolId(srcPoolId), _dstPoolId(dstPoolId), _flux(flux) { }

    bool FluxRecord::operator==(const Record<FluxRow>& other) {
        auto otherRow = other.asPersistable();
        return _dateId == otherRow.get<1>()
            && _locationId == otherRow.get<2>()
            && _moduleId == otherRow.get<3>()
            && _srcPoolId == otherRow.get<4>()
            && _dstPoolId == otherRow.get<5>();
    }

    size_t FluxRecord::hash() {
        return moja::hash::hash_combine(
            _dateId, _locationId, _moduleId, _srcPoolId, _dstPoolId);
    }

    FluxRow FluxRecord::asPersistable() const {
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

    bool PoolRecord::operator==(const Record<PoolRow>& other) {
        auto otherRow = other.asPersistable();
        return _dateId == otherRow.get<1>()
            && _locationId == otherRow.get<2>()
            && _poolId == otherRow.get<3>();
    }

    size_t PoolRecord::hash() {
        return moja::hash::hash_combine(_dateId, _locationId, _poolId);
    }

    PoolRow PoolRecord::asPersistable() const {
        return PoolRow{ _id, _dateId, _locationId, _poolId, _value };
    }

    void PoolRecord::merge(Record<PoolRow>* other) {
        auto otherRow = other->asPersistable();
        _value += otherRow.get<4>();
    }
    // --

}}} // namespace moja::modules::cbm
