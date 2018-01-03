#include "moja/modules/cbm/cbmaggregatorpostgresqlwriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>

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

    void CBMAggregatorPostgreSQLWriter::configure(const DynamicObject& config) {
        _connectionString = config["connection_string"].convert<std::string>();
        _schema = config["schema"].convert<std::string>();
    }

    void CBMAggregatorPostgreSQLWriter::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::SystemInit, &CBMAggregatorPostgreSQLWriter::onSystemInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown, &CBMAggregatorPostgreSQLWriter::onSystemShutdown, *this);
	}

	void CBMAggregatorPostgreSQLWriter::doSystemInit() {
		if (!_isPrimaryAggregator) {
			return;
		}

        ODBC::Connector::registerConnector();
        Session session(ODBC::Connector::KEY, _connectionString);

        std::vector<std::string> ddl{
            (boost::format("DROP SCHEMA IF EXISTS %1% CASCADE") % _schema).str(),
            (boost::format("CREATE SCHEMA %1%") % _schema).str()
        };

        for (const auto& sql : ddl) {
            tryExecute(session, [&sql](auto& sess) {
                sess << sql, now;
            });
        }

        Poco::Data::ODBC::Connector::unregisterConnector();
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
		Session session(ODBC::Connector::KEY, _connectionString);

        std::vector<std::string> ddl{
            (boost::format("SET search_path = %1%") % _schema).str(),
			(boost::format("CREATE TABLE ClassifierSetDimension (id BIGINT PRIMARY KEY, %1% VARCHAR)") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE TABLE DateDimension (id BIGINT PRIMARY KEY, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)",
			"CREATE TABLE PoolDimension (id BIGINT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE TABLE LandClassDimension (id BIGINT PRIMARY KEY, name VARCHAR(255))",
			"CREATE TABLE ModuleInfoDimension (id BIGINT PRIMARY KEY, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255))",
            "CREATE TABLE AgeClassDimension (id INTEGER PRIMARY KEY, startAge INTEGER, endAge INTEGER)",
            "CREATE TABLE LocationDimension (id BIGINT PRIMARY KEY, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT, FOREIGN KEY(classifierSetDimId) REFERENCES ClassifierSetDimension(id), FOREIGN KEY(dateDimId) REFERENCES DateDimension(id), FOREIGN KEY(landClassDimId) REFERENCES LandClassDimension(id), FOREIGN KEY(ageClassDimId) REFERENCES AgeClassDimension(id))",
            "CREATE TABLE DisturbanceTypeDimension (id BIGINT PRIMARY KEY, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255))",
			"CREATE TABLE DisturbanceDimension (id BIGINT PRIMARY KEY, locationDimId BIGINT, disturbanceTypeDimId BIGINT, preDistAgeClassDimId INTEGER, area FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(disturbanceTypeDimId) REFERENCES DisturbanceTypeDimension(id), FOREIGN KEY(preDistAgeClassDimId) REFERENCES AgeClassDimension(id))",
			"CREATE TABLE Pools (id BIGINT PRIMARY KEY, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(poolId) REFERENCES PoolDimension(id))",
			"CREATE TABLE Fluxes (id BIGINT PRIMARY KEY, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(moduleInfoDimId) REFERENCES ModuleInfoDimension(id), FOREIGN KEY(disturbanceDimId) REFERENCES DisturbanceDimension(id), FOREIGN KEY(poolSrcDimId) REFERENCES PoolDimension(id), FOREIGN KEY(poolDstDimId) REFERENCES PoolDimension(id))",
			"CREATE TABLE ErrorDimension (id BIGINT PRIMARY KEY, module VARCHAR, error VARCHAR)",
			"CREATE TABLE LocationErrorDimension (id BIGINT, locationDimId BIGINT, errorDimId BIGINT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(errorDimId) REFERENCES ErrorDimension(id))",
			"CREATE TABLE AgeArea (id BIGINT PRIMARY KEY, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(ageClassDimId) REFERENCES AgeClassDimension(id))",			
		};

		for (const auto& sql : ddl) {
			tryExecute(session, [&sql](auto& sess) {
				sess << sql, now;
			});
		}

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

		load(session, "DateDimension",		      _dateDimension);
		load(session, "PoolDimension",		      _poolInfoDimension);
		load(session, "LandClassDimension",       _landClassDimension);
		load(session, "ModuleInfoDimension",      _moduleInfoDimension);
        load(session, "AgeClassDimension",        _ageClassDimension);
        load(session, "LocationDimension",	      _locationDimension);
        load(session, "DisturbanceTypeDimension", _disturbanceTypeDimension);
		load(session, "DisturbanceDimension",     _disturbanceDimension);
		load(session, "Pools",				      _poolDimension);
		load(session, "Fluxes",				      _fluxDimension);
		load(session, "ErrorDimension",		      _errorDimension);
		load(session, "LocationErrorDimension",   _locationErrorDimension);
		load(session, "AgeArea",				  _ageAreaDimension);

        Poco::Data::ODBC::Connector::unregisterConnector();
        MOJA_LOG_INFO << "PostgreSQL insert complete." << std::endl;
    }

	template<typename TAccumulator>
	void CBMAggregatorPostgreSQLWriter::load(
			Poco::Data::Session& session,
			const std::string& table,
			std::shared_ptr<TAccumulator> dataDimension) {

		MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
		tryExecute(session, [table, dataDimension](auto& sess) {
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
