#ifndef MOJA_MODULES_CBM_RECORD_H_
#define MOJA_MODULES_CBM_RECORD_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/types.h"
#include "moja/flint/record.h"

#include <Poco/Tuple.h>

#include <vector>
#include <optional>

namespace moja {
namespace modules {
namespace cbm {

    // id, step, year, month, day, frac of step, years in step
    typedef Poco::Tuple<Int64, int, int, int, int, double, double> DateRow;
    typedef std::tuple<Int64, int, int, int, int, double, double> StdDateRow;
    class CBM_API DateRecord {
    public:
        DateRecord(int step, int year, int month, int day,
                   double fracOfStep, double yearsInStep);

        ~DateRecord() {}
        
        bool operator==(const DateRecord& other) const;
        size_t hash() const;
        DateRow asPersistable() const;
        StdDateRow asTuple() const;
		void merge(const DateRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }
    
    private:
        mutable size_t _hash = -1;
		Int64 _id;

		// Data
        int _step;
        int _year;
        int _month;
        int _day;
        double _fracOfStep;
        double _yearsInStep;
    };

    // id, classifier set id, date id, land class id, age class id, area
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, Poco::Nullable<Int64>, double> TemporalLocationRow;
    typedef std::tuple<Int64, Int64, Int64, Int64, std::optional<Int64>, double> StdTemporalLocationRow;
    class CBM_API TemporalLocationRecord {
    public:
        TemporalLocationRecord(Int64 classifierSetId, Int64 dateId, Int64 landClassId,
                               Poco::Nullable<Int64> ageClassId, double area);

        ~TemporalLocationRecord() {}

        bool operator==(const TemporalLocationRecord& other) const;
        size_t hash() const;
        TemporalLocationRow asPersistable() const;
        StdTemporalLocationRow asTuple() const;
        void merge(const TemporalLocationRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _classifierSetId;
        Int64 _dateId;
        Int64 _landClassId;
        Poco::Nullable<Int64> _ageClassId;
        double _area;
    };

    // id, library type, library info id, module type, module id, module name
    typedef Poco::Tuple<Int64, int, int, int, int, std::string> ModuleInfoRow;
    typedef std::tuple<Int64, int, int, int, int, std::string> StdModuleInfoRow;
    class CBM_API ModuleInfoRecord {
    public:
        ModuleInfoRecord(int libType, int libInfoId, int moduleType, int moduleId, std::string moduleName);
        ~ModuleInfoRecord() {}

        bool operator==(const ModuleInfoRecord& other) const;
        size_t hash() const;
        ModuleInfoRow asPersistable() const;
        StdModuleInfoRow asTuple() const;
        void merge(const ModuleInfoRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		int _libType;
        int _libInfoId;
        int _moduleType;
        int _moduleId;
        std::string _moduleName;
	};

    // id, disturbance type code, disturbance type name
    typedef Poco::Tuple<Int64, int, std::string> DisturbanceTypeRow;
    typedef std::tuple<Int64, int, std::string> StdDisturbanceTypeRow;
    class CBM_API DisturbanceTypeRecord {
    public:
        DisturbanceTypeRecord(int distTypeCode, std::string distTypeName);
        ~DisturbanceTypeRecord() {}

        bool operator==(const DisturbanceTypeRecord& other) const;
        size_t hash() const;
        DisturbanceTypeRow asPersistable() const;
        StdDisturbanceTypeRow asTuple() const;
        void merge(const DisturbanceTypeRecord& other) {}
        void setId(Int64 id) { _id = id; }
        Int64 getId() const { return _id; }

    private:
        mutable size_t _hash = -1;
        Int64 _id;

        // Data
        int _distTypeCode;
        std::string _distTypeName;
    };

    // id, pool name
    typedef Poco::Tuple<Int64, std::string> PoolInfoRow;
    class CBM_API PoolInfoRecord {
    public:
        explicit PoolInfoRecord(const std::string& name);
        ~PoolInfoRecord() {}

        bool operator==(const PoolInfoRecord& other) const;
        size_t hash() const;
        PoolInfoRow asPersistable() const;
        void merge(const PoolInfoRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		std::string _name;
    };

    // id, UNFCCC land class
    typedef Poco::Tuple<Int64, std::string> LandClassRow;
    typedef std::tuple<Int64, std::string> StdLandClassRow;
    class CBM_API LandClassRecord {
    public:
        explicit LandClassRecord(std::string name);
        ~LandClassRecord() {}

        bool operator==(const LandClassRecord& other) const;
        size_t hash() const;
        LandClassRow asPersistable() const;
        StdLandClassRow asTuple() const;
        void merge(const LandClassRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		std::string _name;
    };

    // id, classifier values
    typedef Poco::Tuple<Int64, std::vector<Poco::Nullable<std::string>>> ClassifierSetRow;
    class CBM_API ClassifierSetRecord {
    public:
        explicit ClassifierSetRecord(std::vector<Poco::Nullable<std::string>> classifierValues);
        ~ClassifierSetRecord() {}

