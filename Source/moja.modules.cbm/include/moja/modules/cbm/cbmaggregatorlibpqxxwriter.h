#ifndef MOJA_MODULES_CBM_CBMAGGREGATORLIBPQXXWRITER_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORLIBPQXXWRITER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/flatrecord.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include <moja/flint/spatiallocationinfo.h>

#include <pqxx/pqxx>
#include <vector>

namespace moja {
namespace flint {
	template<class TPersistable, class TRecord>
	class RecordAccumulatorWithMutex2;
}

namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorLibPQXXWriter : public CBMModuleBase {
    public:
        CBMAggregatorLibPQXXWriter(
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatFluxRecord>> fluxDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatPoolRecord>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatErrorRecord>> errorDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatAgeAreaRecord>> ageDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatDisturbanceRecord>> disturbanceDimension,
            std::shared_ptr<std::vector<std::string>> classifierNames,
            bool isPrimary = false)
            : CBMModuleBase(),
              _fluxDimension(fluxDimension),
              _poolDimension(poolDimension),
              _errorDimension(errorDimension),
              _ageDimension(ageDimension),
              _disturbanceDimension(disturbanceDimension),
              _classifierNames(classifierNames),
              _isPrimaryAggregator(isPrimary),
              _dropSchema(true) {}

        virtual ~CBMAggregatorLibPQXXWriter() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void doSystemInit() override;
        void doLocalDomainInit() override;
        void doSystemShutdown() override;

    private:
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatFluxRecord>> _fluxDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatPoolRecord>> _poolDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatErrorRecord>> _errorDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatAgeAreaRecord>> _ageDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatDisturbanceRecord>> _disturbanceDimension;
        std::shared_ptr<std::vector<std::string>> _classifierNames;

        std::shared_ptr<const flint::SpatialLocationInfo> _spatialLocationInfo;

        std::string _connectionString;
        std::string _schema;
        Int64 _jobId;
        bool _isPrimaryAggregator;
        bool _dropSchema;

        template<typename TAccumulator>
        void load(pqxx::work& tx,
                  Int64 jobId,
                  const std::string& table,
                  std::shared_ptr<TAccumulator> dataDimension);

        void doIsolated(pqxx::connection_base& conn, std::string sql, bool optional = false);
        void doIsolated(pqxx::connection_base& conn, std::vector<std::string> sql, bool optional = false);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORLIBPQXXWRITER_H_
