#ifndef MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/modulebase.h"
#include "moja/flint/ipool.h"
#include "moja/flint/spatiallocationinfo.h"
#include "moja/hash.h"

#include <Poco/Tuple.h>

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

    class CBM_API CBMAggregatorLandUnitData : public flint::ModuleBase {
    public:
        CBMAggregatorLandUnitData(
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<DateRow, DateRecord>> dateDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolInfoRow, PoolInfoRecord>> poolInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<ClassifierSetRow, ClassifierSetRecord>> classifierSetDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<LandClassRow, LandClassRecord>> landClassDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<TemporalLocationRow, TemporalLocationRecord>> locationDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<ModuleInfoRow, ModuleInfoRecord>> moduleInfoDimension,
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<DisturbanceRow, DisturbanceRecord>> disturbanceDimension,
            std::shared_ptr<std::set<std::string>> classifierNames,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<PoolRow, PoolRecord>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<FluxRow, FluxRecord>> fluxDimension)
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
		  _landUnitArea(0), 
		  _locationId(0) {}

        virtual ~CBMAggregatorLandUnitData() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void onLocalDomainInit() override;
        void onTimingInit() override;
        void onOutputStep() override;

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

        flint::IVariable* _classifierSet;
        flint::IVariable* _landClass;

        flint::SpatialLocationInfo::ConstPtr _spatialLocationInfo;
        double _landUnitArea;
        Int64 _locationId;
        bool _isPrimaryAggregator;
		std::string _classifierSetVar;

        Int64 getPoolId(flint::IPool::ConstPtr pool);
        Int64 recordLocation(bool isSpinup);
        void recordLandUnitData(bool isSpinup);
        void recordPoolsSet(Int64 locationId, bool isSpinup);
        void recordFluxSet(Int64 locationId);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_
