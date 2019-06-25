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
            doIsolated(conn, (boost::format("DROP SCHEMA %1% CASCADE;") % _schema).str(), true);
        }

        doIsolated(conn, (boost::format("CREATE SCHEMA %1%;") % _schema).str(), true);
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
        doIsolated(conn, (boost::format("SET search_path = %1%;") % _schema).str());

        std::vector<std::string> ddl{
            "CREATE UNLOGGED TABLE IF NOT EXISTS CompletedJobs (id BIGINT PRIMARY KEY);",
            "CREATE UNLOGGED TABLE IF NOT EXISTS PoolDimension (id BIGINT PRIMARY KEY, poolName VARCHAR(255));",
            (boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS Fluxes (%1% VARCHAR, year INTEGER, landClass VARCHAR, ageClass VARCHAR, totalArea FLOAT, disturbedArea FLOAT, disturbanceType VARCHAR, disturbanceCode INTEGER, poolSrcDimId INTEGER, poolDstDimId INTEGER, fluxValue FLOAT);") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
            (boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS Pools (%1% VARCHAR, year INTEGER, landClass VARCHAR, ageClass VARCHAR, totalArea FLOAT, poolDimId INTEGER, poolValue FLOAT);") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
            (boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS AgeArea (%1% VARCHAR, year INTEGER, landClass VARCHAR, ageClass VARCHAR, totalArea FLOAT);") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
            (boost::format("CREATE UNLOGGED TABLE IF NOT EXISTS Disturbances (%1% VARCHAR, year INTEGER, landClass VARCHAR, ageClass VARCHAR, preDistAgeClass VARCHAR, disturbanceType VARCHAR, disturbanceCode INTEGER, disturbedArea FLOAT, disturbedCarbon FLOAT);") % boost::join(*_classifierNames, " VARCHAR, ")).str(),
            (boost::format("CREATE UNIQUE INDEX IF NOT EXISTS u_fluxes ON Fluxes (%1%, year, landClass, COALESCE(ageClass, ''), poolSrcDimId, poolDstDimId, COALESCE(disturbanceType, ''), COALESCE(disturbanceCode, 0));") % boost::join(*_classifierNames, ", ")).str(),
            (boost::format("CREATE UNIQUE INDEX IF NOT EXISTS u_pools ON Pools (%1%, year, landClass, COALESCE(ageClass, ''), poolDimId);") % boost::join(*_classifierNames, ", ")).str(),
            (boost::format("CREATE UNIQUE INDEX IF NOT EXISTS u_agearea ON AgeArea (%1%, year, landClass, COALESCE(ageClass, ''));") % boost::join(*_classifierNames, ", ")).str(),
            (boost::format("CREATE UNIQUE INDEX IF NOT EXISTS u_disturbances ON Disturbances (%1%, year, landClass, COALESCE(ageClass, ''), COALESCE(preDistAgeClass, ''), disturbanceType, COALESCE(disturbanceCode, 0));") % boost::join(*_classifierNames, ", ")).str(),
        };

        MOJA_LOG_INFO << "Creating results tables.";
        doIsolated(conn, ddl, false);

        std::vector<std::string> indexDdl{
            "CREATE INDEX IF NOT EXISTS disturbancedimension_disturbancetype_idx ON Disturbances (disturbanceType);",
            "CREATE INDEX IF NOT EXISTS fluxes_pools_idx ON Fluxes (poolSrcDimid, poolDstDimId);",
            "CREATE INDEX IF NOT EXISTS disturbance_fluxes_idx ON Fluxes (disturbanceType) WHERE disturbanceType IS NULL;",
            "CREATE INDEX IF NOT EXISTS annual_process_fluxes_idx ON Fluxes (disturbanceType) WHERE disturbanceType IS NOT NULL;",
            "CREATE INDEX IF NOT EXISTS disturbancedimension_year_idx ON Disturbances (year);",
            "CREATE INDEX IF NOT EXISTS fluxes_year_idx ON Fluxes (year);",
            "CREATE INDEX IF NOT EXISTS pools_year_idx ON Pools (year);",
            "CREATE INDEX IF NOT EXISTS agearea_year_idx ON AgeArea (year);"
        };

        MOJA_LOG_INFO << "Creating additional results table indexes.";
        doIsolated(conn, indexDdl, true);

        perform([&conn, this] {
            work tx(conn);

            MOJA_LOG_INFO << "Loading PoolDimension";
            auto poolSql = "INSERT INTO PoolDimension VALUES (%1%, '%2%') ON CONFLICT (id) DO NOTHING;";
            std::vector<std::string> poolInsertSql;
            for (const auto& row : _poolInfoDimension->getPersistableCollection()) {
                poolInsertSql.push_back((boost::format(poolSql) % to_string(row.get<0>()) % to_string(row.get<1>())).str());
            }

            for (auto stmt : poolInsertSql) {
                tx.exec(stmt);
            }

            tx.commit();
        });

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

            // Bulk load the job results into a temporary set of relational tables.
            std::vector<std::string> tempTableDdl{
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS ClassifierSetDimension_%1% (id BIGINT, %2% VARCHAR, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId % boost::join(*_classifierNames, " VARCHAR, ")).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS DateDimension_%1% (id BIGINT, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS LandClassDimension_%1% (id BIGINT, name VARCHAR(255), PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS ModuleInfoDimension_%1% (id BIGINT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS AgeClassDimension_%1% (id INTEGER, startAge INTEGER, endAge INTEGER, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS LocationDimension_%1% (id BIGINT, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS DisturbanceTypeDimension_%1% (id BIGINT, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255), PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS DisturbanceDimension_%1% (id BIGINT, locationDimId BIGINT, disturbanceTypeDimId BIGINT, preDistAgeClassDimId INTEGER, area FLOAT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS Pools_%1% (id BIGINT, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS Fluxes_%1% (id BIGINT, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS ErrorDimension_%1% (id BIGINT, module VARCHAR, error VARCHAR, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS LocationErrorDimension_%1% (id BIGINT, locationDimId BIGINT, errorDimId BIGINT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str(),
                (boost::format("CREATE TEMPORARY TABLE IF NOT EXISTS AgeArea_%1% (id BIGINT, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT, PRIMARY KEY (id)) ON COMMIT DROP;") % _jobId).str()
            };

            for (const auto& ddl : tempTableDdl) {
                tx.exec(ddl);
            }

            MOJA_LOG_INFO << "Loading ClassifierSetDimension";
            auto classifierCount = _classifierNames->size();
            auto csetSql = boost::format("INSERT INTO ClassifierSetDimension_%1% VALUES (%2%, %3%);");
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

            load(tx, _jobId, "DateDimension", _dateDimension);
            load(tx, _jobId, "LandClassDimension", _landClassDimension);
            load(tx, _jobId, "ModuleInfoDimension", _moduleInfoDimension);
            load(tx, _jobId, "AgeClassDimension", _ageClassDimension);
            load(tx, _jobId, "LocationDimension", _locationDimension);
            load(tx, _jobId, "DisturbanceTypeDimension", _disturbanceTypeDimension);
            load(tx, _jobId, "DisturbanceDimension", _disturbanceDimension);
            load(tx, _jobId, "Pools", _poolDimension);
            load(tx, _jobId, "Fluxes", _fluxDimension);
            load(tx, _jobId, "ErrorDimension", _errorDimension);
            load(tx, _jobId, "LocationErrorDimension", _locationErrorDimension);
            load(tx, _jobId, "AgeArea", _ageAreaDimension);

            // Finally, merge the temporary job results into a denormalized set of tables for the whole simulation.
            MOJA_LOG_INFO << "Merging results: Fluxes";
            tx.exec((boost::format(R"(
                INSERT INTO Fluxes
                SELECT
                    %1%,
                    year,
                    lc.name AS landClass,
                    CASE
                        WHEN ac.startAge = -1 THEN 'N/A'
                        WHEN ac.endAge = -1 THEN ac.startAge || '+'
                        ELSE ac.startAge || '-' || ac.endAge
                    END AS ageClass,
                    l.area AS totalArea,
                    SUM(d.area) AS disturbedArea,
                    disturbanceTypeName AS disturbanceType,
                    disturbanceType AS disturbanceCode,
                    poolSrcDimId,
                    poolDstDimId,
                    SUM(fluxValue)
                FROM fluxes_%2% f
                INNER JOIN locationdimension_%2% l
                    ON f.locationdimid = l.id
                INNER JOIN datedimension_%2% dt
                    ON l.datedimid = dt.id
                INNER JOIN classifiersetdimension_%2% c
                    ON l.classifiersetdimid = c.id
                INNER JOIN landclassdimension_%2% lc
                    ON l.landclassdimid = lc.id
                LEFT JOIN ageclassdimension_%2% ac
                    ON l.ageclassdimid = ac.id
                LEFT JOIN disturbancedimension_%2% d
                    ON f.disturbancedimid = d.id
                LEFT JOIN disturbancetypedimension_%2% dtd
                    ON d.disturbancetypedimid = dtd.id
                GROUP BY
                    %1%,
                    year,
                    lc.name,
                    ac.startAge, ac.endAge,
                    l.area,
                    disturbanceTypeName,
                    disturbanceType,
                    poolSrcDimId,
                    poolDstDimId
                ON CONFLICT (%1%, year, landClass, COALESCE(ageClass, ''), poolSrcDimId, poolDstDimId, COALESCE(disturbanceType, ''), COALESCE(disturbanceCode, 0)) DO UPDATE
                    SET totalArea     = fluxes.totalArea     + EXCLUDED.totalArea,
                        disturbedArea = fluxes.disturbedArea + EXCLUDED.disturbedArea,
                        fluxValue     = fluxes.fluxValue     + EXCLUDED.fluxValue;
            )") % boost::join(*_classifierNames, ", ")  % _jobId).str());

            MOJA_LOG_INFO << "Merging results: Pools";
            tx.exec((boost::format(R"(
                INSERT INTO Pools
                SELECT
                    %1%,
                    year,
                    lc.name AS landClass,
                    CASE
                        WHEN ac.startAge = -1 THEN 'N/A'
                        WHEN ac.endAge = -1 THEN ac.startAge || '+'
                        ELSE ac.startAge || '-' || ac.endAge
                    END AS ageClass,
                    l.area AS totalArea,
                    poolId AS poolDimId,
                    SUM(poolValue)
                FROM pools_%2% p
                INNER JOIN locationdimension_%2% l
                    ON p.locationdimid = l.id
                INNER JOIN datedimension_%2% dt
                    ON l.datedimid = dt.id
                INNER JOIN classifiersetdimension_%2% c
                    ON l.classifiersetdimid = c.id
                INNER JOIN landclassdimension_%2% lc
                    ON l.landclassdimid = lc.id
                LEFT JOIN ageclassdimension_%2% ac
                    ON l.ageclassdimid = ac.id
                GROUP BY
                    %1%,
                    year,
                    lc.name,
                    ac.startAge, ac.endAge,
                    l.area,
                    poolId
                ON CONFLICT (%1%, year, landClass, COALESCE(ageClass, ''), poolDimId) DO UPDATE
                    SET totalArea = pools.totalArea + EXCLUDED.totalArea,
                        poolValue = pools.poolValue + EXCLUDED.poolValue;
            )") % boost::join(*_classifierNames, ", ") % _jobId).str());

            MOJA_LOG_INFO << "Merging results: AgeArea";
            tx.exec((boost::format(R"(
                INSERT INTO AgeArea
                SELECT
                    %1%,
                    year,
                    lc.name AS landClass,
                    CASE
                        WHEN ac.startAge = -1 THEN 'N/A'
                        WHEN ac.endAge = -1 THEN ac.startAge || '+'
                        ELSE ac.startAge || '-' || ac.endAge
                    END AS ageClass,
                    l.area AS totalArea
                FROM agearea_%2% a
                INNER JOIN locationdimension_%2% l
                    ON a.locationdimid = l.id
                INNER JOIN datedimension_%2% dt
                    ON l.datedimid = dt.id
                INNER JOIN classifiersetdimension_%2% c
                    ON l.classifiersetdimid = c.id
                INNER JOIN landclassdimension_%2% lc
                    ON l.landclassdimid = lc.id
                LEFT JOIN ageclassdimension_%2% ac
                    ON l.ageclassdimid = ac.id
                GROUP BY
                    %1%,
                    year,
                    lc.name,
                    ac.startAge, ac.endAge,
                    l.area
                ON CONFLICT (%1%, year, landClass, COALESCE(ageClass, '')) DO UPDATE
                    SET totalArea = AgeArea.totalArea + EXCLUDED.totalArea;
            )") % boost::join(*_classifierNames, ", ") % _jobId).str());

            MOJA_LOG_INFO << "Merging results: Disturbances";
            tx.exec((boost::format(R"(
                INSERT INTO Disturbances
                SELECT
                    %1%,
                    year,
                    lc.name AS landClass,
                    CASE
                        WHEN ac.startAge = -1 THEN 'N/A'
                        WHEN ac.endAge = -1 THEN ac.startAge || '+'
                        ELSE ac.startAge || '-' || ac.endAge
                    END AS ageClass,
                    CASE
                        WHEN ac_pre.startAge = -1 THEN 'N/A'
                        WHEN ac_pre.endAge = -1 THEN ac_pre.startAge || '+'
                        ELSE ac_pre.startAge || '-' || ac_pre.endAge
                    END AS preDistAgeClass,
                    disturbanceTypeName AS disturbanceType,
                    disturbanceType AS disturbanceCode,
                    d.area AS disturbedArea,
                    SUM(f.fluxvalue) AS disturbedCarbon
                FROM disturbancedimension_%2% d
                INNER JOIN locationdimension_%2% l
                    ON d.locationdimid = l.id
                INNER JOIN datedimension_%2% dt
                    ON l.datedimid = dt.id
                INNER JOIN classifiersetdimension_%2% c
                    ON l.classifiersetdimid = c.id
                INNER JOIN landclassdimension_%2% lc
                    ON l.landclassdimid = lc.id
                LEFT JOIN ageclassdimension_%2% ac_pre
                    ON d.predistageclassdimid = ac_pre.id
                LEFT JOIN ageclassdimension_%2% ac
                    ON l.ageclassdimid = ac.id
                LEFT JOIN disturbancetypedimension_%2% dtd
                    ON d.disturbancetypedimid = dtd.id
                LEFT JOIN fluxes_%2% f
                    ON d.id = f.disturbancedimid
                GROUP BY
                    %1%,
                    year,
                    lc.name,
                    ac.startAge, ac.endAge,
                    ac_pre.startAge, ac_pre.endAge,
                    d.area,
                    disturbanceTypeName,
                    disturbanceType
                ON CONFLICT (%1%, year, landClass, COALESCE(ageClass, ''), COALESCE(preDistAgeClass, ''), disturbanceType, COALESCE(disturbanceCode, 0)) DO UPDATE
                    SET disturbedArea   = Disturbances.disturbedArea   + EXCLUDED.disturbedArea,
                        disturbedCarbon = Disturbances.disturbedCarbon + EXCLUDED.disturbedCarbon;
            )") % boost::join(*_classifierNames, ", ") % _jobId).str());

            tx.commit();
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
        Int64 jobId,
        const std::string& table,
        std::shared_ptr<TAccumulator> dataDimension) {

        MOJA_LOG_INFO << (boost::format("Loading %1%") % table).str();
        auto tempTableName = (boost::format("%1%_%2%") % table % jobId).str();
        pqxx::stream_to stream(tx, tempTableName);
        auto records = dataDimension->records();
        if (!records.empty()) {
            for (auto& record : records) {
                stream << record.asTuple();
            }
        }
            
        stream.complete();
    }

}}} // namespace moja::modules::cbm
