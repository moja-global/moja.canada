/**
* @file
* @brief description of CBMAggregatorSQLiteWriter.
*
* ******/

#include "moja/modules/cbm/cbmaggregatorsqlitewriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <Poco/Exception.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/SQLite/SQLiteException.h>

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
    * @brief Configuration function
    *
    * Initialize database name.
    *
    * @param config DynamicObject&
    * @return void
    * ************************/	

    void CBMAggregatorSQLiteWriter::configure(const DynamicObject& config) {
        _dbName = config["databasename"].convert<std::string>();
    }

    /**
    * @brief subscribe to signal
    *
    * Subscribes the signal SystemInit and SystemShutDown
    * using the function onSystemInit,onSystemShutDown respectively.
    *
    *
	* @param notificationCenter NotificationCenter&
    * @return void
    * ************************/	

    void CBMAggregatorSQLiteWriter::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::SystemInit, &CBMAggregatorSQLiteWriter::onSystemInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown, &CBMAggregatorSQLiteWriter::onSystemShutdown, *this);
	}

	/**
	* @brief System initiate
	*
	* Removes the database name.
	*
	*
	* @return void
	* ************************/
	void CBMAggregatorSQLiteWriter::doSystemInit() {
		if (_isPrimaryAggregator) {
			std::remove(_dbName.c_str());
		}
	}

	
    /**
    * @brief doSystemShutDown
    *
    * 
    * Creates unlogged tables for the date dimension, land class dimension,
    * module info dimension, location dimension, disturbance type dimension, 
    * disturbance dimension,pools,fluxes,error dimension, age class dimension
    * location error dimension, and age area and loads data into these tables on SqLite.
    *
    *
    * @return void
    * ************************/	
    void CBMAggregatorSQLiteWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

		if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        MOJA_LOG_INFO << (boost::format("Loading results into %1%") % _dbName).str();
		SQLite::Connector::registerConnector();
		Session session("SQLite", _dbName);

		std::vector<std::string> ddl{
            "PRAGMA foreign_keys=ON",
			(boost::format("CREATE TABLE ClassifierSetDimension (id UNSIGNED BIG INT PRIMARY KEY, %1% VARCHAR)") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE TABLE DateDimension (id UNSIGNED BIG INT PRIMARY KEY, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT)",
			"CREATE TABLE PoolDimension (id UNSIGNED BIG INT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE TABLE LandClassDimension (id UNSIGNED BIG INT PRIMARY KEY, name VARCHAR(255))",
			"CREATE TABLE ModuleInfoDimension (id UNSIGNED BIG INT PRIMARY KEY, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255))",
            "CREATE TABLE AgeClassDimension (id UNSIGNED INT PRIMARY KEY, startAge UNSIGNED INT, endAge UNSIGNED INT)",
            "CREATE TABLE LocationDimension (id UNSIGNED BIG INT PRIMARY KEY, classifierSetDimId UNSIGNED BIG INT, dateDimId UNSIGNED BIG INT, landClassDimId UNSIGNED BIG INT, ageClassDimId UNSIGNED INT, area FLOAT, FOREIGN KEY(classifierSetDimId) REFERENCES ClassifierSetDimension(id), FOREIGN KEY(dateDimId) REFERENCES DateDimension(id), FOREIGN KEY(landClassDimId) REFERENCES LandClassDimension(id), FOREIGN KEY(ageClassDimId) REFERENCES AgeClassDimension(id))",
            "CREATE TABLE DisturbanceTypeDimension (id UNSIGNED BIG INT PRIMARY KEY, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255))",
			"CREATE TABLE DisturbanceDimension (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, disturbanceTypeDimId UNSIGNED BIG INT, previousLocationDimId UNSIGNED INT, area FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(disturbanceTypeDimId) REFERENCES DisturbanceTypeDimension(id), FOREIGN KEY(previousLocationDimId) REFERENCES LocationDimension(id))",
			"CREATE TABLE Pools (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, poolId UNSIGNED BIG INT, poolValue FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(poolId) REFERENCES PoolDimension(id))",
			"CREATE TABLE Fluxes (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, moduleInfoDimId UNSIGNED BIG INT, disturbanceDimId UNSIGNED BIG INT, poolSrcDimId UNSIGNED BIG INT, poolDstDimId UNSIGNED BIG INT, fluxValue FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(moduleInfoDimId) REFERENCES ModuleInfoDimension(id), FOREIGN KEY(disturbanceDimId) REFERENCES DisturbanceDimension(id), FOREIGN KEY(poolSrcDimId) REFERENCES PoolDimension(id), FOREIGN KEY(poolDstDimId) REFERENCES PoolDimension(id))",
			"CREATE TABLE ErrorDimension (id UNSIGNED BIG INT PRIMARY KEY, module VARCHAR, error VARCHAR)",
			"CREATE TABLE LocationErrorDimension (id UNSIGNED BIG INT, locationDimId UNSIGNED BIG INT, errorDimId UNSIGNED BIG INT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(errorDimId) REFERENCES ErrorDimension(id))",
			"CREATE TABLE AgeArea (id UNSIGNED BIG INT PRIMARY KEY, locationDimId UNSIGNED BIG INT, ageClassDimId UNSIGNED INT, area FLOAT, FOREIGN KEY(locationDimId) REFERENCES LocationDimension(id), FOREIGN KEY(ageClassDimId) REFERENCES AgeClassDimension(id))",
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

        Poco::Data::SQLite::Connector::unregisterConnector();
        MOJA_LOG_INFO << "SQLite insert complete." << std::endl;
    }

	/**
	* @brief Load data
	*
	* Loads persistable collecton data into the table
	* using sql command
	* 
	* @param session Session&
	* @param table string&
	* @param dataDimension shared_ptr<TAccumulator>
	* @return void
	* ************************/

	template<typename TAccumulator>
	void CBMAggregatorSQLiteWriter::load(
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

	/**
	* @brief tryExecute Function
	*
	* Exceutes the session.
	*
	* @param session Session&
	* @param fn function<void(session&)>
	* @return void
	* @raise AssertionViolationeException: Handles any program error
	* @raise InvalidSQLStatmentException: If the sql statement is invalid
	* @raise ConstraintViolationException: Occurs if the sql statment violates any constraint
	* @raise BindingException: Handles any binding error
	* @raise exception: Handles error
	* ************************/

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
