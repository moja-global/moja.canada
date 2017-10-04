#ifndef MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include <Poco/Data/Session.h>
#include <vector>

namespace moja {
namespace flint {
	template<class TPersistable, class TRecord>
	class RecordAccumulatorWithMutex2;
}

namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorSQLiteWriter : public CBMModuleBase {
    public:
        CBMAggregatorSQLiteWriter(
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<DateRow, DateRecord>> dateDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolInfoRow, PoolInfoRecord>> poolInfoDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<ClassifierSetRow, ClassifierSetRecord>> classifierSetDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<LandClassRow, LandClassRecord>> landClassDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<TemporalLocationRow, TemporalLocationRecord>> locationDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<ModuleInfoRow, ModuleInfoRecord>> moduleInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceTypeRow, DisturbanceTypeRecord>> disturbanceTypeDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceRow, DisturbanceRecord>> disturbanceDimension,
			std::shared_ptr<std::vector<std::string>> classifierNames,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolRow, PoolRecord>> poolDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<FluxRow, FluxRecord>> fluxDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<AgeClassRow, AgeClassRecord>> ageClassDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<AgeAreaRow, AgeAreaRecord>> ageAreaDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<ErrorRow, ErrorRecord>> errorDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<LocationErrorRow, LocationErrorRecord>> locationErrorDimension,
			bool isPrimary = false)
        : CBMModuleBase(),
          _dateDimension(dateDimension),
          _poolInfoDimension(poolInfoDimension),
          _classifierSetDimension(classifierSetDimension),
          _landClassDimension(landClassDimension),
          _locationDimension(locationDimension),
          _moduleInfoDimension(moduleInfoDimension),
          _disturbanceTypeDimension(disturbanceTypeDimension),
		  _disturbanceDimension(disturbanceDimension),
          _classifierNames(classifierNames),
          _poolDimension(poolDimension),
          _fluxDimension(fluxDimension),
		  _ageClassDimension(ageClassDimension),
		  _ageAreaDimension(ageAreaDimension),
		  _errorDimension(errorDimension),
		  _locationErrorDimension(locationErrorDimension),
          _isPrimaryAggregator(isPrimary) {}

        virtual ~CBMAggregatorSQLiteWriter() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void doSystemInit() override;
        void doSystemShutdown() override;

    private:
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<DateRow, DateRecord>> _dateDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolInfoRow, PoolInfoRecord>> _poolInfoDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<ClassifierSetRow, ClassifierSetRecord>> _classifierSetDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<LandClassRow, LandClassRecord>> _landClassDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<TemporalLocationRow, TemporalLocationRecord>> _locationDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<ModuleInfoRow, ModuleInfoRecord>> _moduleInfoDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolRow, PoolRecord>> _poolDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<FluxRow, FluxRecord>> _fluxDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<AgeClassRow, AgeClassRecord>> _ageClassDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<AgeAreaRow, AgeAreaRecord>> _ageAreaDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceTypeRow, DisturbanceTypeRecord>> _disturbanceTypeDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceRow, DisturbanceRecord>> _disturbanceDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<ErrorRow, ErrorRecord>> _errorDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<LocationErrorRow, LocationErrorRecord>> _locationErrorDimension;
		std::shared_ptr<std::vector<std::string>> _classifierNames;

        std::string _dbName;
        bool _isPrimaryAggregator;

		template<typename TAccumulator>
		void load(Poco::Data::Session& session,
				  const std::string& table,
				  std::shared_ptr<TAccumulator> dataDimension);

		static void tryExecute(Poco::Data::Session& session,
							   std::function<void(Poco::Data::Session&)> fn);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_
