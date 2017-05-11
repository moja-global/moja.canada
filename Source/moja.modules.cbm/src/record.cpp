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

    bool DateRecord::operator==(const DateRecord& other) const {
		return _step == other._step;
    }

    size_t DateRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_step);
        }
        
        return _hash;
    }

    DateRow DateRecord::asPersistable() const {
        return DateRow{ _id, _step, _year, _month, _day, _fracOfStep, _yearsInStep };
    }
    // --

    // -- TemporalLocationRecord
    TemporalLocationRecord::TemporalLocationRecord(
		Int64 classifierSetId, Int64 dateId, Int64 landClassId, double area)
        : _classifierSetId(classifierSetId), _dateId(dateId),
		  _landClassId(landClassId), _area(area) { }

    bool TemporalLocationRecord::operator==(const TemporalLocationRecord& other) const {
        return _classifierSetId == other._classifierSetId
            && _dateId          == other._dateId
            && _landClassId     == other._landClassId;
    }

    size_t TemporalLocationRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_classifierSetId, _dateId, _landClassId);
        }

        return _hash;
    }

    TemporalLocationRow TemporalLocationRecord::asPersistable() const {
        return TemporalLocationRow{ _id, _classifierSetId, _dateId, _landClassId, _area };
    }

    void TemporalLocationRecord::merge(const TemporalLocationRecord& other) {
		_area += other._area;
    }
    // --

    // -- ModuleInfoRecord
    ModuleInfoRecord::ModuleInfoRecord(
        int libType, int libInfoId,
        int moduleType, int moduleId, std::string moduleName,
		std::string disturbanceTypeName, int disturbanceType)
        : _libType(libType), _libInfoId(libInfoId),
          _moduleType(moduleType), _moduleId(moduleId), _moduleName(moduleName),
          _disturbanceTypeName(disturbanceTypeName), _disturbanceType(disturbanceType) { }

    bool ModuleInfoRecord::operator==(const ModuleInfoRecord& other) const {
        return _moduleName			== other._moduleName
			&& _disturbanceTypeName == other._disturbanceTypeName;
    }

    size_t ModuleInfoRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_moduleName, _disturbanceTypeName);
        }

        return _hash;
    }

    ModuleInfoRow ModuleInfoRecord::asPersistable() const {
        return ModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId,
                              _moduleName, _disturbanceTypeName, _disturbanceType };
    }
    // --

    // -- PoolInfoRecord
    PoolInfoRecord::PoolInfoRecord(std::string name) : _name(name) { }

    bool PoolInfoRecord::operator==(const PoolInfoRecord& other) const {
		return _name == other._name;
    }

    size_t PoolInfoRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_name);
        }

        return _hash;
    }

    PoolInfoRow PoolInfoRecord::asPersistable() const {
        return PoolInfoRow{ _id, _name };
    }
    // --

    // -- LandClassRecord
    LandClassRecord::LandClassRecord(std::string name) : _name(name) { }

    bool LandClassRecord::operator==(const LandClassRecord& other) const {
        return _name == other._name;
    }

    size_t LandClassRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_name);
        }

        return _hash;
    }

    LandClassRow LandClassRecord::asPersistable() const {
        return LandClassRow{ _id, _name };
    }
    // --

    // -- ClassifierSetRecord
    ClassifierSetRecord::ClassifierSetRecord(std::vector<Poco::Nullable<std::string>> classifierValues)
        : _classifierValues(classifierValues) { }

    bool ClassifierSetRecord::operator==(const ClassifierSetRecord& other) const {
        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i] != other._classifierValues[i]) {
                return false;
            }
        }

        return true;
    }

    size_t ClassifierSetRecord::hash() const {
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
    // --

    // -- FluxRecord
    FluxRecord::FluxRecord(Int64 locationId, Int64 moduleId, Int64 srcPoolId,
                           Int64 dstPoolId, double flux)
        : _locationId(locationId), _moduleId(moduleId), _srcPoolId(srcPoolId),
          _dstPoolId(dstPoolId), _flux(flux) { }

    bool FluxRecord::operator==(const FluxRecord& other) const {
		return _locationId == other._locationId
			&& _moduleId   == other._moduleId
			&& _srcPoolId  == other._srcPoolId
			&& _dstPoolId  == other._dstPoolId;
    }

    size_t FluxRecord::hash() const {
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

    void FluxRecord::merge(const FluxRecord& other) {
        _flux += other._flux;
    }
    // --

	// -- DisturbanceRecord
	DisturbanceRecord::DisturbanceRecord(Int64 locationId, Int64 moduleId, double area)
		: _locationId(locationId), _moduleId(moduleId), _area(area) { }

	bool DisturbanceRecord::operator==(const DisturbanceRecord& other) const {
		return _locationId == other._locationId
			&& _moduleId   == other._moduleId;
	}

	size_t DisturbanceRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hashCombine(_locationId, _moduleId);
		}

		return _hash;
	}

	DisturbanceRow DisturbanceRecord::asPersistable() const {
		return DisturbanceRow{ _id, _locationId, _moduleId, _area };
	}

	void DisturbanceRecord::merge(const DisturbanceRecord& other) {
		_area += other._area;
	}
	// --

	// -- PoolRecord
    PoolRecord::PoolRecord(Int64 locationId, Int64 poolId, double value)
        : _locationId(locationId), _poolId(poolId), _value(value) { }

    bool PoolRecord::operator==(const PoolRecord& other) const {
        return _locationId == other._locationId
            && _poolId     == other._poolId;
    }

    size_t PoolRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hashCombine(_locationId, _poolId);
        }

        return _hash;
    }

    PoolRow PoolRecord::asPersistable() const {
        return PoolRow{ _id, _locationId, _poolId, _value };
    }

    void PoolRecord::merge(const PoolRecord& other) {
        _value += other._value;
    }
    // --

}}} // namespace moja::modules::cbm
