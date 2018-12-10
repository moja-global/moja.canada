#include "moja/modules/cbm/cbmaggregatorpostgresqlwriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ilandunitdatawrapper.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ivariable.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/hash.h>

#include <Poco/Exception.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/ODBC/Connector.h>
#include <Poco/Data/ODBC/ODBCException.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

using namespace Poco::Data;
using Poco::format;
using Poco::NotFoundException;

namespace moja {
namespace modules {
namespace cbm {

    void CBMAggregatorPostgreSQLWriter::configure(const DynamicObject& config) {
        _connectionString = config["connection_string"].convert<std::string>();
        _schema = config["schema"].convert<std::string>();
        _schemaLock = moja::hash::hashCombine(_schema);

        if (config.contains("drop_schema")) {
            _dropSchema = config["drop_schema"];
        }
    }

    void CBMAggregatorPostgreSQLWriter::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::SystemInit, &CBMAggregatorPostgreSQLWriter::onSystemInit, *this);
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorPostgreSQLWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown, &CBMAggregatorPostgreSQLWriter::onSystemShutdown, *this);
	}

	void CBMAggregatorPostgreSQLWriter::doSystemInit() {
		if (!_isPrimaryAggregator) {
			return;
		}

        ODBC::Connector::registerConnector();
        Session session(ODBC::Connector::KEY, _connectionString, 0);
        session.setFeature("autoCommit", false);

        std::vector<std::string> ddl;
        if (_dropSchema) {
            ddl.push_back((boost::format("DROP SCHEMA IF EXISTS %1% CASCADE") % _schema).str());
        }

        ddl.push_back((boost::format("CREATE SCHEMA IF NOT EXISTS %1%") % _schema).str());

        session << (boost::format("SELECT pg_advisory_lock(%1%)") % _schemaLock).str(), now;
        session.commit();

        for (const auto& sql : ddl) {
            tryExecute(session, [&sql](auto& sess) {
                sess << sql, now;
            });
        }

        session << (boost::format("SELECT pg_advisory_unlock(%1%)") % _schemaLock).str(), now;
        session.commit();
        Poco::Data::ODBC::Connector::unregisterConnector();
    }

    void CBMAggregatorPostgreSQLWriter::doLocalDomainInit() {
        _spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(
            _landUnitData->getVariable("spatialLocationInfo")->value()
            .extract<std::shared_ptr<flint::IFlintData>>());
    }

    void CBMAggregatorPostgreSQLWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

		if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        MOJA_LOG_INFO << (boost::format("Loading results into %1% on server: %2%")
            % _schema % _connectionString).str();

		ODBC::Connector::registerConnector();
		Session session(ODBC::Connector::KEY, _connectionString, 0);
        session.setFeature("autoCommit", false);
        session << (boost::format("SET search_path = %1%") % _schema).str(), now;

        // Acquire a lock on the schema before attempting to create tables to prevent a race condition
        // with other workers: IF NOT EXISTS could be true at the same time for different processes.
        session << (boost::format("SELECT pg_advisory_lock(%1%)") % _schemaLock).str(), now;
        session.commit();

        std::vector<std::string> ddl{
			(boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS ClassifierSetDimension (tileId BIGINT, blockId BIGINT, id BIGINT, %1% VARCHAR)") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE UNLOGGED TABLE IF NOT EXISTS DateDimension (tileId BIGINT, blockId BIGINT, id BIGINT, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS PoolDimension (id BIGINT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LandClassDimension (tileId BIGINT, blockId BIGINT, id BIGINT, name VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ModuleInfoDimension (tileId BIGINT, blockId BIGINT, id BIGINT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255))",
            "CREATE UNLOGGED TABLE IF NOT EXISTS AgeClassDimension (tileId BIGINT, blockId BIGINT, id INTEGER, startAge INTEGER, endAge INTEGER)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS LocationDimension (tileId BIGINT, blockId BIGINT, id BIGINT, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceTypeDimension (tileId BIGINT, blockId BIGINT, id BIGINT, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceDimension (tileId BIGINT, blockId BIGINT, id BIGINT, locationDimId BIGINT, disturbanceTypeDimId BIGINT, preDistAgeClassDimId INTEGER, area FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Pools (tileId BIGINT, blockId BIGINT, id BIGINT, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Fluxes (tileId BIGINT, blockId BIGINT, id BIGINT, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ErrorDimension (tileId BIGINT, blockId BIGINT, id BIGINT, module VARCHAR, error VARCHAR)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LocationErrorDimension (tileId BIGINT, blockId BIGINT, id BIGINT, locationDimId BIGINT, errorDimId BIGINT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS AgeArea (tileId BIGINT, blockId BIGINT, id BIGINT, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT)",
		};

		for (const auto& sql : ddl) {
			tryExecute(session, [&sql](auto& sess) {
				sess << sql, now;
			});
		}

        // Commit the table DDL.
        session.commit();

        // Release the schema lock in a separate transaction after running the table DDL.
        session << (boost::format("SELECT pg_advisory_unlock(%1%)") % _schemaLock).str(), now;
        session.commit();

        Int64 tileIdx = _spatialLocationInfo->getProperty("tileIdx");
        Int64 blockIdx = _spatialLocationInfo->getProperty("blockIdx");

        auto poolSql = "INSERT INTO PoolDimension VALUES (?, ?) ON CONFLICT (id) DO NOTHING";
        tryExecute(session, [this, &poolSql](auto& sess) {
            Statement insert(sess);
            auto data = this->_poolInfoDimension->getPersistableCollection();
            if (!data.empty()) {
                sess << poolSql, use(data), now;
            }
        });
        session.commit();

        std::vector<std::string> csetPlaceholders;
        auto classifierCount = _classifierNames->size();
        for (auto i = 0; i < classifierCount; i++) {
            csetPlaceholders.push_back("?");
        }

        auto csetSql = (boost::format("INSERT INTO ClassifierSetDimension VALUES (?, ?, ?, %1%)")
            % boost::join(csetPlaceholders, ", ")).str();

		tryExecute(session, [this, &csetSql, &classifierCount, tileIdx, blockIdx](auto& sess) {
			for (auto cset : this->_classifierSetDimension->getPersistableCollection()) {
				Statement insert(sess);
				insert << csetSql, bind(tileIdx), bind(blockIdx), bind(cset.get<0>());
				auto values = cset.get<1>();
				for (int i = 0; i < classifierCount; i++) {
					insert, bind(values[i]);
				}

				insert.execute();
			}
		});

		load(session, tileIdx, blockIdx, "DateDimension",		     _dateDimension);
		load(session, tileIdx, blockIdx, "LandClassDimension",       _landClassDimension);
		load(session, tileIdx, blockIdx, "ModuleInfoDimension",      _moduleInfoDimension);
        load(session, tileIdx, blockIdx, "AgeClassDimension",        _ageClassDimension);
        load(session, tileIdx, blockIdx, "LocationDimension",	     _locationDimension);
        load(session, tileIdx, blockIdx, "DisturbanceTypeDimension", _disturbanceTypeDimension);
		load(session, tileIdx, blockIdx, "DisturbanceDimension",     _disturbanceDimension);
		load(session, tileIdx, blockIdx, "Pools",				     _poolDimension);
		load(session, tileIdx, blockIdx, "Fluxes",				     _fluxDimension);
		load(session, tileIdx, blockIdx, "ErrorDimension",		     _errorDimension);
		load(session, tileIdx, blockIdx, "LocationErrorDimension",   _locationErrorDimension);
		load(session, tileIdx, blockIdx, "AgeArea",				     _ageAreaDimension);
        
        session.commit();
        Poco::Data::ODBC::Connector::unregisterConnector();
        MOJA_LOG_INFO << "PostgreSQL insert complete." << std::endl;
    }

	template<typename TAccumulator>
	void CBMAggregatorPostgreSQLWriter::load(
			Poco::Data::Session& session,
            Int64 tileIdx,
            Int64 blockIdx,
            const std::string& table,
			std::shared_ptr<TAccumulator> dataDimension) {

		MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
		tryExecute(session, [table, dataDimension, tileIdx, blockIdx](auto& sess) {
			auto data = dataDimension->getPersistableCollection();
			if (!data.empty()) {
				std::vector<std::string> placeholders;
				for (auto i = 0; i < data[0].length; i++) {
					placeholders.push_back("?");
				}

				auto sql = (boost::format("INSERT INTO %1% VALUES (%2%, %3%, %4%)")
					% table % tileIdx % blockIdx % boost::join(placeholders, ", ")).str();

				sess << sql, use(data), now;
			}
		});
	}

	void CBMAggregatorPostgreSQLWriter::tryExecute(
			Poco::Data::Session& session,
			std::function<void(Poco::Data::Session&)> fn) {

		try {
			fn(session);
		} catch (Poco::AssertionViolationException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
		} catch (Poco::Data::ODBC::StatementException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
		} catch (Poco::Data::ODBC::ODBCException& exc) {
			MOJA_LOG_FATAL << exc.displayText() << std::endl;
        } catch (Poco::InvalidAccessException& exc) {
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
