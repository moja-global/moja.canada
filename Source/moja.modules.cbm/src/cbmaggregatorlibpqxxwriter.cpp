/**
 * @file
 * The CBMAggregatorLibPQXXWriter module writes the stand-level information gathered 
 * by CBMAggregatorLandUnitData into a PostgreSQL database. It is designed mainly for 
 * distributed runs where the simulation is divided up and each portion of work is loaded 
 * into a separate set of tables before being merged together with a post-processing script,
 * although this module can also be used for a standard simulation
 ********/

#include "moja/modules/cbm/cbmaggregatorlibpqxxwriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ilandunitdatawrapper.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ivariable.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/hash.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace pqxx;
using Poco::format;
using Poco::NotFoundException;

namespace moja {
namespace modules {
namespace cbm {

    /**
    * Configuration function
    *
    * Assign CBMAggregatorLibPQXXWriter._connectionString as variable "connection_string" in parameter config, \n
    * CBMAggregatorLibPQXXWriter._schema as variable "schema" in parameter config, \n
    * If parameter config has "drop_schema", assign it to CBMAggregatorLibPQXXWriter._dropSchema
    * 
    * @param config DynamicObject&
    * @return void
    * ************************/

    void CBMAggregatorLibPQXXWriter::configure(const DynamicObject& config) {
        _connectionString = config["connection_string"].convert<std::string>();
        _schema = config["schema"].convert<std::string>();

        if (config.contains("drop_schema")) {
            _dropSchema = config["drop_schema"];
        }
    }

    /**
    * Subscribes to the signals SystemInit, LocalDomainInit and SystemShutDown
    * 
    * @param notificationCenter NotificationCenter&
    * @return void
    * ************************/

