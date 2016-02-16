#ifndef MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/recordaccumulator.h"
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
            std::shared_ptr<flint::RecordAccumulator<DateRow>> dateDimension,
            std::shared_ptr<flint::RecordAccumulator<PoolInfoRow>> poolInfoDimension,
            std::shared_ptr<flint::RecordAccumulator<ClassifierSetRow>> classifierSetDimension,
            std::shared_ptr<flint::RecordAccumulator<LocationRow>> locationDimension)
                : ModuleBase(),
                  _dateDimension(dateDimension),
                  _poolInfoDimension(poolInfoDimension),
                  _classifierSetDimension(classifierSetDimension),
                  _locationDimension(locationDimension), 
				  _landUnitArea(0), 
			      _locationId(0) {}

        virtual ~CBMAggregatorFluxSQLite() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes ModuleType() override { return flint::ModuleTypes::System; };

		void onLocalDomainInit() override;
		void onLocalDomainShutdown() override;
        void onTimingInit() override;
        void onOutputStep() override;

    private:
        std::shared_ptr<flint::RecordAccumulator<DateRow>> _dateDimension;
        std::shared_ptr<flint::RecordAccumulator<PoolInfoRow>> _poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulator<ClassifierSetRow>> _classifierSetDimension;
        std::shared_ptr<flint::RecordAccumulator<LocationRow>> _locationDimension;

        flint::RecordAccumulator<FluxRow> _fluxDimension;
        flint::RecordAccumulator<ModuleInfoRow> _moduleInfoDimension;

        double _landUnitArea;
        std::string _dbName;
        std::vector<std::string> _classifierNames;
        Int64 _locationId;

        void recordFluxSet();
    };

}}} // namespace moja::Modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORFLUXSQLITE_H_
