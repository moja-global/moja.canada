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
        };

        MOJA_LOG_INFO << "Creating results tables.";
        doIsolated(conn, ddl, false);

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

            // Add a record to the job tracking table for the merge results task.
            tx.exec((boost::format("INSERT INTO run_status (task_type, task_name) VALUES ('merge results', '%1%');") % _jobId).str());

            // Bulk load the job results into a temporary set of relational tables.
            std::vector<std::string> tempTableDdl{
                (boost::format("CREATE UNLOGGED TABLE ClassifierSetDimension_%1% (id BIGINT, %2% VARCHAR, PRIMARY KEY (id));") % _jobId % boost::join(*_classifierNames, " VARCHAR, ")).str(),
                (boost::format("CREATE UNLOGGED TABLE DateDimension_%1% (id BIGINT, step INTEGER, year INTEGER, month INTEGER, day INTEGER, fracOfStep FLOAT, lengthOfStepInYears FLOAT, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE LandClassDimension_%1% (id BIGINT, name VARCHAR(255), PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE ModuleInfoDimension_%1% (id BIGINT, libraryType INTEGER, libraryInfoId INTEGER, moduleType INTEGER, moduleId INTEGER, moduleName VARCHAR(255), PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE AgeClassDimension_%1% (id INTEGER, startAge INTEGER, endAge INTEGER, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE LocationDimension_%1% (id BIGINT, classifierSetDimId BIGINT, dateDimId BIGINT, landClassDimId BIGINT, ageClassDimId INT, area FLOAT, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE DisturbanceTypeDimension_%1% (id BIGINT, disturbanceType INTEGER, disturbanceTypeName VARCHAR(255), PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE DisturbanceDimension_%1% (id BIGINT, locationDimId BIGINT, disturbanceTypeDimId BIGINT, previousLocationDimId INTEGER, area FLOAT, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE Pools_%1% (id BIGINT, locationDimId BIGINT, poolId BIGINT, poolValue FLOAT, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE Fluxes_%1% (id BIGINT, locationDimId BIGINT, moduleInfoDimId BIGINT, disturbanceDimId BIGINT, poolSrcDimId BIGINT, poolDstDimId BIGINT, fluxValue FLOAT, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE ErrorDimension_%1% (id BIGINT, module VARCHAR, error VARCHAR, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE LocationErrorDimension_%1% (id BIGINT, locationDimId BIGINT, errorDimId BIGINT, PRIMARY KEY (id));") % _jobId).str(),
                (boost::format("CREATE UNLOGGED TABLE AgeArea_%1% (id BIGINT, locationDimId BIGINT, ageClassDimId INTEGER, area FLOAT, PRIMARY KEY (id));") % _jobId).str()
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
                    classifierValues.push_back(value.isNull() ? "NULL" : tx.quote(value.value()));
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
