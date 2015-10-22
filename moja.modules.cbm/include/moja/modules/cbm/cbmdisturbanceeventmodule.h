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
            _disturbance_type_id(row["disturbance_type_id"]),
            _year(row["year"]) { }

        int	disturbance_type_id() const { return _disturbance_type_id; }
        double year() const { return _year; }

    private:
        int	_disturbance_type_id;
        int	_year;
    };

    class CBMDistEventTransfer {
    public:
        typedef std::unique_ptr<CBMDistEventTransfer> UniquePtr;
        typedef std::shared_ptr<CBMDistEventTransfer> Ptr;

        CBMDistEventTransfer() = default;

        CBMDistEventTransfer(flint::ILandUnitDataWrapper& landUnitData, const DynamicObject& data) :
            _disturbance_matrix_id(data["disturbance_matrix_id"]),
            _disturbance_type_id(data["disturbance_type_id"]),
            _spatial_unit_id(data["spatial_unit_id"]),
            _source_pool(landUnitData.getPool(data["source_pool_name"].convert<std::string>())), 
            _dest_pool(landUnitData.getPool(data["dest_pool_name"].convert<std::string>())),
            _proportion(data["proportion"]) { }

        int					disturbance_matrix_id() const { return _disturbance_matrix_id; }
        int					disturbance_type_id()	const { return _disturbance_type_id; }
        int					spatial_unit_id()		const { return _spatial_unit_id; }
        flint::IPool::ConstPtr	source_pool()		const { return _source_pool; }
        flint::IPool::ConstPtr	dest_pool()			const { return _dest_pool; }
        double				proportion()			const { return _proportion; }

    private:
        int _disturbance_matrix_id;
        int _disturbance_type_id;
        int _spatial_unit_id;
        flint::IPool::ConstPtr _source_pool;
        flint::IPool::ConstPtr _dest_pool;
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
        virtual void onTimingInit (const flint::TimingInitNotification::Ptr&) override;
        virtual void onTimingStep (const flint::TimingStepNotification::Ptr&) override;

    private:
        typedef std::tuple<int, int> event_map_key;
        typedef std::vector<CBMDistEventTransfer::Ptr> event_vector;
        typedef std::unordered_map<event_map_key, event_vector, hash_tuple::hash<event_map_key>> event_map;

        std::vector<std::string> _layerNames;
        std::vector<const flint::IVariable*> _layers;
        event_map _matrices;
        std::vector<CBMDistEventRef> _landUnitEvents;
        int _spu;
    };


}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_