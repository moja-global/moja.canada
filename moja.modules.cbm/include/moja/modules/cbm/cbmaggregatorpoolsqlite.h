#ifndef MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_

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

#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/Exception.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/SQLiteException.h"

#include <boost/algorithm/string/join.hpp>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

using namespace Poco::Data;
using Poco::format;
using Poco::NotFoundException;

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API CBMAggregatorPoolSQLite : public flint::ModuleBase {
			public:
				CBMAggregatorPoolSQLite() : ModuleBase() {}
				virtual ~CBMAggregatorPoolSQLite() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;	

				void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
				void onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& n) override;
				void onTimingInit(const flint::TimingInitNotification::Ptr& n) override;
				void onTimingShutdown(const flint::TimingShutdownNotification::Ptr& n) override;
				//void onPostNotification(const flint::PostNotificationNotification::Ptr&) override;
				
				void onOutputStep(const flint::OutputStepNotification::Ptr&) override;

				void onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n) override;
				
				void recordPoolsSet();

				void recordAfterSpinningupPoolsSet();
			private:
				// accumulated Pool record Id - local domain scope
				Int64 _curPoolRecordId;				

				// current classifier record id for a land unit classifier
				int _classfierSetRecordId;									
				
				// int dateId, int classifierSetId
				typedef std::tuple<int, int>	PoolKey;					

				// pool record lookup map <PoolKey, PoolRecordId>, Pool values are aggregated/grouped on <dateId, ClassifierSetId>
				std::unordered_map<const PoolKey, Int64, hash_tuple::hash<PoolKey>> _poolIdMapLU;				

				//pool records - each record of the pool values are stored in the vector
				std::vector<std::vector<double>> _poolValueVectorLU;
				
				// current land unit area
				double _landUnitArea;	

				// current land unit Id
				Int64 _luid;

				// output database name
				std::string _dbName;

				// record of pool names defined in the configuration file
				std::vector<std::string> _poolNames;				

				// record of classifier name defined in the configuration file
				std::vector<std::string> _classifierNames;					

				//private methods

				// drop if exists, and create ClassifierSet table
				void createClassifierSetTable(Session session, std::vector<std::string> classifierNames);

				// build insert classifier SQL
				std::string buildClassifierInsertSQL(std::vector<std::string> classifierNames);

				// insert classifier set records into the output SQLite database
				void insertClassifierSetRecords(Session& session, std::string insertClassifierSetsSQL, Int64 classifierSetID, std::vector<std::string>classifierValues, int numberOfClassifiers);

				// drop if exists, and create the Pools table
				void createPoolsTable(Session session, std::vector<std::string> poolNames);

				// build insert pool value SQL
				std::string buildPoolValueInsertSQL(std::vector<std::string> poolNames);

				// insert pool records into the output SQLite database
				void insertPoolRecord(Session session, std::string insertPoolsSql, Int64 recordIDKey, int dateRecordId, int classifierSetId, std::vector<double>poolValues);

				// help method to split a string into a vector
				std::vector<std::string> split(std::string str, char delimiter);

				// based on classifier information of the landunit, set or get the classifier set Id
				void updateLandUnitInformation();
			};
		}
	}
} // namespace moja::Modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_