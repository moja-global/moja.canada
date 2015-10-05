#include "moja/modules/cbm/cbmaggregatorpoolsqlite.h"
#include "moja/flint/matrix.h"
#include "moja/flint/landunitcontroller.h"
//#include "moja/modules/sleek/metadata.h"
#include "moja/flint/operationmatrix.h"
#include "moja/observer.h"
#include <moja/mathex.h>



//Poco::SharedPtr<Poco::Data::Session> _pSession = 0;

// Some poco settings to help SQLite
#if defined(POCO_SQLITE_SETTINGS_FOR_PERF)
#pragma main.page_size = 4096;
#pragma main.cache_size = 100000;
#pragma main.locking_mode = EXCLUSIVE;
#pragma main.synchronous = OFF;	// NORMAL
#pragma main.journal_mode = MEMORY;	// WAL
//#pragma main.cache_size = 5000;
#pragma main.temp_store = MEMORY;
#endif

#include <iomanip>      // std::setprecision

// --------------------------------------------------------------------------------------------

using namespace std;

// accumulated Date record Id - local domain scope
int _curDateId;

// accumulated ClassifierSet record Id - local domain scope
int _curClassifierSetId;

// list of classifier values as a string delimited by ",", unique classifier set Id
std::unordered_map<std::string, int> _classifierSetLU;

// int id, int step, int substep, int year, int month, int day, double fracOfStep, double lengthOfStepInYears
typedef std::tuple<int, int, int, int, int, int, double, double>	DateRecord;

// date Dimensions - record the simulation date	- time steps	
std::vector<DateRecord>	_dateDimension;

namespace moja {
	namespace modules {
		namespace cbm {

			// --------------------------------------------------------------------------------------------

			void CBMAggregatorPoolSQLite::configure(const DynamicObject& config) {
				_dbName = config["databasename"].convert<string>();
			}			

