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
#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

using namespace pqxx;
using Poco::format;
using Poco::NotFoundException;

namespace moja {
namespace modules {
namespace cbm {

    void CBMAggregatorLibPQXXWriter::configure(const DynamicObject& config) {
        _pgConnectionString = config["connection_string"].convert<std::string>();
        _chConnectionString = _pgConnectionString;
        boost::replace_first(_chConnectionString, "5432", "5430");
        boost::replace_first(_chConnectionString, "dbname=postgres", "");
        _schema = config["schema"].convert<std::string>();
    }

    void CBMAggregatorLibPQXXWriter::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::SystemInit,      &CBMAggregatorLibPQXXWriter::onSystemInit,      *this);
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorLibPQXXWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown,  &CBMAggregatorLibPQXXWriter::onSystemShutdown,  *this);
	}
    
	void CBMAggregatorLibPQXXWriter::doSystemInit() {
        if (!_isPrimaryAggregator) {
            return;
        }

        connection pgConn(_pgConnectionString);
        connection chConn(_chConnectionString);
        if (_dropSchema) {
            doIsolated(pgConn, (boost::format("DROP SCHEMA %1% CASCADE;") % _schema).str(), true);
            doIsolated(chConn, (boost::format("DROP DATABASE %1%;") % _schema).str(), true);
        }

        doIsolated(pgConn, (boost::format("CREATE SCHEMA %1%;") % _schema).str(), true);
        doIsolated(chConn, (boost::format("CREATE DATABASE %1%;") % _schema).str(), true);
    }

    void CBMAggregatorLibPQXXWriter::doLocalDomainInit() {
        _jobId = _landUnitData->hasVariable("job_id")
            ? _landUnitData->getVariable("job_id")->value().convert<Int64>()
            : 0;
    }

    void CBMAggregatorLibPQXXWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        MOJA_LOG_INFO << (boost::format("Loading results into %1% on server: %2%")
            % _schema % _pgConnectionString).str();

        connection pgConn(_pgConnectionString);
        connection chConn(_chConnectionString);
        doIsolated(pgConn, (boost::format("SET search_path = %1%;") % _schema).str());

        MOJA_LOG_INFO << "Creating results tables.";
        doIsolated(pgConn, "CREATE UNLOGGED TABLE IF NOT EXISTS CompletedJobs (id BIGINT PRIMARY KEY);", false);

        bool resultsPreviouslyLoaded = perform([&pgConn, this] {
            return !nontransaction(pgConn).exec((boost::format(
                "SELECT 1 FROM CompletedJobs WHERE id = %1%;"
            ) % _jobId).str()).empty();
        });

        if (resultsPreviouslyLoaded) {
            MOJA_LOG_INFO << "Results previously loaded for jobId " << _jobId << " - skipping.";
            return;
        }

        perform([&pgConn, &chConn, this] {
            work pgTx(pgConn);
            work chTx(chConn);

            // First, try to insert into the completed jobs table - if this is a duplicate, the transaction
            // will fail immediately.
            pgTx.exec((boost::format("INSERT INTO CompletedJobs VALUES (%1%);") % _jobId).str());

            // Bulk load the job results into a temporary set of tables.
            std::vector<std::string> tempTableDdl{
                (boost::format("CREATE TABLE IF NOT EXISTS %1%.raw_fluxes (year INTEGER PRIMARY KEY, %2% VARCHAR PRIMARY KEY, unfccc_land_class VARCHAR PRIMARY KEY, age_range VARCHAR PRIMARY KEY, %3%_previous VARCHAR PRIMARY KEY, unfccc_land_class_previous VARCHAR PRIMARY KEY, age_range_previous VARCHAR PRIMARY KEY, disturbance_type VARCHAR PRIMARY KEY, disturbance_code INTEGER PRIMARY KEY, from_pool VARCHAR PRIMARY KEY, to_pool VARCHAR PRIMARY KEY, flux_tc NUMERIC) ENGINE = SummingMergeTree;") % _schema % boost::join(*_classifierNames, " VARCHAR PRIMARY KEY, ") % boost::join(*_classifierNames, "_previous VARCHAR PRIMARY KEY, ")).str(),
                (boost::format("CREATE TABLE IF NOT EXISTS %1%.raw_pools (year INTEGER PRIMARY KEY, %2% VARCHAR PRIMARY KEY, unfccc_land_class VARCHAR PRIMARY KEY, age_range VARCHAR PRIMARY KEY, pool VARCHAR PRIMARY KEY, pool_tc NUMERIC) ENGINE = SummingMergeTree;") % _schema % boost::join(*_classifierNames, " VARCHAR PRIMARY KEY, ")).str(),
                (boost::format("CREATE TABLE IF NOT EXISTS %1%.raw_errors (year INTEGER PRIMARY KEY, %2% VARCHAR PRIMARY KEY, module VARCHAR PRIMARY KEY, error VARCHAR PRIMARY KEY, area NUMERIC) ENGINE = SummingMergeTree;") % _schema % boost::join(*_classifierNames, " VARCHAR PRIMARY KEY, ")).str(),
                (boost::format("CREATE TABLE IF NOT EXISTS %1%.raw_ages (year INTEGER PRIMARY KEY, %2% VARCHAR PRIMARY KEY, unfccc_land_class VARCHAR PRIMARY KEY, age_range VARCHAR PRIMARY KEY, area NUMERIC) ENGINE = SummingMergeTree;") % _schema % boost::join(*_classifierNames, " VARCHAR PRIMARY KEY, ")).str(),
                (boost::format("CREATE TABLE IF NOT EXISTS %1%.raw_disturbances (year INTEGER PRIMARY KEY, %2% VARCHAR PRIMARY KEY, unfccc_land_class VARCHAR PRIMARY KEY, age_range VARCHAR PRIMARY KEY, %3%_previous VARCHAR PRIMARY KEY, unfccc_land_class_previous VARCHAR PRIMARY KEY, age_range_previous VARCHAR PRIMARY KEY, disturbance_type VARCHAR PRIMARY KEY, disturbance_code INTEGER PRIMARY KEY, area NUMERIC) ENGINE = SummingMergeTree;") % _schema % boost::join(*_classifierNames, " VARCHAR PRIMARY KEY, ") % boost::join(*_classifierNames, "_previous VARCHAR PRIMARY KEY, ")).str()
            };

            for (const auto& ddl : tempTableDdl) {
                chTx.exec(ddl);
            }

            load(chTx, (boost::format("%1%.raw_fluxes") % _schema).str(), _fluxDimension);
            load(chTx, (boost::format("%1%.raw_pools") % _schema).str(), _poolDimension);
            load(chTx, (boost::format("%1%.raw_errors") % _schema).str(), _errorDimension);
            load(chTx, (boost::format("%1%.raw_ages") % _schema).str(), _ageDimension);
            load(chTx, (boost::format("%1%.raw_disturbances") % _schema).str(), _disturbanceDimension);

            pgTx.commit();
            chTx.commit();
        });

        MOJA_LOG_INFO << "PostgreSQL insert complete." << std::endl;
    }

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

    template<typename TAccumulator>
    void CBMAggregatorLibPQXXWriter::load(
        pqxx::work& tx,
        const std::string& table,
        std::shared_ptr<TAccumulator> dataDimension) {

        MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
        pqxx::stream_to stream(tx, table);
        auto records = dataDimension->records();
        if (!records.empty()) {
            for (auto& record : records) {
                stream << record.asVector();
            }
        }
            
        stream.complete();
    }

}}} // namespace moja::modules::cbm
