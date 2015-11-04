#ifndef MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/recordaccumulator.h"
#include "moja/flint/modulebase.h"
#include "moja/notification.h"
#include "moja/hash.h"

#include <Poco/Tuple.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorPoolSQLite : public flint::ModuleBase {
    public:
        CBMAggregatorPoolSQLite(
            std::shared_ptr<RecordAccumulator<DateRow>> dateDimension,
            std::shared_ptr<RecordAccumulator<PoolInfoRow>> poolInfoDimension,
            std::shared_ptr<RecordAccumulator<ClassifierSetRow>> classifierSetDimension,
            std::shared_ptr<RecordAccumulator<LocationRow>> locationDimension)
                : ModuleBase(),
                  _dateDimension(dateDimension),
                  _poolInfoDimension(poolInfoDimension),
                  _classifierSetDimension(classifierSetDimension),
                  _locationDimension(locationDimension) {}

        virtual ~CBMAggregatorPoolSQLite() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;	

        void onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& n) override;
        void onOutputStep(const flint::OutputStepNotification::Ptr&) override;
        void onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n) override;
                
    private:
        std::shared_ptr<RecordAccumulator<DateRow>> _dateDimension;
        std::shared_ptr<RecordAccumulator<PoolInfoRow>> _poolInfoDimension;
        std::shared_ptr<RecordAccumulator<ClassifierSetRow>> _classifierSetDimension;
        std::shared_ptr<RecordAccumulator<LocationRow>> _locationDimension;

        RecordAccumulator<PoolRow> _poolDimension;

        Int64 _locationId;
        std::string _dbName;
        std::vector<std::string> _classifierNames;
        double _landUnitArea;

        void recordPoolsSet(bool isSpinup);
    };

}}} // namespace moja::Modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORPOOLSQLITE_H_