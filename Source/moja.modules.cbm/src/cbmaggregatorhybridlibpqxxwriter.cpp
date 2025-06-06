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
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

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

        connection conn(_chConnectionString);

        if (checkCompleted(conn)) {
            MOJA_LOG_INFO << "Results previously loaded for jobId " << _jobId << " - skipping.";
            return;
        }

        {
            std::string lockName = (boost::format("%1%.lock") % _jobId).str();
            boost::interprocess::file_lock lock{ lockName.c_str() };
            boost::interprocess::scoped_lock scopedLock{ lock };

            perform([&conn, this] {
                if (checkCompleted(conn)) {
                    MOJA_LOG_INFO << "Results previously loaded for jobId " << _jobId << " - skipping.";
                    return;
                }

                work tx(conn);
                tx.exec((boost::format("INSERT INTO %1%.completed_jobs VALUES (%2%);") % _schema % _jobId).str());
                load(tx, (boost::format("%1%.raw_fluxes") % _schema).str(), _fluxDimension);
                load(tx, (boost::format("%1%.raw_pools") % _schema).str(), _poolDimension);
                load(tx, (boost::format("%1%.raw_errors") % _schema).str(), _errorDimension);
                load(tx, (boost::format("%1%.raw_ages") % _schema).str(), _ageDimension);
                load(tx, (boost::format("%1%.raw_disturbances") % _schema).str(), _disturbanceDimension);
                tx.commit();
            });
        }

        MOJA_LOG_INFO << "Insert complete." << std::endl;
    }

    bool CBMAggregatorHybridLibPQXXWriter::checkCompleted(pqxx::connection_base& conn) {
        return perform([&conn, this] {
            work tx(conn);
            row result = tx.exec1((boost::format(
                "SELECT EXISTS (SELECT * FROM %1%.completed_jobs WHERE id = %2%);"
            ) % _schema % _jobId).str());

            tx.commit();
            return result[0].as<int>() == 1;
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
