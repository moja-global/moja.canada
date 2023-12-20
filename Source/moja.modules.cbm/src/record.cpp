#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <pqxx/stream_to>

#include "moja/modules/cbm/record.h"
#include "moja/hash.h"

namespace moja {
namespace modules {
namespace cbm {

    // -- DateRecord
     /**
	 * Constructor.
	 * 
	 * Initialise variables DateRecord._step as parameter step,DateRecord._year as parameter year,DateRecord._month as parameter month \n
     * DateRecord._day as parameter day,DateRecord._fracOfStep as parameter fracOfStep and DateRecord._yearsInStep as parameter yearsInStep. 
	 * 
	 * @param step int
     * @param year int
     * @param month int
     * @param day int
     * @param fracOfStep double
     * @param yearsInStep
	 * *************************/
    DateRecord::DateRecord(int step, int year, int month, int day,
                           double fracOfStep, double yearsInStep)
        : _step(step), _year(year), _month(month),
          _day(day), _fracOfStep(fracOfStep), _yearsInStep(yearsInStep) { }

     /**
	 * Check if DateRecord._step is equal to parameter other DateRecord._step. \n
     * return the boolean value
	 * 
	 * @param other DateRecord&
     * @return bool
	 * *************************/
    bool DateRecord::operator==(const DateRecord& other) const {
		return _step == other._step;
    }
     /**
	 * If DateRecord._hash is equal to -1, assign DateRecord._hash as moja::hash::hash_combine() using DateRecord._step as a parameter. \n
     * return DateRecord._hash.
	 * 
     * @return size_t
	 * *************************/
    size_t DateRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_step);
        }
        
