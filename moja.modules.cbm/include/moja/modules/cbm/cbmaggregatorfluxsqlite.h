#ifndef MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/notification.h"
#include "moja/hash.h"

#include <Poco/Tuple.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace moja {
namespace modules {
namespace cbm {

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
		//void onPostNotification(const flint::PostNotificationNotification::Ptr&) override;	

		void onOutputStep(const flint::OutputStepNotification::Ptr&) override;
	

	private:
		//int _curDateId;
		//int _curLocationId;
		int _curModuleInfoId;
		//int _curForestKey;
		int _curPoolId;
		int _curFactIdLD;
		int _curFactIdLU;
		// current classifier record id for a land unit classifier
		int _classfierSetRecordId = -1;

		typedef Poco::Tuple<int, int, int, int, int, int, double, double>	DateRecord;			// int id, int step, int substep, int year, int month, int day, double fracOfStep, double lengthOfStepInYears
        typedef Poco::Tuple<int, int, int, int>								LocationRecord;		// int id, int localDomainId, int landUnitId, int countyId
        typedef Poco::Tuple<int, int, int, int, int, std::string, int>		ModuleInfoRecord;	// int id, int libraryType, int libraryInfoId, int moduleType, int moduleId, string moduleName, int disturbanceType
        typedef Poco::Tuple<int, int, std::string>							PoolRecord;			// int id, int pool id, string poolName
        typedef std::tuple<int, int, int, int, int>							FactKey;			// int dateId, int locnId, int moduleId, int forestId, int srcPoolId, int dstPoolId
        typedef Poco::Tuple<Int64, FactKey, Int64, double, double>			FactRecord;			// Int64 id, FactKey key, Int64 itemCount, double areaSum, double fluxValue

		// Land Unit level Facts
		std::unordered_map<const FactKey, Int64, hash_tuple::hash<FactKey>> _factIdMapLU;
		std::vector<FactRecord> _factVectorLU;

		// Local Domain level Facts and Dimensions
		typedef std::vector<DateRecord> DateDimension;
        typedef std::vector<LocationRecord> LocationDimension;
        typedef std::vector<ModuleInfoRecord> ModuleInfoDimension;
        typedef std::vector<PoolRecord> PoolInfoDimension;
        typedef std::vector<FactRecord> FactDimension;

		std::unordered_map<const FactKey, Int64, hash_tuple::hash<FactKey>> _factIdMapLD;
		std::vector<FactRecord> _factVectorLD;

		double _landUnitArea;
		std::string _dbName;	

		DateDimension		_dateDimension;
		ModuleInfoDimension	_moduleInfoDimension;
		PoolInfoDimension	_poolDimension;				
	};

}}} // namespace moja::Modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_
