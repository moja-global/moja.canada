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
        boost::replace_first(_chConnectionString, "dbname=postgres", "dbname=default");
        _schema = config["schema"].convert<std::string>();
    }

    void CBMAggregatorLibPQXXWriter::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorLibPQXXWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown,  &CBMAggregatorLibPQXXWriter::onSystemShutdown,  *this);
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
        auto records = dataDimension->records();
        if (!records.empty()) {
            auto columns = records[0].header(*_classifierNames);
            boost::replace_first(columns, "\n", "");
            auto stream = pqxx::stream_to::raw_table(tx, table, columns);
            for (auto& record : records) {
                stream << record.asVector();
            }

            stream.complete();
        }
    }

}}} // namespace moja::modules::cbm
