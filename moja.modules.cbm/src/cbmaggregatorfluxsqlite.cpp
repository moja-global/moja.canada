#include "moja/modules/cbm/cbmaggregatorfluxsqlite.h"
#include "moja/flint/landunitcontroller.h"
#include "moja/observer.h"
#include <moja/mathex.h>

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

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

using namespace Poco::Data;
using Poco::format;
using Poco::NotFoundException;

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

#include <iomanip>

using namespace std;

namespace moja {
namespace modules {
namespace cbm {

    void CBMAggregatorFluxSQLite::configure(const DynamicObject& config) {
        _dbName = config["databasename"].convert<std::string>();
    }

    void CBMAggregatorFluxSQLite::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(
            std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(
                *this, &IModule::onLocalDomainInit));

        notificationCenter.addObserver(
            std::make_shared<Observer<IModule, flint::LocalDomainShutdownNotification>>(
                *this, &IModule::onLocalDomainShutdown));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(*this, &IModule::onTimingInit));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingShutdownNotification>>(*this, &IModule::onTimingShutdown));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::PostNotificationNotification>>(*this, &IModule::onPostNotification));
    }

    void CBMAggregatorFluxSQLite::RecordFluxSet() {
        const auto timing = _landUnitData->timing();
        int curStep = timing->step();
        int curSubStep = timing->subStep();

        // If Flux set is empty return immediately
        if (_landUnitData->getOperationLastAppliedIterator().empty())
            return;

        // These Dimensions are constant for the whole flux set so find them first

        // ***** Find the date dimension record
        bool foundDate = false;
        int dateRecordId = -1;
        for (auto& date : _dateDimension) {
            bool fracMatch = FloatCmp::equalTo(date.get<6>(), timing->fractionOfStep()) ? true : false;
            if (date.get<1>() == curStep
                && date.get<2>() == curSubStep
                && date.get<3>() == timing->curStartDate().year()
                && date.get<4>() == timing->curStartDate().month()
                && date.get<5>() == timing->curStartDate().day()
                && fracMatch) {	// Not required now we have subStep
                // found match of date dimension
                foundDate = true;
                dateRecordId = date.get<0>();
                break;
            }
        }
        if (!foundDate) {
            // No match for date dimension
            dateRecordId = _curDateId++;
            _dateDimension.push_back(DateRecord{
                dateRecordId, curStep, curSubStep, timing->curStartDate().year(),
                timing->curStartDate().month(), timing->curStartDate().day(),
                timing->fractionOfStep(), timing->stepLengthInYears()
            });
        }

        // ***** Find the location dimension record
        bool foundLocation = false;
        int locationRecordId = -1;
        for (auto& location : _locationDimension) {
            if (location.get<1>() == _localDomainId
                && location.get<2>() == 0 /*_landUnitId*/
                && location.get<3>() == _countyId) {
                // found match of location dimension
                foundLocation = true;
                locationRecordId = location.get<0>();
                break;
            }
        }
        if (!foundLocation) {
            // No match for location dimension
            locationRecordId = _curLocationId++;
            _locationDimension.push_back(LocationRecord{
                locationRecordId, _localDomainId, 0 /*_landUnitId*/, _countyId
            });
        }

        // Dimensions from here change per flux - so need to start loop the current fluxes

		//for (auto opIt = _landUnitData->getOperationLastAppliedIterator(); opIt->operator bool(); opIt->operator++()) {
		for (auto operationResult : _landUnitData->getOperationLastAppliedIterator()) {
            //const auto operationResult = opIt->value();
            const auto metaData = operationResult->metaData();
            //auto itPtr = operationResult->getIterator();
            //auto it = itPtr.get();
            //for (; (*it); ++(*it)) {
			for (auto it : operationResult->operationResultFluxCollection()) {
                auto srcIx = it->source();
                auto dstIx = it->sink();
                if (srcIx == dstIx)
                    continue;// don't process diagonal - flux to & from same pool is ignored
                auto fluxValue = it->value();
                auto srcPool = _landUnitData->getPool(srcIx);
                auto dstPool = _landUnitData->getPool(dstIx);

                // ***** Find the moduleInfo dimension record
                bool foundModuleInfo = false;
                int moduleInfoRecordId = -1;
                for (auto& moduleInfo : _moduleInfoDimension) {
                    if (   moduleInfo.get<1>() == metaData.libraryType
                        && moduleInfo.get<2>() == metaData.libraryInfoId
                        && moduleInfo.get<3>() == metaData.moduleType
                        && moduleInfo.get<4>() == metaData.moduleId
                        && moduleInfo.get<5>() == metaData.moduleName
                        && moduleInfo.get<6>() == metaData.disturbanceType
                        ) {
                        // found match of moduleInfo dimension
                        foundModuleInfo = true;
                        moduleInfoRecordId = moduleInfo.get<0>();
                        break;
                    }
                }
                if (!foundModuleInfo) {
                    // No match for location dimension
                    moduleInfoRecordId = _curModuleInfoId++;
                    _moduleInfoDimension.push_back(ModuleInfoRecord{
                        moduleInfoRecordId, metaData.libraryType, metaData.libraryInfoId,
                        metaData.moduleType, metaData.moduleId, metaData.moduleName,
                        metaData.disturbanceType
                    });
                }

                // ***** Find the SRC pool dimension record
                bool foundPool = false;
                int poolSrcRecordId = -1;
                for (auto& pool : _poolDimension) {
                    if (pool.get<1>() == srcIx) {
                        // found match of pool dimension
                        foundPool = true;
                        poolSrcRecordId = pool.get<0>();
                        break;
                    }
                }
                if (!foundPool) {
                    // No match for location dimension
                    poolSrcRecordId = _curPoolId++;
                    _poolDimension.push_back(PoolRecord{
                        poolSrcRecordId, srcIx, srcPool->name()
                    });
                }

                // ***** Find the DST pool dimension record
                foundPool = false;
                int poolDstRecordId = -1;
                for (auto& pool : _poolDimension) {
                    if (pool.get<1>() == dstIx) {
                        // found match of pool dimension
                        foundPool = true;
                        poolDstRecordId = pool.get<0>();
                        break;
                    }
                }
                if (!foundPool) {
                    // No match for location dimension
                    poolDstRecordId = _curPoolId++;
                    _poolDimension.push_back(PoolRecord{
                        poolDstRecordId, dstIx, dstPool->name()
                    });
                }

                // now have the required dimensions - look for the fact record
                FactKey factKey = std::make_tuple(
                    dateRecordId, locationRecordId, moduleInfoRecordId,
                    0/*forestId*/, poolSrcRecordId, poolDstRecordId);

                Int64 factRecordId = -1;
                auto it = _factIdMapLU.find(factKey);
                if (it != _factIdMapLU.end()) {
                    // found a fact match!
                    factRecordId = (*it).second;
                    auto& factRecord = _factVectorLU[factRecordId];
                    factRecord.get<2>()++; // itemCount
                    factRecord.get<3>() += _landUnitArea; // areaSum
                    factRecord.get<4>() += fluxValue; // flux value
                }
                else {
                    // No fact record found
                    factRecordId = _curFactIdLU++;
                    _factIdMapLU.insert(std::pair<FactKey, Int64>(factKey, factRecordId));
                    _factVectorLU.push_back(FactRecord{
                        factRecordId, factKey, 1, _landUnitArea, fluxValue
                    });
                }
            }
        }
    }

    void CBMAggregatorFluxSQLite::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& /*n*/) {
        // Initialize the fatc and dimension data
        _curDateId = 1;
        _curLocationId = 1;
        _curFactIdLD = 0;
        _curFactIdLU = 0;
        _curModuleInfoId = 0;
        _curPoolId = 0;

        // We could generate dimension tables here, ones with known data. This way id's across localdomains 
        // would be consistent
    }

    void CBMAggregatorFluxSQLite::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {
        // Variables we want to use, these won't change during a land unit simulation (timing is run for each LU)
        _localDomainId = 1; //  _landUnitData->getVariable("LocalDomainId")->value();
        _countyId = 1;		//  _landUnitData->getVariable("countyId")->value();
        _landUnitArea = 1;	//  _landUnitData->getVariable("LandUnitArea")->value();
        _forestType = 1;	//  _landUnitData->getVariable("forests")->value();
        _lossYear = 1;		//  _landUnitData->getVariable("lossyear")->value();

        RecordFluxSet();	// in case init had some
    }

    void CBMAggregatorFluxSQLite::onTimingShutdown(const flint::TimingShutdownNotification::Ptr& n) {
        // Has the Land Unit be successful? should we aggregate the results into the Local Domain
        // or ignore them?

        for (auto factLU : _factVectorLU) {
            int factId = int(factLU.get<0>() + MathEx::k0Plus);
            FactKey factKey = factLU.get<1>();
            Int64 itemCount = factLU.get<2>();
            double areaSum = factLU.get<3>();
            double fluxValueSum = factLU.get<4>();

            Int64 factRecordId = -1;
            auto it = _factIdMapLD.find(factKey);
            if (it != _factIdMapLD.end()) {
                // found a fact match!
                factRecordId = (*it).second;
                auto& factRecord = _factVectorLD[factRecordId];
                factRecord.get<2>() += itemCount;
                factRecord.get<3>() += areaSum;
                factRecord.get<4>() += fluxValueSum;
            }
            else {
                // No fact record found
                factRecordId = _curFactIdLD++;
                _factIdMapLD.insert(std::pair<FactKey, Int64>(factKey, factRecordId));
                _factVectorLD.push_back(FactRecord{
                    factRecordId, factKey, itemCount, _landUnitArea, fluxValueSum
                });
            }
        }

        _factVectorLU.clear();
        _factIdMapLU.clear();
        _curFactIdLU = 0;
    }

    void CBMAggregatorFluxSQLite::onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& /*n*/) {
        DateTime startTime = DateTime::now();

        std::cout << "_factIdMapLD has " << _factIdMapLD.bucket_count() << " buckets" << std::endl;

        // Output to SQLITE the fact and dimension database - using POCO SQLITE
        try {
            Poco::Data::SQLite::Connector::registerConnector();
            DateTime startSessionSetupTime = DateTime::now();
            Session session("SQLite", _dbName);

            session << "DROP TABLE IF EXISTS DateDimension", now;
            session << "DROP TABLE IF EXISTS LocationDimension", now;
            session << "DROP TABLE IF EXISTS ModuleInfoDimension", now;
            session << "DROP TABLE IF EXISTS PoolDimension", now;
            session << "DROP TABLE IF EXISTS Facts", now;

            session << "CREATE TABLE DateDimension (id UNSIGNED BIG INT, step INTEGER, substep INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)", now;
            session << "CREATE TABLE LocationDimension (id UNSIGNED BIG INT, localDomainId INTEGER, landUnitId INTEGER, countyId INTEGER)", now;
            session << "CREATE TABLE ModuleInfoDimension (id UNSIGNED BIG INT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), disturbanceType INTEGER)", now;
            session << "CREATE TABLE PoolDimension (id UNSIGNED BIG INT, poolId INTEGER, poolName VARCHAR(255))", now;
            session << "CREATE TABLE Facts (id UNSIGNED BIG INT, dateDimId UNSIGNED BIG INT, locationDimId UNSIGNED BIG INT, moduleInfoDimId UNSIGNED BIG INT, forestId INT, poolSrcDimId UNSIGNED BIG INT, poolDstDimId UNSIGNED BIG INT, itemCount INTEGER, areaSum FLOAT, fluxValue FLOAT)", now;

            session << "INSERT INTO PoolDimension VALUES(?, ?, ?)", use(_poolDimension), now;
            session << "INSERT INTO DateDimension VALUES(?, ?, ?, ?, ?, ?, ?, ?)", use(_dateDimension), now;
            session << "INSERT INTO LocationDimension VALUES(?, ?, ?, ?)", use(_locationDimension), now;
            session << "INSERT INTO ModuleInfoDimension VALUES(?, ?, ?, ?, ?, ?, ?)", use(_moduleInfoDimension), now;
                    
            session.begin();
            FactRecord fact;
            for (auto& fact : _factVectorLD) {
                auto key = fact.get<1>();
                Statement insert(session);
                insert << "INSERT INTO Facts VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    useRef(fact.get<0>()),	    // 1
                    useRef(std::get<0>(key)),	// 2
                    useRef(std::get<1>(key)),	// 3
                    useRef(std::get<2>(key)),	// 4
                    useRef(std::get<3>(key)),	// 5
                    useRef(std::get<4>(key)),	// 6
                    useRef(std::get<5>(key)),	// 7
                    useRef(fact.get<2>()),	    // 8
                    useRef(fact.get<3>()),	    // 9
                    useRef(fact.get<4>());	    // 10
                insert.execute();
            }
            session.commit();

            Poco::Data::SQLite::Connector::unregisterConnector();

            std::cout << "SQLite insert:" << std::endl;
            std::cout << "Fact records     : " << _factVectorLD.size() << std::endl;
            std::cout << std::endl;
        }
        catch (Poco::AssertionViolationException& exc) {
            std::cerr << exc.displayText() << std::endl;
        }
        catch (Poco::Data::SQLite::InvalidSQLStatementException&) {
            std::cerr << std::endl;
        }
        catch (...) {
        }
    }

    void CBMAggregatorFluxSQLite::onPostNotification(const flint::PostNotificationNotification::Ptr&) {
        RecordFluxSet();
        _landUnitData->clearLastAppliedOperationResults();
    }

}}} // namespace moja::modules::cbm
