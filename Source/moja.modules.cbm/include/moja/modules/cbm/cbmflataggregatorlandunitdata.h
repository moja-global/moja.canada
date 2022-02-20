#ifndef MOJA_MODULES_CBM_CBMFLATAGGREGATORLANDUNITDATA_H_
#define MOJA_MODULES_CBM_CBMFLATAGGREGATORLANDUNITDATA_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/flatrecord.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/ageclasshelper.h"
#include "moja/flint/spatiallocationinfo.h"

#include <Poco/Mutex.h>

#include <vector>
#include <optional>

namespace moja {
namespace flint {
	template<class TPersistable, class TRecord>
	class RecordAccumulatorWithMutex2;
}

namespace modules {
namespace cbm {

    class CBM_API CBMFlatAggregatorLandUnitData : public CBMModuleBase {
    public:
        CBMFlatAggregatorLandUnitData(
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatFluxRecord>> fluxDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatPoolRecord>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatErrorRecord>> errorDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatAgeAreaRecord>> ageDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatDisturbanceRecord>> disturbanceDimension,
            std::shared_ptr<std::vector<std::string>> classifierNames,
            std::shared_ptr<Poco::Mutex> classifierNamesLock)
        : CBMModuleBase(),
          _fluxDimension(fluxDimension),
          _poolDimension(poolDimension),
          _errorDimension(errorDimension),
          _ageDimension(ageDimension),
          _disturbanceDimension(disturbanceDimension),
          _classifierNames(classifierNames),
          _classifierNamesLock(classifierNamesLock),
		  _landUnitArea(0),
          _previousAttributes() {}

        virtual ~CBMFlatAggregatorLandUnitData() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void doLocalDomainInit() override;
        void doTimingInit() override;
        void doOutputStep() override;
		void doError(std::string msg) override;

    private:
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatFluxRecord>> _fluxDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatPoolRecord>> _poolDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatErrorRecord>> _errorDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatAgeAreaRecord>> _ageDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatDisturbanceRecord>> _disturbanceDimension;
		std::shared_ptr<std::vector<std::string>> _classifierNames;
		std::shared_ptr<Poco::Mutex> _classifierNamesLock;

		flint::IVariable* _classifierSet;
        flint::IVariable* _landClass;

        std::shared_ptr<const flint::SpatialLocationInfo> _spatialLocationInfo;
        double _landUnitArea;
        std::optional<FlatAgeAreaRecord> _previousAttributes;
        bool _isPrimaryAggregator;
		std::string _classifierSetVar;
        AgeClassHelper _ageClassHelper;

        FlatAgeAreaRecord recordLocation(bool isSpinup);
        void recordLandUnitData(bool isSpinup);
        void recordPoolsSet(const FlatAgeAreaRecord& location);
        void recordFluxSet(const FlatAgeAreaRecord& location);
		void recordClassifierNames(const DynamicObject& classifierSet);
        bool hasDisturbanceInfo(std::shared_ptr<flint::IOperationResult> flux);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMFLATAGGREGATORLANDUNITDATA_H_
