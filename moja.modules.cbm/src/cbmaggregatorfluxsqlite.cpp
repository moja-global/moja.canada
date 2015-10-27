#include "moja/modules/cbm/cbmaggregatorfluxsqlite.h"
#include "moja/flint/landunitcontroller.h"
#include "moja/observer.h"

#include <Poco/String.h>
#include <Poco/Format.h>
#include <Poco/Data/StatementImpl.h>
#include <Poco/Exception.h>
#include <Poco/Logger.h>
#include <Poco/Data/SessionPool.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/SQLite/SQLiteException.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <iomanip>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

using namespace Poco::Data;
using Poco::format;
using Poco::NotFoundException;

namespace moja {
namespace modules {
namespace cbm {

    void CBMAggregatorFluxSQLite::configure(const DynamicObject& config) {
        _dbName = config["databasename"].convert<std::string>();
    }

    void CBMAggregatorFluxSQLite::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainShutdownNotification>>(*this, &IModule::onLocalDomainShutdown));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(*this, &IModule::onTimingInit));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingShutdownNotification>>(*this, &IModule::onTimingShutdown));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::PostNotificationNotification>>(*this, &IModule::onPostNotification));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::OutputStepNotification>>(*this, &IModule::onOutputStep));
    }

    void CBMAggregatorFluxSQLite::recordFluxSet() {
        const auto timing = _landUnitData->timing();
        int curStep = timing->step();
        int curSubStep = timing->subStep();

        // If Flux set is empty return immediately
        if (_landUnitData->getOperationLastAppliedIterator().empty())
            return;
            }

        // Find the date dimension record.
        auto dateRecord = std::make_shared<DateRecord>(
            curStep, curSubStep, timing->curStartDate().year(),
                timing->curStartDate().month(), timing->curStartDate().day(),
            timing->fractionOfStep(), timing->stepLengthInYears());

        auto storedDateRecord = _dateDimension->accumulate(dateRecord);
        auto dateRecordId = storedDateRecord->getId();

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

                // Find the module info dimension record.
                auto moduleInfoRecord = std::make_shared<ModuleInfoRecord>(
                    metaData.libraryType, metaData.libraryInfoId,
                        metaData.moduleType, metaData.moduleId, metaData.moduleName,
                    metaData.disturbanceType);

                auto storedModuleInfoRecord = _moduleInfoDimension.accumulate(moduleInfoRecord);
                auto moduleInfoRecordId = storedModuleInfoRecord->getId();

                // Find the source pool dimension record.
                auto srcPoolRecord = std::make_shared<PoolInfoRecord>(srcPool->name());
                auto storedSrcPoolRecord = _poolInfoDimension->accumulate(srcPoolRecord);
                auto poolSrcRecordId = storedSrcPoolRecord->getId();

                // Find the destination pool dimension record.
                auto dstPoolRecord = std::make_shared<PoolInfoRecord>(dstPool->name());
                auto storedDstPoolRecord = _poolInfoDimension->accumulate(dstPoolRecord);
                auto poolDstRecordId = storedDstPoolRecord->getId();

                // Now have the required dimensions - look for the flux record.
                auto fluxRecord = std::make_shared<FluxRecord>(
                    dateRecordId, _locationId, moduleInfoRecordId,
                    poolSrcRecordId, poolDstRecordId, fluxValue);

                _fluxDimension.accumulate(fluxRecord);
        }
    }
    }

    void CBMAggregatorFluxSQLite::onTimingInit(const flint::TimingInitNotification::Ptr& /*n*/) {
        // Classifier set information.
        const auto& landUnitClassifierSet = _landUnitData->getVariable("classifier_set")->value()
            .extract<std::vector<DynamicObject>>();

        std::vector<std::string> classifierSet;
        bool firstPass = _classifierNames.empty();
        for (const auto& item : landUnitClassifierSet) {
            if (firstPass) {
                auto key = item["classifier_name"].convert<std::string>();
                std::replace(key.begin(), key.end(), '.', ' ');
                std::replace(key.begin(), key.end(), ' ', '_');
                _classifierNames.push_back(key);
    }

            auto value = item["classifier_value"].convert<std::string>();
            classifierSet.push_back(value);
        }

        auto cSetRecord = std::make_shared<ClassifierSetRecord>(classifierSet);
        auto storedCSetRecord = _classifierSetDimension->accumulate(cSetRecord);
        auto classifierSetRecordId = storedCSetRecord->getId();

        auto landUnitArea = _landUnitData->getVariable("LandUnitArea")->value();
        auto locationRecord = std::make_shared<LocationRecord>(classifierSetRecordId, landUnitArea);
        auto storedLocationRecord = _locationDimension->accumulate(locationRecord);
        _locationId = storedLocationRecord->getId();
    }

    void CBMAggregatorFluxSQLite::onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& /*n*/) {
        // Output to SQLITE the fact and dimension database - using POCO SQLITE.
        try {
            Poco::Data::SQLite::Connector::registerConnector();
            Session session("SQLite", _dbName);

            session << "DROP TABLE IF EXISTS DateDimension", now;
            session << "DROP TABLE IF EXISTS ModuleInfoDimension", now;
            session << "DROP TABLE IF EXISTS PoolDimension", now;
            session << "DROP TABLE IF EXISTS Fluxes", now;
            session << "DROP TABLE IF EXISTS ClassifierSetDimension", now;
            session << "DROP TABLE IF EXISTS LocationDimension", now;

            session << "CREATE TABLE DateDimension (id UNSIGNED BIG INT, step INTEGER, substep INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)", now;
            session << "CREATE TABLE ModuleInfoDimension (id UNSIGNED BIG INT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), disturbanceType INTEGER)", now;
            session << "CREATE TABLE PoolDimension (id UNSIGNED BIG INT, poolName VARCHAR(255))", now;
            session << "CREATE TABLE Fluxes (id UNSIGNED BIG INT, dateDimId UNSIGNED BIG INT, locationDimId UNSIGNED BIG INT, moduleInfoDimId UNSIGNED BIG INT, poolSrcDimId UNSIGNED BIG INT, poolDstDimId UNSIGNED BIG INT, fluxValue FLOAT)", now;
            session << "CREATE TABLE LocationDimension (id UNSIGNED BIG INT, classifierSetDimId UNSIGNED BIG INT, area FLOAT)", now;
            session << (boost::format("CREATE TABLE ClassifierSetDimension (id UNSIGNED BIG INT, %1% VARCHAR)") % boost::join(_classifierNames, " VARCHAR, ")).str(), now;

            std::vector<std::string> csetPlaceholders;
            auto classifierCount = _classifierNames.size();
            for (auto i = 0; i < classifierCount; i++) {
                csetPlaceholders.push_back("?");
            }

            auto csetSql = (boost::format("INSERT INTO ClassifierSetDimension VALUES(?, %1%)")
                % boost::join(csetPlaceholders, ", ")).str();
                    
            session.begin();
            for (auto cset : _classifierSetDimension->getPersistableCollection()) {
                Statement insert(session);
                insert << csetSql, use(cset.get<0>());
                auto values = cset.get<1>();
                for (int i = 0; i < classifierCount; i++) {
                    insert, use(values[i]);
                }

                insert.execute();
            }
            session.commit();

            session.begin();
            session << "INSERT INTO PoolDimension VALUES(?, ?)",
                       use(_poolInfoDimension->getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO DateDimension VALUES(?, ?, ?, ?, ?, ?, ?, ?)",
                       use(_dateDimension->getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO ModuleInfoDimension VALUES(?, ?, ?, ?, ?, ?, ?)",
                       use(_moduleInfoDimension.getPersistableCollection()), now;
            session.commit();
            
            session.begin();
            session << "INSERT INTO Fluxes VALUES(?, ?, ?, ?, ?, ?, ?)",
                       use(_fluxDimension.getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO LocationDimension VALUES(?, ?, ?)",
                       use(_locationDimension->getPersistableCollection()), now;
            session.commit();

            Poco::Data::SQLite::Connector::unregisterConnector();
            std::cout << "SQLite insert complete" << std::endl;
        }
        catch (Poco::AssertionViolationException& exc) {
            std::cerr << exc.displayText() << std::endl;
        }
        catch (Poco::Data::SQLite::InvalidSQLStatementException& exc) {
            std::cerr << exc.displayText() << std::endl;
        }
        catch (...) {
        }
    }

    void CBMAggregatorFluxSQLite::onOutputStep(const flint::OutputStepNotification::Ptr&) {
        recordFluxSet();
        _landUnitData->clearLastAppliedOperationResults();
    }

}}} // namespace moja::modules::cbm
