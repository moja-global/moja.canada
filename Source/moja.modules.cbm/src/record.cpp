#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <pqxx/stream_to>

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
            _hash = moja::hash::hash_combine(_step);
        }
        
        return _hash;
    }

    DateRow DateRecord::asPersistable() const {
        return DateRow{ _id, _step, _year, _month, _day, _fracOfStep, _yearsInStep };
    }

    StdDateRow DateRecord::asTuple() const {
        return StdDateRow{ _id, _step, _year, _month, _day, _fracOfStep, _yearsInStep };
    }
    // --

    // -- TemporalLocationRecord
    TemporalLocationRecord::TemporalLocationRecord(
		Int64 classifierSetId, Int64 dateId, Int64 landClassId, Poco::Nullable<Int64> ageClassId, double area)
        : _classifierSetId(classifierSetId), _dateId(dateId), _landClassId(landClassId),
		  _ageClassId(ageClassId), _area(area) { }

    bool TemporalLocationRecord::operator==(const TemporalLocationRecord& other) const {
        return _classifierSetId == other._classifierSetId
            && _dateId          == other._dateId
            && _landClassId     == other._landClassId
            && _ageClassId      == other._ageClassId;
    }

    size_t TemporalLocationRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_classifierSetId, _dateId, _landClassId, _ageClassId);
        }

        return _hash;
    }

    TemporalLocationRow TemporalLocationRecord::asPersistable() const {
        return TemporalLocationRow{ _id, _classifierSetId, _dateId, _landClassId, _ageClassId, _area };
    }

    StdTemporalLocationRow TemporalLocationRecord::asTuple() const {
        return std::make_tuple(_id, _classifierSetId, _dateId, _landClassId,
            _ageClassId.isNull() ? std::optional<Int64>() : _ageClassId.value(),
            _area);
    }

    void TemporalLocationRecord::merge(const TemporalLocationRecord& other) {
		_area += other._area;
    }
    // --

    // -- ModuleInfoRecord
    ModuleInfoRecord::ModuleInfoRecord(
        int libType, int libInfoId,
        int moduleType, int moduleId, std::string moduleName)
        : _libType(libType), _libInfoId(libInfoId),
          _moduleType(moduleType), _moduleId(moduleId), _moduleName(moduleName) { }

    bool ModuleInfoRecord::operator==(const ModuleInfoRecord& other) const {
        return _moduleName == other._moduleName;
    }

    size_t ModuleInfoRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_moduleName);
        }

        return _hash;
    }

    ModuleInfoRow ModuleInfoRecord::asPersistable() const {
        return ModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId, _moduleName };
    }

    StdModuleInfoRow ModuleInfoRecord::asTuple() const {
        return StdModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId, _moduleName };
    }
    // --

    // -- DisturbanceTypeRecord
    DisturbanceTypeRecord::DisturbanceTypeRecord(int distTypeCode, std::string distTypeName)
        : _distTypeCode(distTypeCode), _distTypeName(distTypeName) { }

    bool DisturbanceTypeRecord::operator==(const DisturbanceTypeRecord& other) const {
        return _distTypeCode == other._distTypeCode;
    }

    size_t DisturbanceTypeRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_distTypeCode);
        }

        return _hash;
    }

    DisturbanceTypeRow DisturbanceTypeRecord::asPersistable() const {
        return DisturbanceTypeRow{ _id, _distTypeCode, _distTypeName };
    }

    StdDisturbanceTypeRow DisturbanceTypeRecord::asTuple() const {
        return StdDisturbanceTypeRow{ _id, _distTypeCode, _distTypeName };
    }
    // --

    // -- PoolInfoRecord
    PoolInfoRecord::PoolInfoRecord(const std::string& name) : _name(name) { }

    bool PoolInfoRecord::operator==(const PoolInfoRecord& other) const {
		return _name == other._name;
    }

    size_t PoolInfoRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_name);
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
            _hash = moja::hash::hash_combine(_name);
        }

        return _hash;
    }

    LandClassRow LandClassRecord::asPersistable() const {
        return LandClassRow{ _id, _name };
    }

    StdLandClassRow LandClassRecord::asTuple() const {
        return StdLandClassRow{ _id, _name };
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
            _hash = moja::hash::hash_range(_classifierValues.begin(),
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
    FluxRecord::FluxRecord(Int64 locationId, Int64 moduleId, Poco::Nullable<Int64> distId,
                           Int64 srcPoolId, Int64 dstPoolId, double flux)
        : _locationId(locationId), _moduleId(moduleId), _distId(distId),
          _srcPoolId(srcPoolId), _dstPoolId(dstPoolId), _flux(flux) { }

    bool FluxRecord::operator==(const FluxRecord& other) const {
		return _locationId == other._locationId
			&& _moduleId   == other._moduleId
            && _distId     == other._distId
			&& _srcPoolId  == other._srcPoolId
			&& _dstPoolId  == other._dstPoolId;
    }

    size_t FluxRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_locationId, _moduleId, _distId, _srcPoolId, _dstPoolId);
        }

        return _hash;
    }

    FluxRow FluxRecord::asPersistable() const {
        return FluxRow{
            _id, _locationId, _moduleId, _distId, _srcPoolId, _dstPoolId, _flux
        };
    }

    StdFluxRow FluxRecord::asTuple() const {
        return StdFluxRow{
            _id, _locationId, _moduleId, _distId.isNull() ? std::optional<Int64>() : _distId.value(),
            _srcPoolId, _dstPoolId, _flux
        };
    }

    void FluxRecord::merge(const FluxRecord& other) {
        _flux += other._flux;
    }
    // --

	// -- DisturbanceRecord
	DisturbanceRecord::DisturbanceRecord(Int64 locationId, Int64 distRecId,
                                         Int64 previousLocationId, double area)
		: _locationId(locationId), _distRecId(distRecId), _previousLocationId(previousLocationId),
          _area(area) { }

	bool DisturbanceRecord::operator==(const DisturbanceRecord& other) const {
		return _locationId == other._locationId
			&& _distRecId == other._distRecId
            && _previousLocationId == other._previousLocationId;
	}

	size_t DisturbanceRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_locationId, _distRecId, _previousLocationId);
		}

		return _hash;
	}

	DisturbanceRow DisturbanceRecord::asPersistable() const {
		return DisturbanceRow{ _id, _locationId, _distRecId, _previousLocationId, _area };
	}

    StdDisturbanceRow DisturbanceRecord::asTuple() const {
        return StdDisturbanceRow{ _id, _locationId, _distRecId, _previousLocationId, _area };
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
            _hash = moja::hash::hash_combine(_locationId, _poolId);
        }

        return _hash;
    }

    PoolRow PoolRecord::asPersistable() const {
        return PoolRow{ _id, _locationId, _poolId, _value };
    }

    StdPoolRow PoolRecord::asTuple() const {
        return StdPoolRow{ _id, _locationId, _poolId, _value };
    }

    void PoolRecord::merge(const PoolRecord& other) {
        _value += other._value;
    }
    // --

	// -- ErrorRecord
	ErrorRecord::ErrorRecord(std::string module, std::string error)
		: _module(module), _error(error) { }

	bool ErrorRecord::operator==(const ErrorRecord& other) const {
		return _module == other._module
			&& _error == other._error;
	}

	size_t ErrorRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_module, _error);
		}

		return _hash;
	}

	ErrorRow ErrorRecord::asPersistable() const {
		return ErrorRow{ _id, _module, _error };
	}

    StdErrorRow ErrorRecord::asTuple() const {
        return StdErrorRow{ _id, _module, _error };
    }
    // --

	// -- LocationErrorRecord
	LocationErrorRecord::LocationErrorRecord(Int64 locationId, Int64 errorId)
		: _locationId(locationId), _errorId(errorId) { }

	bool LocationErrorRecord::operator==(const LocationErrorRecord& other) const {
		return _locationId == other._locationId
			&& _errorId == other._errorId;
	}

	size_t LocationErrorRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_locationId, _errorId);
		}

		return _hash;
	}

	LocationErrorRow LocationErrorRecord::asPersistable() const {
		return LocationErrorRow{ _id, _locationId, _errorId };
	}

    StdLocationErrorRow LocationErrorRecord::asTuple() const {
        return StdLocationErrorRow{ _id, _locationId, _errorId };
    }
    // --

	// -- AgeAreaRecord
	AgeAreaRecord::AgeAreaRecord(Int64 locationId, Int64 ageClassId,  double area)
		: _locationId(locationId), _ageClassId(ageClassId), _area(area) { }

	bool AgeAreaRecord::operator==(const AgeAreaRecord& other) const {
		return _locationId == other._locationId
			&& _ageClassId == other._ageClassId;
	}

	size_t AgeAreaRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_locationId, _ageClassId);
		}

		return _hash;
	}

	AgeAreaRow AgeAreaRecord::asPersistable() const {
		return AgeAreaRow{ _id, _locationId, _ageClassId, _area };
	}

    StdAgeAreaRow AgeAreaRecord::asTuple() const {
        return StdAgeAreaRow{ _id, _locationId, _ageClassId, _area };
    }

	void AgeAreaRecord::merge(const AgeAreaRecord& other) {
		_area += other._area;
	}
    // --

    // -- AgeClassRecord
    AgeClassRecord::AgeClassRecord(Int64 startAge, Int64 endAge)
		: _startAge(startAge), _endAge(endAge) { }
	
    bool AgeClassRecord::operator==(const AgeClassRecord& other) const {
		return _startAge == other._startAge
			&& _endAge == other._endAge;
	}

	size_t AgeClassRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_startAge, _endAge);
		}

		return _hash;
	}

	AgeClassRow AgeClassRecord::asPersistable() const {
		return AgeClassRow{ _id, _startAge, _endAge };
	}

    StdAgeClassRow AgeClassRecord::asTuple() const {
        return StdAgeClassRow{ _id, _startAge, _endAge };
    }
    // --
}}} // namespace moja::modules::cbm