        return _hash;
    }
     /**
	 * Return DateRow using DateRecord._id,DateRecord._step,DateRecord._month,DateRecord._day,DateRecord._fracOfStep,DateRecord._yearsInStep as paramters.
	 * 
	 * @return DateRow
	 * *************************/
    DateRow DateRecord::asPersistable() const {
        return DateRow{ _id, _step, _year, _month, _day, _fracOfStep, _yearsInStep };
    }

     /**
	 * Return stdDateRow using DateRecord._id,DateRecord._step,DateRecord._month,DateRecord._day,DateRecord._fracOfStep,DateRecord._yearsInStep as paramters.
	 * 
	 * @return StdDateRow
	 * *************************/
    StdDateRow DateRecord::asTuple() const {
        return StdDateRow{ _id, _step, _year, _month, _day, _fracOfStep, _yearsInStep };
    }
    // --

    // -- TemporalLocationRecord
     /**
     * Constructor.
     * 
	 * Initialise variables TemporalLocationRecord._classifierSetId as parameter classifierSetId,TemporalLocationRecord._dateId as parameter dateId, \n
     * TemporalLocationRecord._landClassId as parameter landClassId, TemporalLocationRecord._ageClassId as parameter ageClassId and TemporalLocationRecord._area as parameter area.
	 * 
	 * @param classifierSetId Int64
     * @param dateId Int64
     * @param landClassId,
     * @param ageClassId Poco::Nullable<Int64>
     * @param area double
	 * *************************/
    TemporalLocationRecord::TemporalLocationRecord(
		Int64 classifierSetId, Int64 dateId, Int64 landClassId, Poco::Nullable<Int64> ageClassId, double area)
        : _classifierSetId(classifierSetId), _dateId(dateId), _landClassId(landClassId),
		  _ageClassId(ageClassId), _area(area) { }

     /**
	 * Check if TemporalLocationRecord._classifierSetId is equal parameter other TemporalLocationRecord._classifierSetId and \n
     * TemporalLocationRecord._dateId is equal to parameter other TemporalLocationRecord._dateId and TemporalLocationRecord._landClassId is equal to parameter other TemporalLocationRecord._landClassId and \n
     * TemporalLocationRecord._ageClassId is equal to parameter other TemporalLocationRecord._ageClassId. \n
     * return rhe boolean value.
	 * 
	 * @param other TemporalLocationRecord&
     * @return bool
	 * *************************/
    bool TemporalLocationRecord::operator==(const TemporalLocationRecord& other) const {
        return _classifierSetId == other._classifierSetId
            && _dateId          == other._dateId
            && _landClassId     == other._landClassId
            && _ageClassId.value(-1) == other._ageClassId.value(-1);
    }
     /**
	 * If TemporalLocationRecord._hash is equal to -1, assign TemporalLocationRecord._hash as moja::hash::hash_combine() \n
     * using TemporalLocationRecord._classifierSetId,TemporalLocationRecord._dateId,TemporalLocationRecord._landClassId and TemporalLocationRecord._ageClassId as parameters. \n
	 * return TemporalLocationRecord._hash.
     * 
	 * @return size_t
	 * *************************/
    size_t TemporalLocationRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_combine(_classifierSetId, _dateId, _landClassId),
                _ageClassId.value(-1));
        }

        return _hash;
    }
     /**
	 * Return TemporalLocationRow using TemporalLocationRecord._classifierSetId,TemporalLocationRecord._dateId,TemporalLocationRecord._landClassId and TemporalLocationRecord._ageClassId as parameters.
	 * 
	 * @return TemporalLocationRow
	 * *************************/
    TemporalLocationRow TemporalLocationRecord::asPersistable() const {
        return TemporalLocationRow{ _id, _classifierSetId, _dateId, _landClassId, _ageClassId, _area };
    }

     /**
	 * Return a tuple object using TemporalLocationRecord._classifierSetId,TemporalLocationRecord._dateId,TemporalLocationRecord._landClassId and TemporalLocationRecord._ageClassId as parameters.
	 * 
	 * @return StdTemporalLocationRow
	 * *************************/
    StdTemporalLocationRow TemporalLocationRecord::asTuple() const {
        return std::make_tuple(_id, _classifierSetId, _dateId, _landClassId,
            _ageClassId.isNull() ? std::optional<Int64>() : _ageClassId.value(),
            _area);
    }

     /**
	 * Increase TemporalLocationRecord._area by parameter other TemporalLocationRecord._area.
	 * 
     * @param other TemporalLocationRecord&
	 * @return  void
	 * *************************/
    void TemporalLocationRecord::merge(const TemporalLocationRecord& other) {
		_area += other._area;
    }
    // --

    // -- ModuleInfoRecord
     /**
	 * Constructor.
     * 
     * Initialise variables ModuleInfoRecord._libType as parameter libType, ModuleInfoRecord._libInfoId as parameter libInfoId, \n
     * ModuleInfoRecord._moduleType as parameter moduleType,ModuleInfoRecord._moduleId as parameter moduleId,ModuleInfoRecord._moduleName as parameter moduleName.
     * 
	 * @param libType int
     * @param libInfoId int
     * @param moduleType int
     * @param moduleId
     * @param moduleName string
	 * *************************/
    ModuleInfoRecord::ModuleInfoRecord(
        int libType, int libInfoId,
        int moduleType, int moduleId, std::string moduleName)
        : _libType(libType), _libInfoId(libInfoId),
          _moduleType(moduleType), _moduleId(moduleId), _moduleName(moduleName) { }

     /**
	 * Check if ModuleInfoRecord._moduleName is equal to parameter other ModuleInfoRecord._moduleName. \n
     * return the boolean value.
	 * 
     * @param other ModuleInfoRecord&
	 * @return bool
	 * *************************/
    bool ModuleInfoRecord::operator==(const ModuleInfoRecord& other) const {
        return _moduleName == other._moduleName;
    }

     /**
	 * If ModuleInfoRecord._hash is equal to -1, \n
     * assign ModuleInfoRecord._hash as moja::hash::hash_combine() using ModuleInfoRecord._moduleName as a parameter. \n
	 * return ModuleInfoRecord._hash.
     * 
	 * @return size_t
	 * *************************/
    size_t ModuleInfoRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_moduleName);
        }

        return _hash;
    }

     /**
	 * Return ModuleInfoRow using ModuleInfoRecord._id,ModuleInfoRecord._libType,ModuleInfoRecord._moduleType,ModuleInfoRecord._moduleId,ModuleInfoRecord._moduleName as paramters.
	 * 
	 * @return ModuleInfoRow
	 * *************************/
    ModuleInfoRow ModuleInfoRecord::asPersistable() const {
        return ModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId, _moduleName };
    }

     /**
	 * Return StdModuleInfoRow using ModuleInfoRecord._id,ModuleInfoRecord._libType,ModuleInfoRecord._moduleType,ModuleInfoRecord._moduleId,ModuleInfoRecord._moduleName as paramters.
	 * 
	 * @return ModuleInfoRow
	 * *************************/
    StdModuleInfoRow ModuleInfoRecord::asTuple() const {
        return StdModuleInfoRow{ _id, _libType, _libInfoId, _moduleType, _moduleId, _moduleName };
    }
    // --

    // -- DisturbanceTypeRecord
     /**
     * 
     * Initialise variables DisturbanceTypeRecord._distTypeCode as parameter distTypeCode and DisturbanceTypeRecord._distTypeName as parameter distTypeName.
     * 
     * @param distTypeCode int
     * @param distTypename string
	 * *************************/
    DisturbanceTypeRecord::DisturbanceTypeRecord(int distTypeCode, std::string distTypeName)
        : _distTypeCode(distTypeCode), _distTypeName(distTypeName) { }

    /**
	 * Check if DisturbanceTypeRecord._distTypeCode is equal to parameter other DisturbanceTypeRecord._distTypeCode. \n
     * return the boolean value.
     * 
	 * @param other DisturbanceTypeRecord&
	 * @return bool
	 * *************************/
    bool DisturbanceTypeRecord::operator==(const DisturbanceTypeRecord& other) const {
        return _distTypeCode == other._distTypeCode;
    }

    /**
	 * If DisturbanceTypeRecord._hash is equal to -1, \n
     * assign DisturbanceTypeRecord._hash as moja::hash::hash_combine() using DisturbanceTypeRecord._distTypeCode.
	 * return DisturbanceTypeRecord._hash
	 * @return size_t
	 * *************************/
    size_t DisturbanceTypeRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_distTypeCode);
        }

        return _hash;
    }
    /**
	 * Return  DisturbanceTypeRow using DisturbanceTypeRecord._id,DisturbanceTypeRecord._distTypeCode and DisturbanceTypeRecord._distTypeName as parameters.
	 * 
	 * @return  DisturbanceTypeRow
	 * *************************/
    DisturbanceTypeRow DisturbanceTypeRecord::asPersistable() const {
        return DisturbanceTypeRow{ _id, _distTypeCode, _distTypeName };
    }

    /**
	 * Return StdDisturbanceTypeRow using DisturbanceTypeRecord._id,DisturbanceTypeRecord._distTypeCode and DisturbanceTypeRecord._distTypeName as parameters.
	 * 
	 * @return StdDisturbanceTypeRow
	 * *************************/
    StdDisturbanceTypeRow DisturbanceTypeRecord::asTuple() const {
        return StdDisturbanceTypeRow{ _id, _distTypeCode, _distTypeName };
    }
    // --

    // -- PoolInfoRecord
    /**
	 * Constructor
	 * 
     * Initialise PoolInfoRecord._name as parameter name.
     * 
     * @param name string
	 * *************************/
    PoolInfoRecord::PoolInfoRecord(const std::string& name) : _name(name) { }
    /**
	 * Check if PoolInfoRecord._name is equal to parameter other PoolInfoRecord._name. \n
     * return the boolean value.
	 * 
     * @param other PoolInfoRecord&
	 * @return bool
	 * *************************/
    bool PoolInfoRecord::operator==(const PoolInfoRecord& other) const {
		return _name == other._name;
    }
    /**
	 * If PoolInfoRecord._hash is equal to -1, \n
     * assign PoolInfoRecord._hash as moja::hash::hash_combine() using PoolInfoRecord._name. \n
	 * return PoolInfoRecord._hash.
     * 
	 * @return size_t
	 * *************************/
    size_t PoolInfoRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_name);
        }

        return _hash;
    }
    /**
	 * return PoolInfoRow using PoolInfoRecord._id and PoolInfoRecord._name as parameters.
	 * 
	 * @return PoolInfoRow
	 * *************************/
    PoolInfoRow PoolInfoRecord::asPersistable() const {
        return PoolInfoRow{ _id, _name };
    }
    // --

    // -- LandClassRecord
    /**
	 * Constructor.
	 * 
     * Initialise LandClassRecord._name as parameter name.
     * 
     * @param name string
	 * *************************/
    LandClassRecord::LandClassRecord(std::string name) : _name(name) { }

    /**
	 * Check if LandClassRecord._name is equal to other LandClassRecord._name. \n
     * return the boolean value.
     * 
	 * @param other LandClassRecord
	 * @return bool
	 * *************************/
    bool LandClassRecord::operator==(const LandClassRecord& other) const {
        return _name == other._name;
    }
    /**
	 * If LandClassRecord._hash is equal to -1, \n
     * assign LandClassRecord._hash as moja::hash::hash_combine using LandClassRecord._name. \n
	 * return LandClassRecord._hash.
     * 
	 * @return size_t
	 * *************************/
    size_t LandClassRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_name);
        }

        return _hash;
    }
    /**
	 * Return LandClassRow using LandClassRecord._id and LandClassRecord._name.
	 * 
	 * @return LandClassRow
	 * *************************/
    LandClassRow LandClassRecord::asPersistable() const {
        return LandClassRow{ _id, _name };
    }

    /**
	 * return StdLandClassRow using LandClassRecord._id and LandClassRecord._name.
	 * 
	 * @return StdLandClassRow
	 * *************************/
    StdLandClassRow LandClassRecord::asTuple() const {
        return StdLandClassRow{ _id, _name };
    }
    // --

    // -- ClassifierSetRecord
     /**
	 * Constructor.
     * 
     * Initialise ClassifierSetRecord._classifierValues as  parameter classifierValues.
     * 
	 * @param classifierValues vector<Nullable<string>>
	 * *************************/
    ClassifierSetRecord::ClassifierSetRecord(std::vector<Poco::Nullable<std::string>> classifierValues)
        : _classifierValues(classifierValues) { }

     /**
	 * For each value in ClassifierSetRecord._classifierValues, if the value is not equal to corresponding value in parameter other ClassifierSetRecord._classifierValues.\n
     * return false. \n
     * else, return true.
     * 
     * @param other ClassifierSetRecord&
	 * @return bool
	 * *************************/
    bool ClassifierSetRecord::operator==(const ClassifierSetRecord& other) const {
        for (int i = 0; i < other._classifierValues.size(); i++) {
            if (_classifierValues[i].value("") != other._classifierValues[i].value("")) {
                return false;
            }
        }

        return true;
    }

     /**
     * If ClassifierSetRecord._hash is equal to -1, \n
     * assign ClassifierSetRecord._hash as moja::hash::hash_range() using first value of ClassifierSetRecord._classifierValues,last value of ClassifierSetRecord._classifierValues,0 and moja::Hash().
     * return ClassifierSetRecord._hash.  
     * 
	 * @return size_t
	 * *************************/
    size_t ClassifierSetRecord::hash() const {
        if (_hash == -1) {
            _hash = 0;
            for (const auto& classifierValue : _classifierValues) {
                _hash = moja::hash::hash_combine(_hash, classifierValue.value(""));
            }
        }

        return _hash;
    }
    /**
	 * Return ClassifierSetRow using ClassifierSetRecord._id ,ClassifierSetRecord._classifierValues.
     * 
	 * @return ClassifierSetRow
	 * *************************/
    ClassifierSetRow ClassifierSetRecord::asPersistable() const {
        return ClassifierSetRow{ _id, _classifierValues };
    }
    // --

    // -- FluxRecord
    /**
	 * Constructor
     * 
     * Initialise variables FluxRecord._locationId as parameter locationId,FluxRecord._moduleId as parameter moduleId, \n
     * FluxRecord._distId as parameter distId,FluxRecord._srcPoolId as parameter srcPoolId,FluxRecord._dstPoolId as parameter dstPoolId and FluxRecord._flux as parameter flux.
     * 
	 * @param locationId Int64
     * @param moduleId Int64
     * @param distId Poco::Nullable<Int64>
     * @param srcPoolId Int64
     * @param dstPoolId Int64
     * @param flux double
	 * *************************/
    FluxRecord::FluxRecord(Int64 locationId, Int64 moduleId, Poco::Nullable<Int64> distId,
                           Int64 srcPoolId, Int64 dstPoolId, double flux)
        : _locationId(locationId), _moduleId(moduleId), _distId(distId),
          _srcPoolId(srcPoolId), _dstPoolId(dstPoolId), _flux(flux) { }

    /**
    * Check if FluxRecord._locationId is equal to parameter other FluxRecord._locationId,FluxRecord._moduleId is equal to parameter other FluxRecord._moduleId, \n
    * FluxRecord._distId is equal to parameter other FluxRecord._distId,FluxRecord._srcPoolId is equal to parameter other FluxRecord._srcPoolId and \n
    * FluxRecord._dstPoolId is equal to parameter other FluxRecord._dstPoolId.
    * return boolean values.
    * 
	* @return other FluxRecord&
	* @param bool
	* *************************/
    bool FluxRecord::operator==(const FluxRecord& other) const {
		return _locationId       == other._locationId
			&& _moduleId         == other._moduleId
            && _distId.value(-1) == other._distId.value(-1)
			&& _srcPoolId        == other._srcPoolId
			&& _dstPoolId        == other._dstPoolId;
    }

     /**
	 * If FluxRecord._hash is equal to -1, assign FluxRecord._hash as moja::hash::hash_combine() using FluxRecord._locationId,FluxRecord._moduleId, \n
     * FluxRecord._distId,FluxRecord._srcPoolId and FluxRecord._dstPoolId. return FluxRecord._hash.
     * 
	 * @return size_t
	 * *************************/
    size_t FluxRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(
                moja::hash::hash_combine(_locationId, _moduleId, _srcPoolId, _dstPoolId),
                _distId.value(-1));
        }

        return _hash;
    }

    /**
	 * Return FluxRow using FluxRecord._id,FluxRecord._locationId,FluxRecord._moduleId,FluxRecord._srcPoolId,FluxRecord._dstPoolId and FluxRecord._flux.
     * 
	 * @return FluxRow
	 * *************************/
    FluxRow FluxRecord::asPersistable() const {
        return FluxRow{
            _id, _locationId, _moduleId, _distId, _srcPoolId, _dstPoolId, _flux
        };
    }
    /**
	 * Return StdFluxRow using FluxRecord._id,FluxRecord._locationId,FluxRecord._moduleId,FluxRecord._srcPoolId,FluxRecord._dstPoolId and FluxRecord._flux.
	 * 
     * @return StdFluxRow
	 * *************************/
    StdFluxRow FluxRecord::asTuple() const {
        return StdFluxRow{
            _id, _locationId, _moduleId, _distId.isNull() ? std::optional<Int64>() : _distId.value(),
            _srcPoolId, _dstPoolId, _flux
        };
    }
    /**
	 * Increase FluxRecord._flux by parameter other FluxRecord._flux.
     * 
     * @param other FluxRecord&
	 * @return void
	 * *************************/
    void FluxRecord::merge(const FluxRecord& other) {
        _flux += other._flux;
    }
    // --

	// -- DisturbanceRecord
    /**
	 * Constructor
     * 
     * Initialise variables DisturbanceRecord._locationId as parameter locationId,DisturbanceRecord._distRecId as parameter distRecId, \n
     * DisturbanceRecord._previousLocationId as parameter previousLocationId and DisturbanceRecord._area as parameter area.
     * 
	 * @param locationId Int64
     * @param distRecId Int64
     * @param previousLocationId Int64
     * @param area double
	 * *************************/
	DisturbanceRecord::DisturbanceRecord(Int64 locationId, Int64 distRecId,
                                         Int64 previousLocationId, double area)
		: _locationId(locationId), _distRecId(distRecId), _previousLocationId(previousLocationId),
          _area(area) { }

     /**
	 * Check if DisturbanceRecord._locationId is equal to parameter other DisturbanceRecord._locationId, \n
     * DisturbanceRecord._distRecId is equal to parameter other DisturbanceRecord._distRecId and \n
     * DisturbanceRecord._previousLocationId is equal to  parameter other DisturbanceRecord._previousLocationId \n
     * return boolean values.
     * 
     * @param other DisturbanceRecord&
	 * @return bool
	 * *************************/
	bool DisturbanceRecord::operator==(const DisturbanceRecord& other) const {
		return _locationId == other._locationId
			&& _distRecId == other._distRecId
            && _previousLocationId == other._previousLocationId;
	}

     /**
	 * If DisturbanceRecord._hash is equal to -1, assign DisturbanceRecord._hash as moja::moja::hash_combine() using DisturbanceRecord._locationId,DisturbanceRecord._distRecId, \n
     * DisturbanceRecord._distRecId and DisturbanceRecord._previousLocationId as parameters. \n
     * return DisturbanceRecord._hash.
     * 
	 * @return size_t
	 * *************************/
	size_t DisturbanceRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_locationId, _distRecId, _previousLocationId);
		}

		return _hash;
	}
     /**
	 * Return DisturbanceRow using DisturbanceRecord._id,DisturbanceRecord._locationId,DisturbanceRecord._distRecId, \n
     * DisturbanceRecord._distRecId,DisturbanceRecord._previousLocationId and DisturbanceRecord._area.
     * 
	 * @return DisturbanceRow
	 * *************************/
	DisturbanceRow DisturbanceRecord::asPersistable() const {
		return DisturbanceRow{ _id, _locationId, _distRecId, _previousLocationId, _area };
	}
    /**
	 * Return StdDisturbanceRow using DisturbanceRecord._id,DisturbanceRecord._locationId,DisturbanceRecord._distRecId, \n
     * DisturbanceRecord._distRecId,DisturbanceRecord._previousLocationId and DisturbanceRecord._area.
     * 
	 * @return StdDisturbanceRow
	 * *************************/
    StdDisturbanceRow DisturbanceRecord::asTuple() const {
        return StdDisturbanceRow{ _id, _locationId, _distRecId, _previousLocationId, _area };
    }
    /**
	 * Increase DisturbanceRecord._area by  parameter other DisturbanceRecord._area.
     * 
     * @param other DisturbanceRecord&
	 * @return void
	 * *************************/
	void DisturbanceRecord::merge(const DisturbanceRecord& other) {
		_area += other._area;
	}
	// --

	// -- PoolRecord
    /**
	 * Constructor.
     * 
     * Initialise variables PoolRecord._locationId as parameter locationId,PoolRecord._poolId as parameter poolId, \n
     * PoolRecord._value as parameter value
     * 
	 * @param locationId Int64
     * @param poolId Int64
     * @param value double
	 * *************************/
    PoolRecord::PoolRecord(Int64 locationId, Int64 poolId, double value)
        : _locationId(locationId), _poolId(poolId), _value(value) { }
    /**
	 * Check if PoolRecord._locationId is equal to  parameter other PoolRecord._locationId and \n
     * PoolRecord._poolId is equal to  parameter other PoolRecord._poolId and \n
     * return the boolean value,
     * 
     * @param other PoolRecord&
	 * @return bool
	 * *************************/
    bool PoolRecord::operator==(const PoolRecord& other) const {
        return _locationId == other._locationId
            && _poolId     == other._poolId;
    }
     /**
	 * If PoolRecord._hash is equal to -1, assign PoolRecord._hash as moja::moja::hash_combine() using PoolRecord._locationId,PoolRecord._poolId. \n
     * return PoolRecord._hash.
	 * 
     * @return size_t
	 * *************************/
    size_t PoolRecord::hash() const {
        if (_hash == -1) {
            _hash = moja::hash::hash_combine(_locationId, _poolId);
        }

        return _hash;
    }
    /**
	 * Return PoolRow using PoolRecord._id,PoolRecord._locationId,PoolRecord._poolId and PoolRecord._value.
     * 
	 * @return PoolRow
	 * *************************/
    PoolRow PoolRecord::asPersistable() const {
        return PoolRow{ _id, _locationId, _poolId, _value };
    }
    /**
	 * Return StdPoolRow using PoolRecord._id,PoolRecord._locationId,PoolRecord._poolId and PoolRecord._value.
	 * 
     * @return StdPoolRow
	 * *************************/
    StdPoolRow PoolRecord::asTuple() const {
        return StdPoolRow{ _id, _locationId, _poolId, _value };
    }
    /**
	 * Increase PoolRecord._value by parameter other PoolRecord._value.
     * 
     * @param other PoolRecord&
	 * @return void
	 * *************************/
    void PoolRecord::merge(const PoolRecord& other) {
        _value += other._value;
    }
    // --

	// -- ErrorRecord
    /**
	 * Constructor.
     * 
     * Initialise variables ErrorRecord._module as parameter module and ErrorRecord._error as parameter error. 
     * 
	 * @param module string
     * @param error string
	 * *************************/
	ErrorRecord::ErrorRecord(std::string module, std::string error)
		: _module(module), _error(error) { }

    /**
	 * Check if ErrorRecord._module is equal parameter other ErrorRecord._module and ErrorRecord._error is equal to parameter other ErrorRecord._error. \n
     * return the boolean value.
     * 
     * @param other ErrorRecord&
	 * @return bool
	 * *************************/
	bool ErrorRecord::operator==(const ErrorRecord& other) const {
		return _module == other._module
			&& _error == other._error;
	}

    /**
	 * If ErrorRecord._hash is equal to -1, \n
     * assign ErrorRecord._hash as moja::hash::hash_combine() using ErrorRecord._module and ErrorRecord._error. \n
	 * return ErrorRecord._hash.
	 * 
     * @return size_t
	 * *************************/
	size_t ErrorRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_module, _error);
		}

		return _hash;
	}
    /**
	 * Return ErrorRow using ErrorRecord._id,ErrorRecord._module and ErrorRecord._error as parameters.
	 * 
     * @return ErrorRow
	 * *************************/
	ErrorRow ErrorRecord::asPersistable() const {
		return ErrorRow{ _id, _module, _error };
	}
    /**
	 * Return StdErrorRow using ErrorRecord._id,ErrorRecord._module and ErrorRecord._error as parameters.
	 * 
     * @return StdErrorRow
	 * *************************/
    StdErrorRow ErrorRecord::asTuple() const {
        return StdErrorRow{ _id, _module, _error };
    }
    // --

	// -- LocationErrorRecord
    /**
	 * Constructor.
     * 
     * Initialise LocationErrorRecord._locationId as parameter locationId and LocationErrorRecord._errorId as parameter errorId.
     * 
	 * @param locationId Int64
     * @param errorId Int64
	 * *************************/
	LocationErrorRecord::LocationErrorRecord(Int64 locationId, Int64 errorId)
		: _locationId(locationId), _errorId(errorId) { }

    /**
	 * Check if LocationErrorRecord._locationid is equal to parameter other LocationErrorRecord._locationdId and LocationErrorRecord._errorId is equal to  parameter other LocationErrorRecord._errorId.
     * 
     * @param other LocationErrorRecord&
	 * @return bool
	 * *************************/
	bool LocationErrorRecord::operator==(const LocationErrorRecord& other) const {
		return _locationId == other._locationId
			&& _errorId == other._errorId;
	}
    /**
	 * if LocationErrorRecord._hash is equal to -1, \n
     * assign LocationErrorRecord._hash as moja::hash::hash_combine() using LocationErrorRecord._locationId and LocationErrorRecord._errorId. \n
	 * return LocationErrorRecord._hash.
     * 
     * @return size_t
	 * *************************/
	size_t LocationErrorRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_locationId, _errorId);
		}

		return _hash;
	}
    /**
	 * Return LocationErrorRow using LocationErrorRecord._id,LocationErrorRecord._locationId and LocationErrorRecord._errorId.
	 * 
     * @return LocationErrorRow
	 * *************************/
	LocationErrorRow LocationErrorRecord::asPersistable() const {
		return LocationErrorRow{ _id, _locationId, _errorId };
	}

    /**
	 * Return StdLocationErrorRow using LocationErrorRecord._id,LocationErrorRecord._locationId and LocationErrorRecord._errorId.
	 * 
     * @return StdLocationErrorRow
	 * *************************/
    StdLocationErrorRow LocationErrorRecord::asTuple() const {
        return StdLocationErrorRow{ _id, _locationId, _errorId };
    }
    // --

	// -- AgeAreaRecord
    /**
	 * Constructor.
	 * 
     * Initialise AgeAreaRecord._locationId as parameter locationId,AgeAreaRecord._ageClassId as  parameter ageClassId and AgeAreaRecord._area as parameter area.
     * 
     * @param locationId Int64
     * @param ageClassId Int64
     * @param area double
	 * *************************/
	AgeAreaRecord::AgeAreaRecord(Int64 locationId, Int64 ageClassId,  double area)
		: _locationId(locationId), _ageClassId(ageClassId), _area(area) { }
    
    /**
	 * Check if AgeAreaRecord._locationId is equal to parameter other AgeAreaRecord._locationId and AgeAreaRecord._ageClassId is equal to parameter other AgeAreaRecord._ageClassId. \n
     * return boolean value.
     * 
	 * @param other AgeAreaRecord&
     * @return bool
	 * *************************/
	bool AgeAreaRecord::operator==(const AgeAreaRecord& other) const {
		return _locationId == other._locationId
			&& _ageClassId == other._ageClassId;
	}
    /**
	 * if AgeAreaRecord._hash is equal to -1, \n
     * assign AgeAreaRecord._hash as moja::hash::hash_combine() using AgeAreaRecord._locationId and AgeAreaRecord._ageClassId. \n
     * return AgeAreaRecord._hash.
     * 
	 * @return size_t
	 * *************************/
	size_t AgeAreaRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_locationId, _ageClassId);
		}

		return _hash;
	}

    /**
	 * Return AgeAreaRow using AgeAreaRecord._id,AgeAreaRecord._locationId,AgeAreaRecord._ageClassId and AgeAreaRecord._area.
     * 
	 * @return AgeAreaRow
	 * *************************/
	AgeAreaRow AgeAreaRecord::asPersistable() const {
		return AgeAreaRow{ _id, _locationId, _ageClassId, _area };
	}
    /**
	 * Return StdAgeAreaRow using AgeAreaRecord._id,AgeAreaRecord._locationId,AgeAreaRecord._ageClassId and AgeAreaRecord._area.
	 * 
     * @return StdAgeAreaRow
	 * *************************/
    StdAgeAreaRow AgeAreaRecord::asTuple() const {
        return StdAgeAreaRow{ _id, _locationId, _ageClassId, _area };
    }
    /**
	 * Increase AgeAreaRecord._area by parameter other AgeAreaRecord._area.
     * 
     * @param other AgeAreaRecord&
	 * return void
	 * *************************/
	void AgeAreaRecord::merge(const AgeAreaRecord& other) {
		_area += other._area;
	}
    // --

    // -- AgeClassRecord

    /**
	 * Constructor.
	 * 
     * Initialise AgeClassRecord._startAge as parameter startAge and AgeClassRecord._endAge as parameter endAge.
     * 
     * @param startAge Int64
     * @param endAge Int64
	 * *************************/
    AgeClassRecord::AgeClassRecord(Int64 startAge, Int64 endAge)
		: _startAge(startAge), _endAge(endAge) { }
	
    /**
	 * Check if AgeClassRecord._startAge is equal to parameter other AgeClassRecord._startAge and AgeClassRecord._endAge is equal to parameter other AgeClassRecord._endAge. \n
     * return the boolean values.
     * 
	 * @return bool
	 * *************************/
    bool AgeClassRecord::operator==(const AgeClassRecord& other) const {
		return _startAge == other._startAge
			&& _endAge == other._endAge;
	}

    /**
	 * if AgeClassRecord._hash is equal to -1, \n
     * assign AgeClassRecord._hash as moja::hash::hash_combine() using AgeClassRecord._startAge and AgeClassRecord._endAge parameters. \n
     * return AgeClassRecord._hash.
	 * 
     * @return size_t
	 * *************************/
	size_t AgeClassRecord::hash() const {
		if (_hash == -1) {
			_hash = moja::hash::hash_combine(_startAge, _endAge);
		}

		return _hash;
	}

    /**
	 * Return AgeClassRow using AgeClassRecord._id, AgeClassRecord._startAge and AgeClassRecord._endAge.
	 * 
     * @return AgeClassRow
	 * *************************/
	AgeClassRow AgeClassRecord::asPersistable() const {
		return AgeClassRow{ _id, _startAge, _endAge };
	}

    /**
	 * Return StdAgeClassRow using AgeClassRecord._id, AgeClassRecord._startAge and AgeClassRecord._endAge.
     * 
	 * @return StdAgeClassRow
	 * *************************/
    StdAgeClassRow AgeClassRecord::asTuple() const {
        return StdAgeClassRow{ _id, _startAge, _endAge };
    }
    // --
}}} // namespace moja::modules::cbm