			void CBMAggregatorPoolSQLite::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(*this, &IModule::onLocalDomainInit));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainShutdownNotification>>(*this, &IModule::onLocalDomainShutdown));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(*this, &IModule::onTimingInit));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingShutdownNotification>>(*this, &IModule::onTimingShutdown));			
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::OutputStepNotification>>(*this, &IModule::onOutputStep));
				notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::PreTimingSequenceNotification>>(*this, &IModule::onPreTimingSequence));
			}

			// --------------------------------------------------------------------------------------------

			void CBMAggregatorPoolSQLite::createClassifierSetTable(Session session, std::vector<std::string> classifierNames) /*throw()*/ {
				session << "DROP TABLE IF EXISTS ClassifierSets", now;			
				string createTablePart1 = "CREATE TABLE ClassifierSets (ID UNSIGNED BIG INT, ";
				string createTablePart2 = boost::algorithm::join(classifierNames, " TEXT, ");
				string createPoolSql = createTablePart1 + createTablePart2 + " TEXT)";
				session << createPoolSql, now;
			}

			std::string CBMAggregatorPoolSQLite::buildClassifierInsertSQL(std::vector<std::string> classifierNames) {
				int classifierSize = classifierNames.size();
				string insertClassifierSetSql = "INSERT INTO ClassifierSets VALUES(";
				for (int i = 0; i <= classifierSize; i++){
					if (i > 0){
						insertClassifierSetSql.append(", ");
					}
					insertClassifierSetSql.append("?");
				}
				insertClassifierSetSql.append(")");

				return insertClassifierSetSql;
			}
			
			void CBMAggregatorPoolSQLite::insertClassifierSetRecords(Session& session, std::string insertClassifierSetsSQL, Int64 classifierSetID, std::vector<string>classifierValues, int numberOfClassifiers) {
				Statement insert = (session << insertClassifierSetsSQL, useRef(classifierSetID));
				for (int i = 0; i < numberOfClassifiers; i++){
					insert, useRef(classifierValues[i]);
				}
				insert.execute();
			}

			std::vector<string> CBMAggregatorPoolSQLite::split(std::string str, char delimiter) {
				std::vector<string> internal;
				stringstream ss(str); // Turn the string into a stream.
				string tok;

				while (getline(ss, tok, delimiter)) {
					internal.push_back(tok);
				}

				return internal;
			}

			void CBMAggregatorPoolSQLite::createPoolsTable(Session session, std::vector<std::string> poolNames)	{
				session << "DROP TABLE IF EXISTS Pools", now;
				string createTablePart1 = "CREATE TABLE Pools (ID UNSIGNED BIG INT, DateDimId UNSIGNED BIG INT, ClassifierSetId UNSIGNED BIG INT, Area FLOAT, ";
				string createTablePart2 = boost::algorithm::join(poolNames, " FLOAT, ");
				string createPoolSql = createTablePart1 + createTablePart2 + " FLOAT)";

				session << createPoolSql, now;
			}

			std::string CBMAggregatorPoolSQLite::buildPoolValueInsertSQL(std::vector<std::string> poolNames) {
				int poolSize = poolNames.size();
				string insertPoolSql = "INSERT INTO Pools VALUES(?, ?, ?, "; //id, dateId, classifierSetId
				for (int i = 0; i <= poolSize; i++) {
					if (i > 0){
						insertPoolSql.append(", ");
					}
					insertPoolSql.append("?");
				}
				insertPoolSql.append(")");

				return insertPoolSql;
			}

			void CBMAggregatorPoolSQLite::insertPoolRecord(Session session, std::string insertPoolsSql, Int64 recordIDKey, int dateRecordId, int classifierSetId, std::vector<double>poolValues) {
				Statement insert = (session << insertPoolsSql, useRef(recordIDKey), useRef(dateRecordId), useRef(classifierSetId));
				for (int i = 0; i < poolValues.size(); i++){
					insert, useRef(poolValues[i]);					
				}	

				insert.execute();
			}

			void CBMAggregatorPoolSQLite::updateLandUnitInformation(){
				_landUnitArea = _landUnitData->getVariable("LandUnitArea")->value();				
				_luid = _landUnitData->getVariable("LandUnitId")->value();

				// get the land unit classifier set information							
				const auto& landUnitClassifierSet = _landUnitData->getVariable("classifier_set")->value()
					.extract<std::vector<DynamicObject>>();

				_classifierNames.clear();
				std::vector<std::string> _classifierSet;
				for (const auto& item : landUnitClassifierSet) {
					std::string key = item["classifier_name"].convert<std::string>();
					std::replace(key.begin(), key.end(), '.', ' ');
					std::replace(key.begin(), key.end(), ' ', '_');
					_classifierNames.push_back(key);

					std::string value = item["classifier_value"].convert<std::string>();
					_classifierSet.push_back(value);
				}

				std::string currentClassifierSet = boost::algorithm::join(_classifierSet, ",");
				bool foundClassifierSet = false;
				if (_classifierSetLU.count(currentClassifierSet) == 1){
					foundClassifierSet = true;
					_classfierSetRecordId = _classifierSetLU[currentClassifierSet];
				}
				else{
					_classfierSetRecordId = _curClassifierSetId++;
					_classifierSetLU.insert(std::make_pair(currentClassifierSet, _classfierSetRecordId));
				}
			}

			// --------------------------------------------------------------------------------------------
			void CBMAggregatorPoolSQLite::recordPoolsSet() {
				const auto timing = _landUnitData->timing();
				int curStep = timing->step();
				int curSubStep = timing->subStep();

				// date dimension record
				bool foundDate = false;
				int dateRecordId = -1;
				for (auto& date : _dateDimension) {
					bool fracMatch = FloatCmp::equalTo(std::get<6>(date), timing->fractionOfStep()) ? true : false;
					if (std::get<1>(date) == curStep
						&& std::get<2>(date) == curSubStep
						&& std::get<3>(date) == timing->curStartDate().year()
						&& std::get<4>(date) == timing->curStartDate().month()
						&& std::get<5>(date) == timing->curStartDate().day()
						&& fracMatch) {	// Not required now we have subStep found match of date dimension
						foundDate = true;
						dateRecordId = std::get<0>(date);
						break;
					}
				}
				if (!foundDate) {
					// No match for date dimension
					dateRecordId = _curDateId++;
					_dateDimension.push_back(std::make_tuple(dateRecordId, curStep, curSubStep, timing->curStartDate().year(), timing->curStartDate().month(), timing->curStartDate().day(), timing->fractionOfStep(), timing->stepLengthInYears()));
				}					
				
				// get current pool data
				int pools = this->_landUnitData->getPoolCount();
				std::vector<double> poolValues;
				poolValues.push_back(_landUnitArea); // add first area data

				auto opIt = _landUnitData->poolCollection();
				for (int poolIndex = 0; poolIndex < pools; poolIndex++){
					double poolValue = opIt->poolValue(poolIndex);
					poolValues.push_back(poolValue * _landUnitArea); //convert density to tonnes of cabon
				}

				//create pool key by timeStepID, classifierSetID
				PoolKey poolKey = std::make_tuple(dateRecordId, _classfierSetRecordId);
				Int64 poolRecordId = -1;
				auto it = _poolIdMapLU.find(poolKey);
				if (it != _poolIdMapLU.end()) {
					// found a fact match!
					poolRecordId = (*it).second;
					auto& factRecord = _poolValueVectorLU[poolRecordId-1];	

					//if found, aggregate the values
					for (int i = 0; i < factRecord.size(); i++){
						factRecord[i] += poolValues[i];					
					}
				}
				else {
					// No fact record found
					poolRecordId = _curPoolRecordId++;
					_poolIdMapLU.insert(std::pair<PoolKey, Int64>(poolKey, poolRecordId));
					_poolValueVectorLU.push_back(poolValues);
				}

#if 0
				try {
					Poco::Data::SQLite::Connector::registerConnector();
					// create a session
					Session session("SQLite", _dbName);

					//drop old if any, and create the ClassifierSet table
					createPoolsTable(session, _poolNames);
					string insertPoolsSql = buildPoolValueInsertSQL(_poolNames);
					
					Int64 recordIDKey = 1;
					PoolKey poolKey;				

					std::vector<double> poolValues;
					session.begin();

					auto& it = _poolIdMapLU.begin();
					while (it != _poolIdMapLU.end()) {
						poolKey = it->first;
						recordIDKey = it->second;						
						poolValues = _poolValueVectorLU[recordIDKey - 1];

						insertPoolRecord(session, insertPoolsSql, recordIDKey, dateRecordId, _classfierSetRecordId, poolValues);

						it++;
					}

					session.commit();
					Poco::Data::SQLite::Connector::unregisterConnector();
				}
				catch (Poco::Data::SQLite::InvalidSQLStatementException& exc) {
					std::cerr << exc.displayText() << std::endl;
					std::cerr << std::endl;
				}
				catch (...) {
				}
#endif
			}

			// --------------------------------------------------------------------------------------------
			void CBMAggregatorPoolSQLite::recordAfterSpinningupPoolsSet() {				
				// get current pool data			
				updateLandUnitInformation();

				int pools = this->_landUnitData->getPoolCount();
				std::vector<double> poolValues;
				poolValues.push_back(_landUnitArea); // add first area data

				auto opIt = _landUnitData->poolCollection();
				for (int poolIndex = 0; poolIndex < pools; poolIndex++){
					double poolValue = opIt->poolValue(poolIndex);
					poolValues.push_back(poolValue); //record carbon density after spining up
				}

				//create pool key by timeStepID(0), landUnitId
				PoolKey poolKey = std::make_tuple(0, _luid);
				//PoolKey poolKey = std::make_tuple(0, _classfierSetRecordId);
				Int64 poolRecordId = -1;
				auto it = _poolIdMapLU.find(poolKey);
				if (it != _poolIdMapLU.end()) {
					// found a fact match!
					poolRecordId = (*it).second;
					auto& factRecord = _poolValueVectorLU[poolRecordId - 1];

					//if found, aggregate the values
					for (int i = 0; i < factRecord.size(); i++){
						factRecord[i] += poolValues[i];
					}
				}
				else {
					// No fact record found
					poolRecordId = _curPoolRecordId++;
					_poolIdMapLU.insert(std::pair<PoolKey, Int64>(poolKey, poolRecordId));
					_poolValueVectorLU.push_back(poolValues);
				}
			}

			// --------------------------------------------------------------------------------------------
			void CBMAggregatorPoolSQLite::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
				int pools = this->_landUnitData->getPoolCount();
				this->_poolNames.clear(); 

				auto opIt = _landUnitData->poolCollection();
				for (int poolIndex = 0; poolIndex < pools; poolIndex++){
					double poolValue = opIt->poolValue(poolIndex);		
					auto poolName = opIt->pool(poolIndex)->name();
					std::replace(poolName.begin(), poolName.end(), ' ', '_');
					this->_poolNames.push_back(poolName);
				}					

				// Initialize DateId, PoolRecordId and ClassifierSetId, start from 1
				_curDateId = 1;
				_curPoolRecordId = 1;
				_curClassifierSetId= 1;							
			}

			// --------------------------------------------------------------------------------------------

			void CBMAggregatorPoolSQLite::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {			
				// land unit is changed at this moment
				// get the land unit area
				updateLandUnitInformation();
				//std::cout << "ClassiferSet ID: " << _classfierSetRecordId << std::endl;
			}

			// --------------------------------------------------------------------------------------------

			void CBMAggregatorPoolSQLite::onTimingShutdown(const flint::TimingShutdownNotification::Ptr& n) {	
				// Has the Land Unit be successful? should we aggregate the results into the Local Domain
				// or ignore them?
#if 0				
				for (auto factLU : _factVectorLU) {
					int factId = int(std::get<0>(factLU)+MathEx::k0Plus);
					FactKey factKey = std::get<1>(factLU);
					Int64 itemCount = std::get<2>(factLU);
					double areaSum = std::get<3>(factLU);
					double fluxValueSum = std::get<4>(factLU);

					Int64 factRecordId = -1;
					auto it = _factIdMapLD.find(factKey);
					if (it != _factIdMapLD.end()) {
						// found a fact match!
						factRecordId = (*it).second;
						auto& factRecord = _factVectorLD[factRecordId];
						std::get<2>(factRecord) += itemCount;
						std::get<3>(factRecord) += areaSum;
						std::get<4>(factRecord) += fluxValueSum;
					}
					else {
						// No fact record found
						factRecordId = _curFactIdLD++;
						_factIdMapLD.insert(std::pair<FactKey, Int64>(factKey, factRecordId));
						_factVectorLD.push_back(std::make_tuple(factRecordId, factKey, itemCount, _landUnitArea, fluxValueSum));
					}
				}
#endif		
			}

			// --------------------------------------------------------------------------------------------

			void CBMAggregatorPoolSQLite::onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& /*n*/) {
				DateTime startTime = DateTime::now();

				// Output to SQLITE - using POCO SQLITE
				try {
					Poco::Data::SQLite::Connector::registerConnector();

					DateTime startSessionSetupTime = DateTime::now();
					// create a session
					Session session("SQLite", _dbName);							

					//drop old if any, and create the Pools table
					createPoolsTable(session, _poolNames);		

					//drop old if any, and create the ClassifierSet table
					createClassifierSetTable(session, _classifierNames);

					DateTime endSessionSetupTime = DateTime::now();

					//session for classifier set records
					DateTime startClassiferSetInsertTime = DateTime::now();
					string insertClassifierSetSql = buildClassifierInsertSQL(_classifierNames);		

					Int64 recordIDKey = 1;
					string setValueStr;
					std::vector<string> classifierValues;

					session.begin();

					auto& itC = _classifierSetLU.begin();
					while (itC != _classifierSetLU.end()) {
						setValueStr = itC->first;
						recordIDKey = itC->second;
						classifierValues = split(setValueStr, ',');

						insertClassifierSetRecords(session, insertClassifierSetSql, recordIDKey, classifierValues, _classifierNames.size());

						itC++;
					}

					session.commit();

					DateTime endClassiferSetInsertTime = DateTime::now();					
					std::cout << "ClassifierSet records : " << _classifierSetLU.size() << std::endl;

					string insertPoolsSql = buildPoolValueInsertSQL(_poolNames);

					
					PoolKey poolKey;
					std::vector<double> poolValues;
					DateTime startPoolInsertTime = DateTime::now();

					session.begin();

					auto& itPool = _poolIdMapLU.begin();
					while (itPool != _poolIdMapLU.end()) {
						poolKey = itPool->first;
						recordIDKey = itPool->second;
						poolValues = _poolValueVectorLU[recordIDKey - 1];

						insertPoolRecord(session, insertPoolsSql, recordIDKey, std::get<0>(poolKey), std::get<1>(poolKey), poolValues);

						itPool++;
					}

					session.commit();

					DateTime endPoolInsertTime = DateTime::now();
					std::cout << "Pools records: " << _poolIdMapLU.size() << std::endl;

					Poco::Data::SQLite::Connector::unregisterConnector();					
				}
				catch (Poco::AssertionViolationException& exc) {
					std::cerr << exc.displayText() << std::endl;
				}
				catch (Poco::Data::SQLite::InvalidSQLStatementException& exc) {
					std::cerr << exc.displayText() << std::endl;
					std::cerr << std::endl;
				}
				catch (...) {
				}
			}

			// --------------------------------------------------------------------------------------------
			//void CBMAggregatorPoolSQLite::onOutputStep(const flint::PostNotificationNotification::Ptr& n) {
			void CBMAggregatorPoolSQLite::onOutputStep(const flint::OutputStepNotification::Ptr& n) {			
				recordPoolsSet();				
			}

			void CBMAggregatorPoolSQLite::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n){
				recordAfterSpinningupPoolsSet();
			}
		}
	}
} // namespace moja::modules::cbm