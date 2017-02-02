#ifndef MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/modulebase.h"
#include "moja/flint/ipool.h"
#include "moja/flint/spatiallocationinfo.h"
#include "moja/hash.h"

#include <Poco/Tuple.h>
#include <Poco/Data/Session.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <set>

namespace moja {
namespace flint {
	template<class TPersistable, class TRecord>
	class RecordAccumulatorWithMutex2;
}

namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorSQLiteWriter : public flint::ModuleBase {
    public:
        CBMAggregatorSQLiteWriter(
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<DateRow, DateRecord>> dateDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolInfoRow, PoolInfoRecord>> poolInfoDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<ClassifierSetRow, ClassifierSetRecord>> classifierSetDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<LandClassRow, LandClassRecord>> landClassDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<TemporalLocationRow, TemporalLocationRecord>> locationDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<ModuleInfoRow, ModuleInfoRecord>> moduleInfoDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceRow, DisturbanceRecord>> disturbanceDimension,
			std::shared_ptr<std::set<std::string>> classifierNames,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolRow, PoolRecord>> poolDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<FluxRow, FluxRecord>> fluxDimension,
			bool isPrimary = false)
        : ModuleBase(),
          _dateDimension(dateDimension),
          _poolInfoDimension(poolInfoDimension),
          _classifierSetDimension(classifierSetDimension),
          _landClassDimension(landClassDimension),
          _locationDimension(locationDimension),
          _moduleInfoDimension(moduleInfoDimension),
		  _disturbanceDimension(disturbanceDimension),
          _classifierNames(classifierNames),
          _poolDimension(poolDimension),
          _fluxDimension(fluxDimension),
          _isPrimaryAggregator(isPrimary) {}

        virtual ~CBMAggregatorSQLiteWriter() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

        void onSystemShutdown() override;

    private:
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<DateRow, DateRecord>> _dateDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolInfoRow, PoolInfoRecord>> _poolInfoDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<ClassifierSetRow, ClassifierSetRecord>> _classifierSetDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<LandClassRow, LandClassRecord>> _landClassDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<TemporalLocationRow, TemporalLocationRecord>> _locationDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<ModuleInfoRow, ModuleInfoRecord>> _moduleInfoDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolRow, PoolRecord>> _poolDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<FluxRow, FluxRecord>> _fluxDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceRow, DisturbanceRecord>> _disturbanceDimension;
		std::shared_ptr<std::set<std::string>> _classifierNames;

        std::string _dbName;
        bool _isPrimaryAggregator;

		void tryExecute(Poco::Data::Session& session,
						std::function<void(Poco::Data::Session&)> fn);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_
