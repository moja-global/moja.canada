#include "moja/modules/cbm/cbmaggregatorsqlitewriter.h"
#include "moja/flint/landunitcontroller.h"
#include "moja/flint/recordaccumulatorwithmutex.h"

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
#include <initializer_list>

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
        MOJA_LOG_INFO << "Loading results." << std::endl;

        SQLite::Connector::registerConnector();
        Session session("SQLite", _dbName);

		std::vector<std::string> ddl{
			"DROP TABLE IF EXISTS Pools",
			"DROP TABLE IF EXISTS Fluxes",
			"DROP TABLE IF EXISTS DisturbanceDimension",
			"DROP TABLE IF EXISTS LocationDimension",
			"DROP TABLE IF EXISTS ModuleInfoDimension",
			"DROP TABLE IF EXISTS LandClassDimension",
			"DROP TABLE IF EXISTS PoolDimension",
			"DROP TABLE IF EXISTS DateDimension",
			"DROP TABLE IF EXISTS ClassifierSetDimension",
			(boost::format("CREATE TABLE ClassifierSetDimension (id UNSIGNED BIG INT PRIMARY KEY, %1% VARCHAR)") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE TABLE DateDimension (id UNSIGNED BIG INT PRIMARY KEY, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)",
			"CREATE TABLE PoolDimension (id UNSIGNED BIG INT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE TABLE LandClassDimension (id UNSIGNED BIG INT PRIMARY KEY, name VARCHAR(255))",
			"CREATE TABLE ModuleInfoDimension (id UNSIGNED BIG INT PRIMARY KEY, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), disturbanceTypeName VARCHAR(255), disturbanceType INTEGER)",
			"CREATE TABLE LocationDimension (id UNSIGNED BIG INT PRIMARY KEY, classifierSetDimId UNSIGNED BIG INT, dateDimId UNSIGNED BIG INT, landClassDimId UNSIGNED BIG INT, area FLOAT, FOREIGN KEY(classifierSetDimId) REFERENCES ClassifierSetDimension(id), FOREIGN KEY(dateDimId) REFERENCES DateDimension(id), FOREIGN KEY(landClassDimId) REFERENCES LandClassDimension(id))",
			"CREATE TABLE DisturbanceDimension (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, moduleInfoDimId UNSIGNED BIG INT, area FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(moduleInfoDimId) REFERENCES ModuleInfoDimension(id))",
			"CREATE TABLE Pools (id UNSIGNED BIG INT, locationDimId UNSIGNED BIG INT, poolId UNSIGNED BIG INT, poolValue FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(poolId) REFERENCES PoolDimension(id))",
			"CREATE TABLE Fluxes (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, moduleInfoDimId UNSIGNED BIG INT, poolSrcDimId UNSIGNED BIG INT, poolDstDimId UNSIGNED BIG INT, fluxValue FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(moduleInfoDimId) REFERENCES ModuleInfoDimension(id), FOREIGN KEY(poolSrcDimId) REFERENCES PoolDimension(id), FOREIGN KEY(poolDstDimId) REFERENCES PoolDimension(id))",
		};

		for (const auto& sql : ddl) {
			tryExecute(session, [&sql](auto& session) {
				session << sql, now;
			});
		}

        std::vector<std::string> csetPlaceholders;
        auto classifierCount = _classifierNames->size();
        for (auto i = 0; i < classifierCount; i++) {
            csetPlaceholders.push_back("?");
        }

        auto csetSql = (boost::format("INSERT INTO ClassifierSetDimension VALUES (?, %1%)")
            % boost::join(csetPlaceholders, ", ")).str();

		tryExecute(session, [this, &csetSql, &classifierCount](auto& session) {
			for (auto cset : this->_classifierSetDimension->getPersistableCollection()) {
				Statement insert(session);
				insert << csetSql, use(cset.get<0>());
				auto values = cset.get<1>();
				for (int i = 0; i < classifierCount; i++) {
					insert, use(values[i]);
				}

				insert.execute();
			}
		});

		std::vector<std::tuple<std::string, Poco::Data::AbstractBinding::Ptr>> dimData {
			{ "INSERT INTO DateDimension VALUES (?, ?, ?, ?, ?, ?, ?)", bind(_dateDimension->getPersistableCollection()) },
			{ "INSERT INTO PoolDimension VALUES (?, ?)", bind(_poolInfoDimension->getPersistableCollection()) },
			{ "INSERT INTO LandClassDimension VALUES (?, ?)", bind(_landClassDimension->getPersistableCollection()) },
			{ "INSERT INTO ModuleInfoDimension VALUES (?, ?, ?, ?, ?, ?, ?, ?)", bind(_moduleInfoDimension->getPersistableCollection()) },
			{ "INSERT INTO LocationDimension VALUES (?, ?, ?, ?, ?)", bind(_locationDimension->getPersistableCollection()) },
			{ "INSERT INTO DisturbanceDimension VALUES (?, ?, ?, ?)", bind(_disturbanceDimension->getPersistableCollection()) },
			{ "INSERT INTO Pools VALUES (?, ?, ?, ?)", bind(_poolDimension->getPersistableCollection()) },
			{ "INSERT INTO Fluxes VALUES (?, ?, ?, ?, ?, ?)", bind(_fluxDimension->getPersistableCollection()) }
		};

		for (auto dimInsert : dimData) {
			tryExecute(session, [&dimInsert](auto& session) {
				session << std::get<0>(dimInsert), std::get<1>(dimInsert), now;
			});
		}
            
        Poco::Data::SQLite::Connector::unregisterConnector();
        MOJA_LOG_INFO << "SQLite insert complete." << std::endl;
    }

	void CBMAggregatorSQLiteWriter::tryExecute(
			Poco::Data::Session& session,
			std::function<void(Poco::Data::Session&)> fn) {

		try {
			session.begin();
			fn(session);
			session.commit();
		} catch (Poco::AssertionViolationException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
		} catch (Poco::Data::SQLite::InvalidSQLStatementException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
		} catch (Poco::Data::SQLite::ConstraintViolationException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
		} catch (Poco::Data::BindingException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
		} catch (const std::exception& e) {
			MOJA_LOG_FATAL << e.what() << std::endl;
		} catch (...) {
			MOJA_LOG_FATAL << "Unknown exception." << std::endl;
		}
	}

}}} // namespace moja::modules::cbm
