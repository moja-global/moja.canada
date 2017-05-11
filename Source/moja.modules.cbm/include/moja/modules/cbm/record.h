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
    class CBM_API DateRecord {
    public:
        DateRecord(int step, int year, int month, int day,
                   double fracOfStep, double yearsInStep);

        ~DateRecord() {}
        
        bool operator==(const DateRecord& other) const;
        size_t hash() const;
        DateRow asPersistable() const;
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

    // id, classifier set id, date id, land class id, area
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, double> TemporalLocationRow;
    class CBM_API TemporalLocationRecord {
    public:
        TemporalLocationRecord(Int64 classifierSetId, Int64 dateId, Int64 landClassId, double area);
        ~TemporalLocationRecord() {}

        bool operator==(const TemporalLocationRecord& other) const;
        size_t hash() const;
        TemporalLocationRow asPersistable() const;
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
        double _area;
    };

    // id, library type, library info id, module type, module id, module name, disturbance type name, disturbance type code
    typedef Poco::Tuple<Int64, int, int, int, int, std::string, std::string, int> ModuleInfoRow;
    class CBM_API ModuleInfoRecord {
    public:
        ModuleInfoRecord(int libType, int libInfoId, int moduleType, int moduleId,
						 std::string moduleName, std::string disturbanceTypeName,
						 int disturbanceType);

        ~ModuleInfoRecord() {}

        bool operator==(const ModuleInfoRecord& other) const;
        size_t hash() const;
        ModuleInfoRow asPersistable() const;
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
		std::string _disturbanceTypeName;
		int _disturbanceType;
	};

    // id, pool name
    typedef Poco::Tuple<Int64, std::string> PoolInfoRow;
    class CBM_API PoolInfoRecord {
    public:
        explicit PoolInfoRecord(std::string name);
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
    class CBM_API LandClassRecord {
    public:
        explicit LandClassRecord(std::string name);
        ~LandClassRecord() {}

        bool operator==(const LandClassRecord& other) const;
        size_t hash() const;
        LandClassRow asPersistable() const;
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

	// id, locn id, module id, disturbed area
	typedef Poco::Tuple<Int64, Int64, Int64, double> DisturbanceRow;
	class CBM_API DisturbanceRecord {
	public:
		DisturbanceRecord(Int64 locationId, Int64 moduleId, double area);

		~DisturbanceRecord() {}

		bool operator==(const DisturbanceRecord& other) const;
		size_t hash() const;
		DisturbanceRow asPersistable() const;
		void merge(const DisturbanceRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
		Int64 _moduleId;
		double _area;
	};

    // id, locn id, module id, src pool id, dst pool id, flux value
    typedef Poco::Tuple<Int64, Int64, Int64, Int64, Int64, double> FluxRow;
    class CBM_API FluxRecord {
    public:
        FluxRecord(Int64 locationId, Int64 moduleId, Int64 srcPoolId,
                   Int64 dstPoolId, double flux);

        ~FluxRecord() {}

        bool operator==(const FluxRecord& other) const;
        size_t hash() const;
        FluxRow asPersistable() const;
		void merge(const FluxRecord& other);
		void setId(Int64 id) { _id = id; }
		Int64 getId() const { return _id; }

	private:
		mutable size_t _hash = -1;
		Int64 _id;

		// Data
		Int64 _locationId;
        Int64 _moduleId;
        Int64 _srcPoolId;
        Int64 _dstPoolId;
        double _flux;
    };

    // id, classifier set id, pool id, pool value
    typedef Poco::Tuple<Int64, Int64, Int64, double> PoolRow;
    class CBM_API PoolRecord {
    public:
        PoolRecord(Int64 locationId, Int64 poolId, double value);
        ~PoolRecord() {}

        bool operator==(const PoolRecord& other) const;
        size_t hash() const;
        PoolRow asPersistable() const;
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

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_RECORD_H_
