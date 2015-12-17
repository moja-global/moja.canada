#ifndef MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_
#define MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_

#include "moja/flint/modulebase.h"
#include "moja/hash.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

    class CBMDistEventRef {
    public:
        CBMDistEventRef() = default;
        explicit CBMDistEventRef(const DynamicObject& row) :
            _disturbanceTypeId(row["disturbance_type_id"]),
            _year(row["year"]) {

            _transitionLandClass = row["transition_land_class"].convert<std::string>();
            if (_transitionLandClass != "") {
                _hasLandClassTransition = true;
            }
        }

        int disturbanceTypeId() const { return _disturbanceTypeId; }
        double year() const { return _year; }
        std::string transitionLandClass() const { return _transitionLandClass; }
        bool hasLandClassTransition() const { return _hasLandClassTransition; }

    private:
        int _disturbanceTypeId;
        int	_year;
        bool _hasLandClassTransition = false;
        std::string _transitionLandClass;
    };

    class CBMDistEventTransfer {
    public:
        typedef std::unique_ptr<CBMDistEventTransfer> UniquePtr;
        typedef std::shared_ptr<CBMDistEventTransfer> Ptr;

        CBMDistEventTransfer() = default;

        CBMDistEventTransfer(flint::ILandUnitDataWrapper& landUnitData, const DynamicObject& data) :
            _disturbanceMatrixId(data["disturbance_matrix_id"]),
            _disturbanceTypeId(data["disturbance_type_id"]),
            _spatialUnitId(data["spatial_unit_id"]),
            _sourcePool(landUnitData.getPool(data["source_pool_name"].convert<std::string>())), 
            _destPool(landUnitData.getPool(data["dest_pool_name"].convert<std::string>())),
            _proportion(data["proportion"]) { }

        int disturbanceMatrixId() const { return _disturbanceMatrixId; }
        int disturbanceTypeId() const { return _disturbanceTypeId; }
        int spatialUnitId() const { return _spatialUnitId; }
        flint::IPool::ConstPtr sourcePool() const { return _sourcePool; }
        flint::IPool::ConstPtr destPool() const { return _destPool; }
        double proportion() const { return _proportion; }

    private:
        int _disturbanceMatrixId;
        int _disturbanceTypeId;
        int _spatialUnitId;
        flint::IPool::ConstPtr _sourcePool;
        flint::IPool::ConstPtr _destPool;
        double _proportion;
    };

    class CBMDisturbanceEventModule : public flint::ModuleBase {
    public:
        CBMDisturbanceEventModule() : ModuleBase() {}
        virtual ~CBMDisturbanceEventModule() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes ModuleType() { return flint::ModuleTypes::DisturbanceEvent; };

        virtual void onSystemInit(const flint::SystemInitNotification::Ptr&) override;
        virtual void onTimingInit(const flint::TimingInitNotification::Ptr&) override;
        virtual void onTimingStep(const flint::TimingStepNotification::Ptr&) override;

    private:
        typedef std::tuple<int, int> EventMapKey;
        typedef std::vector<CBMDistEventTransfer::Ptr> EventVector;
        typedef std::unordered_map<EventMapKey, EventVector, moja::Hash> EventMap;

        std::vector<std::string> _layerNames;
        std::vector<const flint::IVariable*> _layers;
        flint::IVariable* _landClass;
        EventMap _matrices;
        std::vector<CBMDistEventRef> _landUnitEvents;
        int _spu;

        flint::IPool::ConstPtr _softwoodMerch;
        flint::IPool::ConstPtr _softwoodOther;
        flint::IPool::ConstPtr _softwoodFoliage;
        flint::IPool::ConstPtr _softwoodCoarseRoots;
        flint::IPool::ConstPtr _softwoodFineRoots;

        flint::IPool::ConstPtr _hardwoodMerch;
        flint::IPool::ConstPtr _hardwoodOther;
        flint::IPool::ConstPtr _hardwoodFoliage;
        flint::IPool::ConstPtr _hardwoodCoarseRoots;
        flint::IPool::ConstPtr _hardwoodFineRoots;
    };


}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_