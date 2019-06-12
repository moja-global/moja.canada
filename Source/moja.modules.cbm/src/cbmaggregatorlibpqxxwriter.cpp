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

        std::vector<std::string> ddl{
			(boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS ClassifierSetDimension (jobId BIGINT NOT NULL, id BIGINT, %1% VARCHAR, PRIMARY KEY (jobId, id))") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
			"CREATE UNLOGGED TABLE IF NOT EXISTS DateDimension (jobId BIGINT NOT NULL, id BIGINT, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT, PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS PoolDimension (id BIGINT PRIMARY KEY, poolName VARCHAR(255))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LandClassDimension (jobId BIGINT NOT NULL, id BIGINT, name VARCHAR(255), PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ModuleInfoDimension (jobId BIGINT NOT NULL, id BIGINT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), PRIMARY KEY (jobId, id))",
            "CREATE UNLOGGED TABLE IF NOT EXISTS AgeClassDimension (jobId BIGINT NOT NULL, id INTEGER, startAge INTEGER, endAge INTEGER, PRIMARY KEY (jobId, id))",
            "CREATE UNLOGGED TABLE IF NOT EXISTS LocationDimension (jobId BIGINT NOT NULL, id BIGINT, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT, PRIMARY KEY (jobId, id))",
            "CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceTypeDimension (jobId BIGINT NOT NULL, id BIGINT, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255), PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS DisturbanceDimension (jobId BIGINT NOT NULL, id BIGINT, locationDimId BIGINT, disturbanceTypeDimId BIGINT, preDistAgeClassDimId INTEGER, area FLOAT, PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Pools (jobId BIGINT NOT NULL, id BIGINT, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT, PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS Fluxes (jobId BIGINT NOT NULL, id BIGINT, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT, PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS ErrorDimension (jobId BIGINT NOT NULL, id BIGINT, module VARCHAR, error VARCHAR, PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS LocationErrorDimension (jobId BIGINT NOT NULL, id BIGINT, locationDimId BIGINT, errorDimId BIGINT, PRIMARY KEY (jobId, id))",
			"CREATE UNLOGGED TABLE IF NOT EXISTS AgeArea (jobId BIGINT NOT NULL, id BIGINT, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT, PRIMARY KEY (jobId, id))",
		};

        MOJA_LOG_INFO << "Creating results tables.";
        doIsolated(conn, ddl, true);

        std::vector<std::string> indexDdl{
            "CREATE INDEX IF NOT EXISTS disturbancedimension_disturbancetype_idx ON DisturbanceDimension (jobId, disturbanceTypeDimId)",
            "CREATE INDEX IF NOT EXISTS fluxes_pools_idx ON Fluxes (poolSrcDimid, poolDstDimId) INCLUDE (jobId)",
            "CREATE INDEX IF NOT EXISTS disturbance_fluxes_idx ON Fluxes (disturbanceDimId) WHERE disturbanceDimId IS NULL",
            "CREATE INDEX IF NOT EXISTS annual_process_fluxes_idx ON Fluxes (disturbanceDimId) WHERE disturbanceDimId IS NOT NULL"
            "CREATE INDEX IF NOT EXISTS disturbancedimension_location_idx ON DisturbanceDimension (jobId, locationDimId)"
            "CREATE INDEX IF NOT EXISTS fluxes_location_idx ON Fluxes (jobId, locationDimId)"
            "CREATE INDEX IF NOT EXISTS pools_location_idx ON Pools (jobId, locationDimId)"
            "CREATE INDEX IF NOT EXISTS locationerrordimension_location_idx ON LocationErrorDimension (jobId, locationDimId)"
            "CREATE INDEX IF NOT EXISTS agearea_location_idx ON AgeArea (jobId, locationDimId)"
        };

        std::vector<std::string> resultsTables{
            "ClassifierSetDimension", "DateDimension", "LandClassDimension", "ModuleInfoDimension",
            "AgeClassDimension", "LocationDimension", "DisturbanceTypeDimension", "DisturbanceDimension",
            "Pools", "Fluxes", "ErrorDimension", "LocationErrorDimension", "AgeArea"
        };
        
        for (auto table : resultsTables) {
            indexDdl.push_back((boost::format("CREATE INDEX IF NOT EXISTS idx_%1%_jobid ON %1% USING BRIN (jobid)") % table).str());
        }

        MOJA_LOG_INFO << "Creating additional results table indexes.";
        doIsolated(conn, indexDdl, true);

        perform([&conn, this] {
            work tx(conn);

            MOJA_LOG_INFO << "Loading PoolDimension";
            auto poolSql = "INSERT INTO PoolDimension VALUES (%1%, '%2%') ON CONFLICT (id) DO NOTHING";
            std::vector<std::string> poolInsertSql;
            for (const auto& row : _poolInfoDimension->getPersistableCollection()) {
                poolInsertSql.push_back((boost::format(poolSql) % to_string(row.get<0>()) % to_string(row.get<1>())).str());
            }

            for (auto stmt : poolInsertSql) {
                tx.exec(stmt);
            }

            tx.commit();
        });

        bool csetsPreviouslyLoaded = perform([&conn, this] {
            return !nontransaction(conn).exec((boost::format(
                "SELECT 1 FROM ClassifierSetDimension WHERE jobId = %1% LIMIT 1"
            ) % _jobId).str()).empty();
        });

        if (!csetsPreviouslyLoaded) {
            perform([&conn, this] {
                work tx(conn);

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

                tx.commit();
            });
        }

        load(conn, _jobId, "DateDimension",            _dateDimension);
		load(conn, _jobId, "LandClassDimension",       _landClassDimension);
		load(conn, _jobId, "ModuleInfoDimension",      _moduleInfoDimension);
        load(conn, _jobId, "AgeClassDimension",        _ageClassDimension);
        load(conn, _jobId, "LocationDimension",        _locationDimension);
        load(conn, _jobId, "DisturbanceTypeDimension", _disturbanceTypeDimension);
		load(conn, _jobId, "DisturbanceDimension",     _disturbanceDimension);
		load(conn, _jobId, "Pools",                    _poolDimension);
		load(conn, _jobId, "Fluxes",                   _fluxDimension);
		load(conn, _jobId, "ErrorDimension",           _errorDimension);
		load(conn, _jobId, "LocationErrorDimension",   _locationErrorDimension);
		load(conn, _jobId, "AgeArea",                  _ageAreaDimension);

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
        pqxx::connection_base& conn,
        Int64 jobId,
        const std::string& table,
        std::shared_ptr<TAccumulator> dataDimension) {

        bool resultsPreviouslyLoaded = perform([&conn, jobId] {
            return !nontransaction(conn).exec((boost::format(
                "SELECT 1 FROM %1% WHERE jobId = %2% LIMIT 1"
            ) % table % jobId).str()).empty();
        });

        if (resultsPreviouslyLoaded) {
            MOJA_LOG_INFO << table << " already loaded - skipping." << std::endl;
            return;
        }

        perform([&conn, jobId, table, dataDimension] {
            work tx(conn);

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
            tx.commit();
        });
    }

}}} // namespace moja::modules::cbm
