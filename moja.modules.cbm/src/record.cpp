#include <boost/functional/hash.hpp>

#include "moja/modules/cbm/record.h"
#include "moja/hash.h"

namespace moja {
namespace modules {
namespace cbm {

    // -- DateRecord
    DateRecord::DateRecord(int step, int year, int month, int day,
                           double fracOfStep, double yearsInStep)
        : _step(step), _year(year), _month(month),
          _day(day), _fracOfStep(fracOfStep), _yearsInStep(yearsInStep) { }

    bool DateRecord::operator==(const Record<DateRow>& other) {
        auto otherRow = other.asPersistable();
        return _step == otherRow.get<1>();
    }

    size_t DateRecord::hash() {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_step);
        }
        
        return _hash;
    }

    DateRow DateRecord::asPersistable() const {
        return DateRow{ _id, _step, _year, _month, _day, _fracOfStep, _yearsInStep };
    }

    void DateRecord::merge(Record<DateRow>* other) { }
    // --

    // -- TemporalLocationRecord
    TemporalLocationRecord::TemporalLocationRecord(Int64 classifierSetId, Int64 dateId, Int64 landClassId, double area)
        : _classifierSetId(classifierSetId), _dateId(dateId), _landClassId(landClassId), _area(area) { }

    bool TemporalLocationRecord::operator==(const Record<TemporalLocationRow>& other) {
        auto otherRow = other.asPersistable();
        return _classifierSetId == otherRow.get<1>()
            && _dateId          == otherRow.get<2>()
            && _landClassId     == otherRow.get<3>();
    }

    size_t TemporalLocationRecord::hash() {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_classifierSetId, _dateId, _landClassId);
        }

        return _hash;
    }

    TemporalLocationRow TemporalLocationRecord::asPersistable() const {
        return TemporalLocationRow{ _id, _classifierSetId, _dateId, _landClassId, _area };
    }

    void TemporalLocationRecord::merge(Record<TemporalLocationRow>* other) {
        auto otherRow = other->asPersistable();
        _area += otherRow.get<4>();
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
        return _moduleName      == otherRow.get<5>()
            && _disturbanceType == otherRow.get<6>();
    }

    size_t ModuleInfoRecord::hash() {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_moduleName, _disturbanceType);
        }

        return _hash;
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
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_name);
        }

        return _hash;
    }

    PoolInfoRow PoolInfoRecord::asPersistable() const {
        return PoolInfoRow{ _id, _name };
    }

    void PoolInfoRecord::merge(Record<PoolInfoRow>* other) { }
    // --

    // -- LandClassRecord
    LandClassRecord::LandClassRecord(std::string name) : _name(name) { }

    bool LandClassRecord::operator==(const Record<LandClassRow>& other) {
        auto otherRow = other.asPersistable();
        return _name == otherRow.get<1>();
    }

    size_t LandClassRecord::hash() {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_name);
        }

        return _hash;
    }

    LandClassRow LandClassRecord::asPersistable() const {
        return LandClassRow{ _id, _name };
    }

    void LandClassRecord::merge(Record<LandClassRow>* other) { }
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
        if (_hash == -1) {
            _hash = moja::hash::hashRange(_classifierValues.begin(),
                                          _classifierValues.end(),
                                          0, moja::Hash());
        }

        return _hash;
    }

    ClassifierSetRow ClassifierSetRecord::asPersistable() const {
        return ClassifierSetRow{ _id, _classifierValues };
    }

    void ClassifierSetRecord::merge(Record<ClassifierSetRow>* other) { }
    // --

    // -- FluxRecord
    FluxRecord::FluxRecord(Int64 locationId, Int64 moduleId, Int64 srcPoolId,
                           Int64 dstPoolId, double flux)
        : _locationId(locationId), _moduleId(moduleId), _srcPoolId(srcPoolId),
          _dstPoolId(dstPoolId), _flux(flux) { }

    bool FluxRecord::operator==(const Record<FluxRow>& other) {
        auto otherRow = other.asPersistable();
        return _locationId == otherRow.get<1>()
            && _moduleId   == otherRow.get<2>()
            && _srcPoolId  == otherRow.get<3>()
            && _dstPoolId  == otherRow.get<4>();
    }

    size_t FluxRecord::hash() {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_locationId, _moduleId, _srcPoolId, _dstPoolId);
        }

        return _hash;
    }

    FluxRow FluxRecord::asPersistable() const {
        return FluxRow{
            _id, _locationId, _moduleId, _srcPoolId, _dstPoolId, _flux
        };
    }

    void FluxRecord::merge(Record<FluxRow>* other) {
        auto otherRow = other->asPersistable();
        _flux += otherRow.get<5>();
    }
    // --

    // -- PoolRecord
    PoolRecord::PoolRecord(Int64 locationId, Int64 poolId, double value)
        : _locationId(locationId), _poolId(poolId), _value(value) { }

    bool PoolRecord::operator==(const Record<PoolRow>& other) {
        auto otherRow = other.asPersistable();
        return _locationId == otherRow.get<1>()
            && _poolId     == otherRow.get<2>();
    }

    size_t PoolRecord::hash() {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_locationId, _poolId);
        }

        return _hash;
    }

    PoolRow PoolRecord::asPersistable() const {
        return PoolRow{ _id, _locationId, _poolId, _value };
    }

    void PoolRecord::merge(Record<PoolRow>* other) {
        auto otherRow = other->asPersistable();
        _value += otherRow.get<3>();
    }
    // --

}}} // namespace moja::modules::cbm
