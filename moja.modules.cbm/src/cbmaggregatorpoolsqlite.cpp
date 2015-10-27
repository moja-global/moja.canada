#include "moja/logging.h"
#include "moja/modules/cbm/cbmaggregatorpoolsqlite.h"
#include "moja/flint/matrix.h"
#include "moja/flint/landunitcontroller.h"
#include "moja/flint/operationmatrix.h"
#include "moja/observer.h"
#include "moja/mathex.h"

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

    void CBMAggregatorPoolSQLite::configure(const DynamicObject& config) {
        _dbName = config["databasename"].convert<std::string>();
    }			

    void CBMAggregatorPoolSQLite::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainShutdownNotification>>(*this, &IModule::onLocalDomainShutdown));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::OutputStepNotification>>(*this, &IModule::onOutputStep));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::PreTimingSequenceNotification>>(*this, &IModule::onPreTimingSequence));
    }

    void CBMAggregatorPoolSQLite::recordPoolsSet(bool isSpinup) {
        const auto timing = _landUnitData->timing();
        int curStep = timing->step();
        int curSubStep = timing->subStep();

        Int64 dateRecordId = -1;
        if (!isSpinup) {
            // Find the date dimension record.
            auto dateRecord = std::make_shared<DateRecord>(
                curStep, curSubStep, timing->curStartDate().year(),
                timing->curStartDate().month(), timing->curStartDate().day(),
                timing->fractionOfStep(), timing->stepLengthInYears());

            auto storedDateRecord = _dateDimension->accumulate(dateRecord);
            dateRecordId = storedDateRecord->getId();
        }
                
        // Get current pool data.
        auto pools = _landUnitData->poolCollection();
        for (const moja::flint::IPool& pool : *pools) {
            auto poolInfoRecord = std::make_shared<PoolInfoRecord>(pool.name());
            auto storedPoolInfoRecord = _poolInfoDimension->accumulate(poolInfoRecord);
            auto poolInfoRecordId = storedPoolInfoRecord->getId();

            auto poolRecord = std::make_shared<PoolRecord>(
                dateRecordId, _locationId, poolInfoRecordId, pool.value());
            
            _poolDimension.accumulate(poolRecord);
        }
    }

    void CBMAggregatorPoolSQLite::onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& /*n*/) {
        // Output to SQLITE - using POCO SQLITE
        try {
            Poco::Data::SQLite::Connector::registerConnector();
            Session session("SQLite", _dbName);

            session << "DROP TABLE IF EXISTS Pools", now;
            session << "CREATE TABLE Pools (id UNSIGNED BIG INT, dateDimId UNSIGNED BIG INT, locationDimId UNSIGNED BIG INT, poolId UNSIGNED BIG INT, poolValue FLOAT)", now;

            session.begin();
            session << "INSERT INTO Pools VALUES(?, ?, ?, ?, ?)",
                       use(_poolDimension.getPersistableCollection()), now;
            session.commit();

            Poco::Data::SQLite::Connector::unregisterConnector();
            std::cout << "SQLite insert complete" << std::endl;
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

    void CBMAggregatorPoolSQLite::onOutputStep(const flint::OutputStepNotification::Ptr& n) {			
        recordPoolsSet(false);				
    }

    void CBMAggregatorPoolSQLite::onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n) {
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

        auto locationRecord = std::make_shared<LocationRecord>(classifierSetRecordId, 0);
        auto storedLocationRecord = _locationDimension->accumulate(locationRecord);
        _locationId = storedLocationRecord->getId();

        // Record post-spinup pool values.
        recordPoolsSet(true);
    }

}}} // namespace moja::modules::cbm
