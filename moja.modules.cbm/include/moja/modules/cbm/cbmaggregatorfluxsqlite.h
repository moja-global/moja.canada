#ifndef CBMAggregatorFluxSQLite_H_
#define CBMAggregatorFluxSQLite_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/notification.h"
#include "moja/hash.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace moja {
namespace modules {
namespace CBM {

	class CBM_API CBMAggregatorFluxSQLite : public flint::ModuleBase {
	public:
		CBMAggregatorFluxSQLite() : ModuleBase() {}
		virtual ~CBMAggregatorFluxSQLite() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes ModuleType() { return flint::ModuleTypes::System; };

		void RecordFluxSet();

		void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
		void onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& n) override;
		void onTimingInit(const flint::TimingInitNotification::Ptr& n) override;
		void onTimingShutdown(const flint::TimingShutdownNotification::Ptr& n) override;
		void onPostNotification(const flint::PostNotificationNotification::Ptr&) override;

	private:
		int _curDateId;
		int _curLocationId;
		int _curModuleInfoId;
		//int _curForestKey;
		int _curPoolId;
		int _curFactIdLD;
		int _curFactIdLU;

		typedef std::tuple<int, int, int, int, int, int, double, double>	DateRecord;			// int id, int step, int substep, int year, int month, int day, double fracOfStep, double lengthOfStepInYears
		typedef std::tuple<int, int, int, int>								LocationRecord;		// int id, int localDomainId, int landUnitId, int countyId
		typedef std::tuple<int, int, int, int, int, std::string, int>		ModuleInfoRecord;	// int id, int libraryType, int libraryInfoId, int moduleType, int moduleId, string moduleName, int disturbanceType
		//typedef std::tuple<int, int, int>									ForestRecord;		// int id, int forestType, int lossYear
		typedef std::tuple<int, int, std::string>							PoolRecord;			// int id, int pool id, string poolName
		typedef std::tuple<int, int, int, int, int, int>					FactKey;			// int dateId, int locnId, int moduleId, int forestId, int srcPoolId, int dstPoolId
		typedef std::tuple<Int64, FactKey, Int64, double, double>			FactRecord;			// Int64 id, FactKey key, Int64 itemCount, double areaSum, double fluxValue

		// Land Unit level Facts
		std::unordered_map<const FactKey, Int64, hash_tuple::hash<FactKey>> _factIdMapLU;
		std::vector<FactRecord> _factVectorLU;

		// Local Domain level Facts and Dimensions
		//std::vector<PoolRecord>			_countyMetaData;
		std::vector<DateRecord>			_dateDimension;
		std::vector<LocationRecord>		_locationDimension;
		std::vector<ModuleInfoRecord>	_moduleInfoDimension;
		//std::vector<ForestRecord>		_forestDimension;
		std::vector<PoolRecord>			_poolDimension;

		std::unordered_map<const FactKey, Int64, hash_tuple::hash<FactKey>> _factIdMapLD;
		std::vector<FactRecord> _factVectorLD;

		// Other data
		int _landUnitId;
		int _localDomainId;
		int _countyId;
		int _forestType;
		int _lossYear;
		double _landUnitArea;

		int		_spatialUnitId;
		double	_area;
		int		_age;
		Int64	_growthCurveId;
		int		_adminBoundryId;
		int		_ecoBoundryId;
		int		_climateTimeSeriesId;

		std::string _dbName;
	};

}}} // namespace moja::Modules::CBM

#endif // CBMAggregatorFluxSQLite_H_
