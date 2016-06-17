#ifndef MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/recordaccumulatortbb.h"
#include "moja/flint/modulebase.h"
#include "moja/flint/spatiallocationinfo.h"
#include "moja/notification.h"
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

    class CBM_API CBMAggregatorPoolSQLite : public flint::ModuleBase {
    public:
        CBMAggregatorPoolSQLite(
            std::shared_ptr<flint::RecordAccumulatorTBB<DateRow>> dateDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<PoolInfoRow>> poolInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<ClassifierSetRow>> classifierSetDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<LocationRow>> locationDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<PoolRow>> poolDimension,
            std::shared_ptr<std::set<std::string>> classifierNames)
                : ModuleBase(),
                  _dateDimension(dateDimension),
                  _poolInfoDimension(poolInfoDimension),
                  _classifierSetDimension(classifierSetDimension),
                  _locationDimension(locationDimension),
                  _poolDimension(poolDimension),
                  _classifierNames(classifierNames) {}

        virtual ~CBMAggregatorPoolSQLite() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;	

        void onLocalDomainInit() override;
        void onSystemShutdown() override;
        void onOutputStep() override;
        void onTimingInit() override;
                
    private:
        std::shared_ptr<flint::RecordAccumulatorTBB<DateRow>> _dateDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<PoolInfoRow>> _poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<ClassifierSetRow>> _classifierSetDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<LocationRow>> _locationDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<PoolRow>> _poolDimension;
        std::shared_ptr<std::set<std::string>> _classifierNames;

        flint::SpatialLocationInfo::Ptr _spatialLocationInfo;
        Int64 _locationId;
        std::string _dbName;
        double _landUnitArea;

        void recordPoolsSet(bool isSpinup);
    };

}}} // namespace moja::Modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_