        bool operator==(const ClassifierSetRecord& other) const;
        size_t hash() const;
        ClassifierSetRow asPersistable() const;
        void merge(const ClassifierSetRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		std::vector<Poco::Nullable<std::string>> _classifierValues;
    };

	// id, locn id, disttype id, previous locn id, disturbed area
	typedef Poco::Tuple<Int64, Int64, Int64, Int64, double> DisturbanceRow;
    typedef std::tuple<Int64, Int64, Int64, Int64, double> StdDisturbanceRow;
    class CBM_API DisturbanceRecord {
	public:
		DisturbanceRecord(Int64 locationId, Int64 distRecId, Int64 previousLocationId, double area);
		~DisturbanceRecord() {}

		bool operator==(const DisturbanceRecord& other) const;
		size_t hash() const;
		DisturbanceRow asPersistable() const;
        StdDisturbanceRow asTuple() const;
        void merge(const DisturbanceRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
        Int64 _distRecId;
        Int64 _previousLocationId;
		double _area;
	};

    // id, locn id, module id, dist record id, src pool id, dst pool id, flux value
    typedef Poco::Tuple<Int64, Int64, Int64, Poco::Nullable<Int64>, Int64, Int64, double> FluxRow;
    typedef std::tuple<Int64, Int64, Int64, std::optional<Int64>, Int64, Int64, double> StdFluxRow;
    class CBM_API FluxRecord {
    public:
        FluxRecord(Int64 locationId, Int64 moduleId, Poco::Nullable<Int64> distId,
                   Int64 srcPoolId, Int64 dstPoolId, double flux);

        ~FluxRecord() {}

        bool operator==(const FluxRecord& other) const;
        size_t hash() const;
        FluxRow asPersistable() const;
        StdFluxRow asTuple() const;
        void merge(const FluxRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
        Int64 _moduleId;
        Poco::Nullable<Int64> _distId;
        Int64 _srcPoolId;
        Int64 _dstPoolId;
        double _flux;
    };

    // id, locn id, pool id, pool value
    typedef Poco::Tuple<Int64, Int64, Int64, double> PoolRow;
    typedef std::tuple<Int64, Int64, Int64, double> StdPoolRow;
    class CBM_API PoolRecord {
    public:
        PoolRecord(Int64 locationId, Int64 poolId, double value);
        ~PoolRecord() {}

        bool operator==(const PoolRecord& other) const;
        size_t hash() const;
        PoolRow asPersistable() const;
        StdPoolRow asTuple() const;
        void merge(const PoolRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
        Int64 _poolId;
        double _value;
    };

	// id, module name, error message
	typedef Poco::Tuple<Int64, std::string, std::string> ErrorRow;
    typedef std::tuple<Int64, std::string, std::string> StdErrorRow;
    class CBM_API ErrorRecord {
	public:
		ErrorRecord(std::string module, std::string error);
		~ErrorRecord() {};

		bool operator==(const ErrorRecord& other) const;
		size_t hash() const;
		ErrorRow asPersistable() const;
        StdErrorRow asTuple() const;
        void merge(const ErrorRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		std::string _module;
		std::string _error;
	};

	// id, loc n, error id
	typedef Poco::Tuple<Int64, Int64, Int64> LocationErrorRow;
    typedef std::tuple<Int64, Int64, Int64> StdLocationErrorRow;
    class CBM_API LocationErrorRecord {
	public:
		LocationErrorRecord(Int64 locationId, Int64 errorId);
		~LocationErrorRecord() {};

		bool operator==(const LocationErrorRecord& other) const;
		size_t hash() const;
		LocationErrorRow asPersistable() const;
        StdLocationErrorRow asTuple() const;
        void merge(const LocationErrorRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
		Int64 _errorId;
	};

	// id, locn id, ageClassId, area
	typedef Poco::Tuple<Int64, Int64, Int64, double> AgeAreaRow;
    typedef std::tuple<Int64, Int64, Int64, double> StdAgeAreaRow;
    class CBM_API AgeAreaRecord {
	public:
		AgeAreaRecord(Int64 locationId, Int64 ageClassId, double);
		~AgeAreaRecord() {}

		bool operator==(const AgeAreaRecord& other) const;
		size_t hash() const;
		AgeAreaRow asPersistable() const;
        StdAgeAreaRow asTuple() const;
        void merge(const AgeAreaRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
		Int64 _ageClassId;	
		double _area;
	};

	typedef Poco::Tuple<Int64, Int64, Int64> AgeClassRow;
    typedef std::tuple<Int64, Int64, Int64> StdAgeClassRow;
    class CBM_API AgeClassRecord {
	public:
		AgeClassRecord(Int64 start_age, Int64 end_age);
		~AgeClassRecord() {}

		bool operator==(const AgeClassRecord& other) const;
		size_t hash() const;
		AgeClassRow asPersistable() const;
        StdAgeClassRow asTuple() const;
        void merge(const AgeClassRecord& other) {}
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;
		Int64 _startAge;
		Int64 _endAge;
	};
}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_RECORD_H_
