/**
 * @file
 * The CBMAggregatorHybridLibPQXXWriter module writes the stand-level information gathered 
 * by CBMAggregatorLandUnitData into a PostgreSQL database. It is designed mainly for 
 * distributed runs where the simulation is divided up and each portion of work is loaded 
 * into a separate set of tables before being merged together with a post-processing script,
 * although this module can also be used for a standard simulation
 ********/

#include "moja/modules/cbm/cbmaggregatorhybridlibpqxxwriter.h"

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

    void CBMAggregatorHybridLibPQXXWriter::configure(const DynamicObject& config) {
        _pgConnectionString = config["connection_string"].convert<std::string>();
        _chConnectionString = _pgConnectionString;
        boost::replace_first(_chConnectionString, "5432", "5430");
        boost::replace_first(_chConnectionString, "dbname=postgres", "dbname=default");
        _schema = config["schema"].convert<std::string>();
    }

    void CBMAggregatorHybridLibPQXXWriter::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorHybridLibPQXXWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown,  &CBMAggregatorHybridLibPQXXWriter::onSystemShutdown,  *this);
	}
    
    void CBMAggregatorHybridLibPQXXWriter::doLocalDomainInit() {
        _jobId = _landUnitData->hasVariable("job_id")
            ? _landUnitData->getVariable("job_id")->value().convert<Int64>()
            : 0;
    }

    void CBMAggregatorHybridLibPQXXWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        MOJA_LOG_INFO << (boost::format("Loading results into %1% on server: %2%")
            % _schema % _chConnectionString).str();

        connection chConn(_chConnectionString);

        std::string guardTable = (boost::format("completed_%1%") % _jobId).str();
        bool resultsPreviouslyLoaded = perform([&chConn, &guardTable, this] {
            work tx(chConn);
            return tx.exec((boost::format(
                "EXISTS TABLE %1%.%2%;"
            ) % _schema % guardTable).str()).at(0, 0).as<int>() == 1;
        });

        if (resultsPreviouslyLoaded) {
            MOJA_LOG_INFO << "Results previously loaded for jobId " << _jobId << " - skipping.";
            return;
        }

        perform([&chConn, &guardTable, this] {
            work chTx(chConn);

            // ClickHouse doesn't support unique constraints, so the guard against
            // duplicate loads for the same job has to be a table. If this is a
            // duplicate, the transaction will fail and roll back the data load.
            chTx.exec((boost::format("CREATE VIEW %1%.%2% AS SELECT 1;") % _schema % guardTable).str());

            load(chTx, (boost::format("%1%.raw_fluxes") % _schema).str(), _fluxDimension);
            load(chTx, (boost::format("%1%.raw_pools") % _schema).str(), _poolDimension);
            load(chTx, (boost::format("%1%.raw_errors") % _schema).str(), _errorDimension);
            load(chTx, (boost::format("%1%.raw_ages") % _schema).str(), _ageDimension);
            load(chTx, (boost::format("%1%.raw_disturbances") % _schema).str(), _disturbanceDimension);

            chTx.commit();
        });

        MOJA_LOG_INFO << "PostgreSQL insert complete." << std::endl;
    }

    void CBMAggregatorHybridLibPQXXWriter::doIsolated(pqxx::connection_base& conn, std::string sql, bool optional) {
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

    void CBMAggregatorHybridLibPQXXWriter::doIsolated(pqxx::connection_base& conn, std::vector<std::string> sql, bool optional) {
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
    void CBMAggregatorHybridLibPQXXWriter::load(
        pqxx::dbtransaction& tx,
        const std::string& table,
        std::shared_ptr<TAccumulator> dataDimension) {

        auto records = dataDimension->records();
        if (!records.empty()) {
            auto columns = records[0].header(*_classifierNames);
            boost::replace_first(columns, "\n", "");
            MOJA_LOG_INFO << (boost::format("Loading %1% (%2%)") % table % columns).str();
            auto baseStmt = "INSERT INTO %1% (%2%) VALUES (%3%)";
            std::vector<std::string> batch;
            int batchRecords = 0;
            for (auto& record : records) {
                if (batchRecords == 100000) {
                    auto insertStmt = (boost::format(baseStmt) % table % columns % boost::join(batch, "),(")).str();
                    batch.clear();
                    batchRecords = 0;
                    tx.exec(insertStmt);
                }

                batch.push_back(record.asPersistable(false));
                batchRecords++;
            }

            if (!batch.empty()) {
                auto insertStmt = (boost::format(baseStmt) % table % columns % boost::join(batch, "),(")).str();
                tx.exec(insertStmt);
            }
        }
    }

}}} // namespace moja::modules::cbm
