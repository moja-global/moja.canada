#ifndef MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_

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

    class CBM_API CBMAggregatorFluxSQLite : public flint::ModuleBase {
    public:
        CBMAggregatorFluxSQLite(
            std::shared_ptr<RecordAccumulator<DateRow>> dateDimension,
            std::shared_ptr<RecordAccumulator<PoolInfoRow>> poolInfoDimension,
            std::shared_ptr<RecordAccumulator<ClassifierSetRow>> classifierSetDimension)
                : ModuleBase(),
                  _dateDimension(dateDimension),
                  _poolInfoDimension(poolInfoDimension),
                  _classifierSetDimension(classifierSetDimension) {}

        virtual ~CBMAggregatorFluxSQLite() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes ModuleType() { return flint::ModuleTypes::System; };

        void onLocalDomainShutdown(const flint::LocalDomainShutdownNotification::Ptr& n) override;
        void onTimingInit(const flint::TimingInitNotification::Ptr& n) override;
        void onOutputStep(const flint::OutputStepNotification::Ptr&) override;

    private:
        std::shared_ptr<RecordAccumulator<DateRow>> _dateDimension;
        std::shared_ptr<RecordAccumulator<PoolInfoRow>> _poolInfoDimension;
        std::shared_ptr<RecordAccumulator<ClassifierSetRow>> _classifierSetDimension;

        RecordAccumulator<FluxRow> _fluxDimension;
        RecordAccumulator<ModuleInfoRow> _moduleInfoDimension;

        double _landUnitArea;
        std::string _dbName;
        std::vector<std::string> _classifierNames;
        Int64 _classifierSetRecordId;

        void recordFluxSet();
    };

}}} // namespace moja::Modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_
