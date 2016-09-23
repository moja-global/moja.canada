#ifndef MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/recordaccumulatortbb.h"
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
namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorLandUnitData : public flint::ModuleBase {
    public:
        CBMAggregatorLandUnitData(
            std::shared_ptr<flint::RecordAccumulatorTBB<DateRow>> dateDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<PoolInfoRow>> poolInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<ClassifierSetRow>> classifierSetDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<LandClassRow>> landClassDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<TemporalLocationRow>> locationDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<ModuleInfoRow>> moduleInfoDimension,
            std::shared_ptr<std::set<std::string>> classifierNames,
            std::shared_ptr<flint::RecordAccumulatorTBB<PoolRow>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<FluxRow>> fluxDimension)
        : ModuleBase(),
          _dateDimension(dateDimension),
          _poolInfoDimension(poolInfoDimension),
          _classifierSetDimension(classifierSetDimension),
          _landClassDimension(landClassDimension),
          _locationDimension(locationDimension),
          _moduleInfoDimension(moduleInfoDimension),
          _classifierNames(classifierNames),
          _poolDimension(poolDimension),
          _fluxDimension(fluxDimension),
		  _landUnitArea(0), 
		  _locationId(0) {}

        virtual ~CBMAggregatorLandUnitData() = default;

        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void onLocalDomainInit() override;
        void onTimingInit() override;
        void onOutputStep() override;

    private:
        std::shared_ptr<flint::RecordAccumulatorTBB<DateRow>> _dateDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<PoolInfoRow>> _poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<ClassifierSetRow>> _classifierSetDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<LandClassRow>> _landClassDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<TemporalLocationRow>> _locationDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<ModuleInfoRow>> _moduleInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<PoolRow>> _poolDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<FluxRow>> _fluxDimension;
        std::shared_ptr<std::set<std::string>> _classifierNames;

        flint::IVariable* _classifierSet;
        flint::IVariable* _landClass;

        flint::SpatialLocationInfo::Ptr _spatialLocationInfo;
        double _landUnitArea;
        Int64 _locationId;
        bool _isPrimaryAggregator;

        Int64 getPoolId(flint::IPool::ConstPtr pool);
        Int64 recordLocation(bool isSpinup);
        void recordLandUnitData(bool isSpinup);
        void recordPoolsSet(Int64 locationId, bool isSpinup);
        void recordFluxSet(Int64 locationId);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORLANDUNITDATA_H_