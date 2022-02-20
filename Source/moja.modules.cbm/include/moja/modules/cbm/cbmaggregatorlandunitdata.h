#ifndef MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/ageclasshelper.h"
#include "moja/flint/spatiallocationinfo.h"

#include <Poco/Mutex.h>

#include <vector>

namespace moja {
namespace flint {
	template<class TPersistable, class TRecord>
	class RecordAccumulatorWithMutex2;
}

namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorLandUnitData : public CBMModuleBase {
    public:
        CBMAggregatorLandUnitData(
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<DateRow, DateRecord>> dateDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolInfoRow, PoolInfoRecord>> poolInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<ClassifierSetRow, ClassifierSetRecord>> classifierSetDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<LandClassRow, LandClassRecord>> landClassDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<TemporalLocationRow, TemporalLocationRecord>> locationDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<ModuleInfoRow, ModuleInfoRecord>> moduleInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceTypeRow, DisturbanceTypeRecord>> disturbanceTypeDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceRow, DisturbanceRecord>> disturbanceDimension,
            std::shared_ptr<std::vector<std::string>> classifierNames,
			std::shared_ptr<Poco::Mutex> classifierNamesLock,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolRow, PoolRecord>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<FluxRow, FluxRecord>> fluxDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<AgeClassRow, AgeClassRecord>> AgeClassDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<AgeAreaRow, AgeAreaRecord>> AgeAreaDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<ErrorRow, ErrorRecord>> errorDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<LocationErrorRow, LocationErrorRecord>> locationErrorDimension)
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
		  _classifierNamesLock(classifierNamesLock),
          _poolDimension(poolDimension),
          _fluxDimension(fluxDimension),
		  _ageClassDimension(AgeClassDimension),
		  _ageAreaDimension(AgeAreaDimension),
		  _errorDimension(errorDimension),
		  _locationErrorDimension(locationErrorDimension),
		  _landUnitArea(0),
          _previousLocationId(0) {}

        virtual ~CBMAggregatorLandUnitData() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void doLocalDomainInit() override;
        void doTimingInit() override;
        void doOutputStep() override;
		void doError(std::string msg) override;

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
		std::shared_ptr<Poco::Mutex> _classifierNamesLock;

		flint::IVariable* _classifierSet;
        flint::IVariable* _landClass;

        std::shared_ptr<const flint::SpatialLocationInfo> _spatialLocationInfo;
        double _landUnitArea;
        Int64 _previousLocationId;
        bool _isPrimaryAggregator;
		std::string _classifierSetVar;
        AgeClassHelper _ageClassHelper;

        Int64 getPoolId(const flint::IPool* pool);
        Int64 recordLocation(bool isSpinup);
        void recordLandUnitData(bool isSpinup);
        void recordPoolsSet(Int64 locationId);
        void recordFluxSet(Int64 locationId);
		void recordClassifierNames(const DynamicObject& classifierSet);
		void recordAgeArea(Int64 locationId);
		void recordAgeClass();
        bool hasDisturbanceInfo(std::shared_ptr<flint::IOperationResult> flux);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_
