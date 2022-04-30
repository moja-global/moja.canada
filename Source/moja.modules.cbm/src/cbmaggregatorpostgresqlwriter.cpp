 /**
 * @file
 * @brief The class subscribes the schema ,pool dimension 
 * and classifier set dimension to their respective signals
 * ******/
#include "moja/modules/cbm/cbmaggregatorpostgresqlwriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ilandunitdatawrapper.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ivariable.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

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

   /**
   * @brief configuration function
   *
   * This is a function to configure
   * the connection string, schema,and if available the drop schema.
   * The values are passed and assigned here
   *
   * @param config DynamicObject&
   * @return void
   * ************************/

    void CBMAggregatorPostgreSQLWriter::configure(const DynamicObject& config) {
        _connectionString = config["connection_string"].convert<std::string>();
        _schema = config["schema"].convert<std::string>();
        if (config.contains("drop_schema")) {
            _dropSchema = config["drop_schema"];
        }
    }

    /**
    * @brief subscribe to signal
    *
    * This function subscribes the signal SystemInit and SystemShutDown
    * using the function onSystemInit,onSystemShutDown respectively.
    * The values are passed and assigned here
    *
    * @return void
    * ************************/

    void CBMAggregatorPostgreSQLWriter::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::SystemInit, &CBMAggregatorPostgreSQLWriter::onSystemInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown, &CBMAggregatorPostgreSQLWriter::onSystemShutdown, *this);
	}

    /**
    * @brief Initiate System
    *
    * This function creates schema if it does not already exist 
    * and deletes schema if it already exists.
    *
    * @return void
    * ************************/

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

        for (const auto& sql : ddl) {
            tryExecute(session, [&sql](auto& sess) {
                sess << sql, now;
            });
        }

        session.commit();
        Poco::Data::ODBC::Connector::unregisterConnector();
    }

    /**
    * @brief doSystemShutDown
    *
    * This function loads data into the pool dimension and classifer set dimension.
    * It also creates tables for the date dimension, land class dimension,
    * module info dimension, location dimension, disturbance type dimension, 
    * disturbance dimension,pools,fluxes,error dimension, age class dimension
    * location error dimension, and age area if they do not already exist, and loads data into these tables.
    *
    *
    * @return void
    * ************************/

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

        std::vector<std::string> ddl{
			(boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS ClassifierSetDimension (id BIGINT, %1% VARCHAR)") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE UNLOGGED TABLE IF NOT EXISTS DateDimension (id BIGINT, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS PoolDimension (id BIGINT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LandClassDimension (id BIGINT, name VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ModuleInfoDimension (id BIGINT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255))",
            "CREATE UNLOGGED TABLE IF NOT EXISTS AgeClassDimension (id INTEGER, startAge INTEGER, endAge INTEGER)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS LocationDimension (id BIGINT, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceTypeDimension (id BIGINT, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceDimension (id BIGINT, locationDimId BIGINT, disturbanceTypeDimId BIGINT, previousLocationDimId INTEGER, area FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Pools (id BIGINT, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Fluxes (id BIGINT, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ErrorDimension (id BIGINT, module VARCHAR, error VARCHAR)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LocationErrorDimension (id BIGINT, locationDimId BIGINT, errorDimId BIGINT)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS AgeArea (id BIGINT, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT)",
		};

		for (const auto& sql : ddl) {
			tryExecute(session, [&sql](auto& sess) {
				sess << sql, now;
			});
		}

        // Commit the table DDL.
        session.commit();

        auto poolSql = "INSERT INTO PoolDimension VALUES (?, ?) ON CONFLICT (id) DO NOTHING";
        tryExecute(session, [this, &poolSql](auto& sess) {
            Statement insert(sess);
            auto data = this->_poolInfoDimension->getPersistableCollection();
            if (!data.empty()) {
                sess << poolSql, use(data), now;
            }
        });

        std::vector<std::string> csetPlaceholders;
        auto classifierCount = _classifierNames->size();
        for (auto i = 0; i < classifierCount; i++) {
            csetPlaceholders.push_back("?");
        }

        auto csetSql = (boost::format("INSERT INTO ClassifierSetDimension VALUES (?, %1%)")
            % boost::join(csetPlaceholders, ", ")).str();

		tryExecute(session, [this, &csetSql, &classifierCount](auto& sess) {
			for (auto cset : this->_classifierSetDimension->getPersistableCollection()) {
				Statement insert(sess);
				insert << csetSql, bind(cset.get<0>());
				auto values = cset.get<1>();
				for (int i = 0; i < classifierCount; i++) {
					insert, bind(values[i]);
				}

				insert.execute();
			}
		});

		load(session, _jobId, "DateDimension",		      _dateDimension);
		load(session, _jobId, "LandClassDimension",       _landClassDimension);
		load(session, _jobId, "ModuleInfoDimension",      _moduleInfoDimension);
        load(session, _jobId, "AgeClassDimension",        _ageClassDimension);
        load(session, _jobId, "LocationDimension",	      _locationDimension);
        load(session, _jobId, "DisturbanceTypeDimension", _disturbanceTypeDimension);
		load(session, _jobId, "DisturbanceDimension",     _disturbanceDimension);
		load(session, _jobId, "Pools",				      _poolDimension);
		load(session, _jobId, "Fluxes",				      _fluxDimension);
		load(session, _jobId, "ErrorDimension",		      _errorDimension);
		load(session, _jobId, "LocationErrorDimension",   _locationErrorDimension);
		load(session, _jobId, "AgeArea",				  _ageAreaDimension);
        
        session.commit();
        Poco::Data::ODBC::Connector::unregisterConnector();
        MOJA_LOG_INFO << "PostgreSQL insert complete." << std::endl;
    }

	/**
	* @brief Load data
	*
	* This function loads data into the table
	* using sql command
	*
	* @return void
	* ************************/

	template<typename TAccumulator>
	void CBMAggregatorPostgreSQLWriter::load(
			Poco::Data::Session& session,
            Int64 jobId,
            const std::string& table,
			std::shared_ptr<TAccumulator> dataDimension) {

		MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
		tryExecute(session, [table, dataDimension, jobId](auto& sess) {
			auto data = dataDimension->getPersistableCollection();
			if (!data.empty()) {
				std::vector<std::string> placeholders;
				for (auto i = 0; i < data[0].length; i++) {
					placeholders.push_back("?");
				}

				auto sql = (boost::format("INSERT INTO %1% VALUES (%2%)")
					% table % boost::join(placeholders, ", ")).str();

				sess << sql, use(data), now;
			}
		});
	}

	/**
	* @brief tryExecute Function
	*
	* This function is used to keep the database connection open 
	* and to catch any exceptions.
	*
	* @return void
	* ************************/

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
