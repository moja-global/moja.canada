#include "moja/modules/cbm/CBMAggregatorSQLiteWriter.h"
#include "moja/flint/landunitcontroller.h"

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

    void CBMAggregatorSQLiteWriter::configure(const DynamicObject& config) {
        _dbName = config["databasename"].convert<std::string>();
    }

    void CBMAggregatorSQLiteWriter::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::SystemShutdown, &CBMAggregatorSQLiteWriter::onSystemShutdown, *this);
	}

    void CBMAggregatorSQLiteWriter::onSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        // Output to SQLITE the fact and dimension database - using POCO SQLITE.
        try {
            MOJA_LOG_INFO << "Loading results." << std::endl;

            SQLite::Connector::registerConnector();
            Session session("SQLite", _dbName);

            session << "DROP TABLE IF EXISTS DateDimension", now;
            session << "DROP TABLE IF EXISTS ModuleInfoDimension", now;
            session << "DROP TABLE IF EXISTS PoolDimension", now;
            session << "DROP TABLE IF EXISTS LocationDimension", now;
            session << "DROP TABLE IF EXISTS ClassifierSetDimension", now;
            session << "DROP TABLE IF EXISTS Pools", now;
            session << "DROP TABLE IF EXISTS LandClassDimension", now;
            session << "DROP TABLE IF EXISTS Fluxes", now;

            session << "CREATE TABLE DateDimension (id UNSIGNED BIG INT PRIMARY KEY, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)", now;
            session << "CREATE TABLE ModuleInfoDimension (id UNSIGNED BIG INT PRIMARY KEY, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), disturbanceTypeName VARCHAR(255), disturbanceType INTEGER)", now;
            session << "CREATE TABLE PoolDimension (id UNSIGNED BIG INT PRIMARY KEY, poolName VARCHAR(255))", now;
            session << "CREATE TABLE LocationDimension (id UNSIGNED BIG INT PRIMARY KEY, classifierSetDimId UNSIGNED BIG INT, dateDimId UNSIGNED BIG INT, landClassDimId UNSIGNED BIG INT, area FLOAT)", now;
            session << (boost::format("CREATE TABLE ClassifierSetDimension (id UNSIGNED BIG INT PRIMARY KEY, %1% VARCHAR)") % boost::join(*_classifierNames, " VARCHAR, ")).str(), now;
            session << "CREATE TABLE LandClassDimension (id UNSIGNED BIG INT PRIMARY KEY, name VARCHAR(255))", now;
            session << "CREATE TABLE Pools (id UNSIGNED BIG INT, locationDimId UNSIGNED BIG INT, poolId UNSIGNED BIG INT, poolValue FLOAT)", now;
            session << "CREATE TABLE Fluxes (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, moduleInfoDimId UNSIGNED BIG INT, poolSrcDimId UNSIGNED BIG INT, poolDstDimId UNSIGNED BIG INT, fluxValue FLOAT)", now;

            std::vector<std::string> csetPlaceholders;
            auto classifierCount = _classifierNames->size();
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
            session << "INSERT INTO LandClassDimension VALUES(?, ?)",
                bind(_landClassDimension->getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO PoolDimension VALUES(?, ?)",
                bind(_poolInfoDimension->getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO DateDimension VALUES(?, ?, ?, ?, ?, ?, ?)",
                bind(_dateDimension->getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO ModuleInfoDimension VALUES(?, ?, ?, ?, ?, ?, ?, ?)",
                bind(_moduleInfoDimension->getPersistableCollection()), now;
            session.commit();
            
            session.begin();
            session << "INSERT INTO LocationDimension VALUES(?, ?, ?, ?, ?)",
                bind(_locationDimension->getPersistableCollection()), now;
            session.commit();

            session.begin();
            session << "INSERT INTO Fluxes VALUES(?, ?, ?, ?, ?, ?)",
                bind(_fluxDimension->getPersistableCollection()), now;
            session.commit();
            
            session.begin();
            session << "INSERT INTO Pools VALUES(?, ?, ?, ?)",
                       bind(_poolDimension->getPersistableCollection()), now;
            session.commit();
            
            Poco::Data::SQLite::Connector::unregisterConnector();
            MOJA_LOG_INFO << "SQLite insert complete." << std::endl;
        }
        catch (Poco::AssertionViolationException& exc) {
            MOJA_LOG_FATAL << exc.displayText() << std::endl;
        }
        catch (Poco::Data::SQLite::InvalidSQLStatementException& exc) {
            MOJA_LOG_FATAL << exc.displayText() << std::endl;
        }
		catch (Poco::Data::SQLite::ConstraintViolationException& exc) {
            MOJA_LOG_FATAL << exc.displayText() << std::endl;
		}
        catch (Poco::Data::BindingException& exc) {
            MOJA_LOG_FATAL << exc.displayText() << std::endl;
        }
		catch (const std::exception& e) {
            MOJA_LOG_FATAL << e.what() << std::endl;
		}
		catch (...) {
            MOJA_LOG_FATAL << "Unknown exception." << std::endl;
		}
    }

}}} // namespace moja::modules::cbm
