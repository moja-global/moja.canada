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

    void CBMAggregatorLibPQXXWriter::configure(const DynamicObject& config) {
        _connectionString = config["connection_string"].convert<std::string>();
        _schema = config["schema"].convert<std::string>();

        if (config.contains("drop_schema")) {
            _dropSchema = config["drop_schema"];
        }
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

        connection conn(_connectionString);
        if (_dropSchema) {
            doIsolated(conn, (boost::format("DROP SCHEMA %1% CASCADE") % _schema).str(), true);
        }

        doIsolated(conn, (boost::format("CREATE SCHEMA %1%") % _schema).str(), true);
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
            % _schema % _connectionString).str();

        connection conn(_connectionString);
        doIsolated(conn, (boost::format("SET search_path = %1%") % _schema).str());

        std::vector<std::string> basePartitionTables{
            "ClassifierSetDimension", "DateDimension", "LandClassDimension", "ModuleInfoDimension",
            "AgeClassDimension", "LocationDimension", "DisturbanceTypeDimension", "DisturbanceDimension",
            "Pools", "Fluxes", "ErrorDimension", "LocationErrorDimension", "AgeArea"
        };

        std::vector<std::string> ddl{
			(boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS ClassifierSetDimension (jobId BIGINT, id BIGINT, %1% VARCHAR) PARTITION BY LIST (jobId)") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE UNLOGGED TABLE IF NOT EXISTS DateDimension (jobId BIGINT, id BIGINT, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS PoolDimension (id BIGINT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LandClassDimension (jobId BIGINT, id BIGINT, name VARCHAR(255)) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ModuleInfoDimension (jobId BIGINT, id BIGINT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255)) PARTITION BY LIST (jobId)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS AgeClassDimension (jobId BIGINT, id INTEGER, startAge INTEGER, endAge INTEGER) PARTITION BY LIST (jobId)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS LocationDimension (jobId BIGINT, id BIGINT, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT) PARTITION BY LIST (jobId)",
            "CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceTypeDimension (jobId BIGINT, id BIGINT, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255)) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceDimension (jobId BIGINT, id BIGINT, locationDimId BIGINT, disturbanceTypeDimId BIGINT, preDistAgeClassDimId INTEGER, area FLOAT) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Pools (jobId BIGINT, id BIGINT, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Fluxes (jobId BIGINT, id BIGINT, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ErrorDimension (jobId BIGINT, id BIGINT, module VARCHAR, error VARCHAR) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LocationErrorDimension (jobId BIGINT, id BIGINT, locationDimId BIGINT, errorDimId BIGINT) PARTITION BY LIST (jobId)",
			"CREATE UNLOGGED TABLE IF NOT EXISTS AgeArea (jobId BIGINT, id BIGINT, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT) PARTITION BY LIST (jobId)",
		};

        doIsolated(conn, ddl, true);

        Int64 lock = moja::hash::hashCombine(_schema, _jobId);
        perform([&conn, this, &basePartitionTables, lock] {
            work tx(conn);

            // Load the results, guarded by a lock specific to the schema and job ID which uniquely identifies
            // the unit of work being run. The lock guarantees that output is not duplicated for multiple
            // simultaneous runs of the same tile or block.
            tx.exec((boost::format("SELECT pg_advisory_lock(%1%)") % lock).str());

            // Clean out any existing results in case this block already ran but was re-queued for some reason.
            std::vector<std::string> partitionDdl;
            for (auto table : basePartitionTables) {
                partitionDdl.push_back((boost::format("DROP TABLE IF EXISTS %1%_%2%") % table % _jobId).str());
                partitionDdl.push_back((boost::format("CREATE TABLE %1%_%2%") % table % _jobId).str());
            }

            for (auto stmt : partitionDdl) {
                tx.exec(stmt);
            }

            MOJA_LOG_INFO << "Loading PoolDimension";
            auto poolSql = "INSERT INTO PoolDimension VALUES (%1%, '%2%') ON CONFLICT (id) DO NOTHING";
            std::vector<std::string> poolInsertSql;
            for (const auto& row : _poolInfoDimension->getPersistableCollection()) {
                poolInsertSql.push_back((boost::format(poolSql) % to_string(row.get<0>()) % to_string(row.get<1>())).str());
            }

            for (auto stmt : poolInsertSql) {
                tx.exec(stmt);
            }

            MOJA_LOG_INFO << "Loading ClassifierSetDimension";
            auto classifierCount = _classifierNames->size();
            auto csetSql = boost::format("INSERT INTO ClassifierSetDimension VALUES (%1%, %2%, %3%)");
            std::vector<std::string> csetInsertSql;
            for (const auto& row : this->_classifierSetDimension->getPersistableCollection()) {
                std::vector<std::string> classifierValues;
                auto values = row.get<1>();
                for (int i = 0; i < classifierCount; i++) {
                    const auto& value = values[i];
                    classifierValues.push_back(value.isNull() ? "NULL" : (boost::format("'%1%'") % value.value()).str());
                }

                csetInsertSql.push_back((csetSql % this->_jobId % row.get<0>() % boost::join(classifierValues, ", ")).str());
            }

            for (auto stmt : csetInsertSql) {
                tx.exec(stmt);
            }

            load(tx, _jobId, "DateDimension",		     _dateDimension);
		    load(tx, _jobId, "LandClassDimension",       _landClassDimension);
		    load(tx, _jobId, "ModuleInfoDimension",      _moduleInfoDimension);
            load(tx, _jobId, "AgeClassDimension",        _ageClassDimension);
            load(tx, _jobId, "LocationDimension",	     _locationDimension);
            load(tx, _jobId, "DisturbanceTypeDimension", _disturbanceTypeDimension);
		    load(tx, _jobId, "DisturbanceDimension",     _disturbanceDimension);
		    load(tx, _jobId, "Pools",				     _poolDimension);
		    load(tx, _jobId, "Fluxes",				     _fluxDimension);
		    load(tx, _jobId, "ErrorDimension",		     _errorDimension);
		    load(tx, _jobId, "LocationErrorDimension",   _locationErrorDimension);
		    load(tx, _jobId, "AgeArea",				     _ageAreaDimension);

            tx.commit();
        });

        for (auto table : basePartitionTables) {
            std::vector<std::string> attachPartitionDdl;
            attachPartitionDdl.push_back((boost::format("ALTER TABLE %1%_%2% ADD CONSTRAINT part_%1%_%2% CHECK (jobid = %2%)" ) % table % _jobId).str());
            attachPartitionDdl.push_back((boost::format("ALTER TABLE %1% ATTACH PARTITION %1%_%2% FOR VALUES IN (%2%)") % table % _jobId).str());
            attachPartitionDdl.push_back((boost::format("ALTER TABLE %1%_%2% DROP CONSTRAINT part_%1%_%2%") % table% _jobId).str());
            doIsolated(conn, attachPartitionDdl, true);
        }

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
        work& tx,
        Int64 jobId,
        const std::string& table,
        std::shared_ptr<TAccumulator> dataDimension) {

        MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
        pqxx::stream_to stream(tx, table);
        auto records = dataDimension->records();
        if (!records.empty()) {
            for (auto& record : records) {
                auto recData = std::tuple_cat(std::make_tuple(jobId), record.asTuple());
                stream << recData;
            }
        }

        stream.complete();
    }

}}} // namespace moja::modules::cbm