    void CBMAggregatorLibPQXXWriter::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::SystemInit,      &CBMAggregatorLibPQXXWriter::onSystemInit,      *this);
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorLibPQXXWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown,  &CBMAggregatorLibPQXXWriter::onSystemShutdown,  *this);
	}
    
    /**
    * Initiate System
    *
    * If CBMAggregatorLibPQXXWriter._isPrimaryAggregator and CBMAggregatorLibPQXXWriter._dropSchema are true \n
    * drop CBMAggregatorLibPQXXWriter._schema \n
    * Create CBMAggregatorLibPQXXWriter._schema
    *
    * @return void
    * ************************/

	void CBMAggregatorLibPQXXWriter::doSystemInit() {
        if (!_isPrimaryAggregator) {
            return;
        }

        connection conn(_connectionString);
        if (_dropSchema) {
            doIsolated(conn, (boost::format("DROP SCHEMA %1% CASCADE;") % _schema).str(), true);
        }

        doIsolated(conn, (boost::format("CREATE SCHEMA %1%;") % _schema).str(), true);
    }

    /**
    * Initiate Local Domain
    *
    * Assign CBMAggregatorLibPQXXWriter._jobId the value of variable "job_id" in _landUnitData, \n
    * if it exists, else to 0
    *
    * @return void
    * ************************/
    void CBMAggregatorLibPQXXWriter::doLocalDomainInit() {
        _jobId = _landUnitData->hasVariable("job_id")
            ? _landUnitData->getVariable("job_id")->value().convert<Int64>()
            : 0;
    }

    /**
    * doSystemShutDown
    *
    * If CBMAggregatorLibPQXXWriter._isPrimaryAggregator is true, create unlogged tables for the DateDimension, LandClassDimension, \n
	* PoolDimension, ClassifierSetDimension, ModuleInfoDimension, LocationDimension, DisturbanceTypeDimension, \n
    * DisturbanceDimension, Pools, Fluxes, ErrorDimension, AgeClassDimension, LocationErrorDimension, \n
	* and AgeArea if they do not already exist, and load data into tables on PostgreSQL
    * 
    * @return void
    * ************************/
    void CBMAggregatorLibPQXXWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        MOJA_LOG_INFO << (boost::format("Loading results into %1% on server: %2%")
            % _schema % _connectionString).str();

        connection conn(_connectionString);
        doIsolated(conn, (boost::format("SET search_path = %1%;") % _schema).str());

        MOJA_LOG_INFO << "Creating results tables.";
        doIsolated(conn, "CREATE UNLOGGED TABLE IF NOT EXISTS CompletedJobs (id BIGINT PRIMARY KEY);", false);

        bool resultsPreviouslyLoaded = perform([&conn, this] {
            return !nontransaction(conn).exec((boost::format(
                "SELECT 1 FROM CompletedJobs WHERE id = %1%;"
            ) % _jobId).str()).empty();
        });

        if (resultsPreviouslyLoaded) {
            MOJA_LOG_INFO << "Results previously loaded for jobId " << _jobId << " - skipping.";
            return;
        }

        perform([&conn, this] {
            work tx(conn);

            // First, try to insert into the completed jobs table - if this is a duplicate, the transaction
            // will fail immediately.
            tx.exec((boost::format("INSERT INTO CompletedJobs VALUES (%1%);") % _jobId).str());

            // Bulk load the job results into a temporary set of tables.
            std::vector<std::string> tempTableDdl{
                (boost::format("CREATE UNLOGGED TABLE flux_%1% (year INTEGER, %2% VARCHAR, unfccc_land_class VARCHAR, age_range VARCHAR, %3%_previous VARCHAR, unfccc_land_class_previous VARCHAR, age_range_previous VARCHAR, disturbance_type VARCHAR, disturbance_code INTEGER, from_pool VARCHAR, to_pool VARCHAR, flux_tc NUMERIC);") % _jobId % boost::join(*_classifierNames, " VARCHAR, ") % boost::join(*_classifierNames, "_previous VARCHAR, ")).str(),
                (boost::format("CREATE UNLOGGED TABLE pool_%1% (year INTEGER, %2% VARCHAR, unfccc_land_class VARCHAR, age_range VARCHAR, pool VARCHAR, pool_tc NUMERIC);") % _jobId % boost::join(*_classifierNames, " VARCHAR, ")).str(),
                (boost::format("CREATE UNLOGGED TABLE error_%1% (year INTEGER, %2% VARCHAR, module VARCHAR, error VARCHAR, area NUMERIC);") % _jobId % boost::join(*_classifierNames, " VARCHAR, ")).str(),
                (boost::format("CREATE UNLOGGED TABLE age_%1% (year INTEGER, %2% VARCHAR, unfccc_land_class VARCHAR, age_range VARCHAR, area NUMERIC);") % _jobId % boost::join(*_classifierNames, " VARCHAR, ")).str(),
                (boost::format("CREATE UNLOGGED TABLE disturbance_%1% (year INTEGER, %2% VARCHAR, unfccc_land_class VARCHAR, age_range VARCHAR, %3%_previous VARCHAR, unfccc_land_class_previous VARCHAR, age_range_previous VARCHAR, disturbance_type VARCHAR, disturbance_code INTEGER, area NUMERIC);") % _jobId % boost::join(*_classifierNames, " VARCHAR, ") % boost::join(*_classifierNames, "_previous VARCHAR, ")).str()
            };

            for (const auto& ddl : tempTableDdl) {
                tx.exec(ddl);
            }

            load(tx, _jobId, "flux", _fluxDimension);
            load(tx, _jobId, "pool", _poolDimension);
            load(tx, _jobId, "error", _errorDimension);
            load(tx, _jobId, "age", _ageDimension);
            load(tx, _jobId, "disturbance", _disturbanceDimension);

            tx.commit();
        });

        MOJA_LOG_INFO << "PostgreSQL insert complete." << std::endl;
    }

    /**
    * Perform a single transaction using SQL commands (Overloaded function) 
    * 
    * @param conn connection_base&
    * @param sql string
    * @param optional bool
    * @return void
    * ************************/
    void CBMAggregatorLibPQXXWriter::doIsolated(pqxx::connection_base& conn, std::string sql, bool optional) {
        perform([&conn, sql, optional] {
            try {
                work tx(conn);
                tx.exec(sql);
                tx.commit();
            } catch (...) {
                if (!optional) {
                    throw;
                }
            }
        });
    }

    /**
    * Perform transactions using SQL commands (Overloaded function) 
    * 
    * @param conn connection_base&
    * @param sql vector<string>
    * @param optional bool
    * @return void
    * ************************/

    void CBMAggregatorLibPQXXWriter::doIsolated(pqxx::connection_base& conn, std::vector<std::string> sql, bool optional) {
        perform([&conn, sql, optional] {
            try {
                work tx(conn);
                for (auto stmt : sql) {
                    tx.exec(stmt);
                }

                tx.commit();
            } catch (...) {
                if (!optional) {
                    throw;
                }
            }
        });
    }

    /**
    * Load each record in paramter dataDimension into table (table name is based on parameter table and jobId) 
    *
    * @param tx work&
    * @param jobId Int64
    * @param table string
    * @param dataDimension shared_ptr<TAccumulator> 
    * @return void
    * ************************/

    template<typename TAccumulator>
    void CBMAggregatorLibPQXXWriter::load(
        pqxx::work& tx,
        Int64 jobId,
        const std::string& table,
        std::shared_ptr<TAccumulator> dataDimension) {

        MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
        auto tempTableName = (boost::format("%1%_%2%") % table % jobId).str();
        pqxx::stream_to stream(tx, tempTableName);
        auto records = dataDimension->records();
        if (!records.empty()) {
            for (auto& record : records) {
                stream << record.asVector();
            }
        }
            
        stream.complete();
    }

}}} // namespace moja::modules::cbm
