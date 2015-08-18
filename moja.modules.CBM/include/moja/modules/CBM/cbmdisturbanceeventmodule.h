#ifndef CBMDisturbanceEventModule_H_
#define CBMDisturbanceEventModule_H_

#include "moja/flint/modulebase.h"
#include "moja/hash.h"
#include <unordered_map>

namespace moja {
	namespace modules {
		namespace CBM {

			class CBMDistEventRef {
			public:
				CBMDistEventRef() = default;
				explicit CBMDistEventRef(const DynamicObject& row) :
					_inventory_id(row["spatial_unit_id"]),
					_spatial_unit_id(row["inventory_id"]),
					_disturbance_type_id(row["disturbance_type_id"]),
					_t_year(row["t_year"]) { }

				int	inventory_id()			const { return _inventory_id; }
				int	spatial_unit_id()		const { return _spatial_unit_id; }
				int	disturbance_type_id()	const { return _disturbance_type_id; }
				double	t_year()			const { return _t_year; }

			private:
				int		_inventory_id;
				int		_spatial_unit_id;
				int		_disturbance_type_id;
				int		_t_year;
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
				const flint::IPool*	source_pool()			const { return _source_pool; }
				const flint::IPool*	dest_pool()				const { return _dest_pool; }
				double				proportion()			const { return _proportion; }

			private:
				int			      _disturbance_matrix_id;
				int			      _disturbance_type_id;
				int			      _spatial_unit_id;
				const flint::IPool* _source_pool;
				const flint::IPool* _dest_pool;
				double			  _proportion;
			};

			class CBMDisturbanceEventModule : public flint::ModuleBase {
			public:
				CBMDisturbanceEventModule() : ModuleBase() {}
				virtual ~CBMDisturbanceEventModule() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes ModuleType() { return flint::ModuleTypes::DisturbanceEvent; };

				virtual void onLocalDomainInit		(const flint::LocalDomainInitNotification::Ptr&)			override;
				virtual void onLocalDomainShutdown	(const flint::LocalDomainShutdownNotification::Ptr&)		override;
				virtual void onPreTimingSequence	(const flint::PreTimingSequenceNotification::Ptr& n)		override;
				virtual void onTimingInit			(const flint::TimingInitNotification::Ptr&)				override;
				virtual void onPostTimingInit		(const flint::TimingPostInitNotification::Ptr&)			override;
				virtual void onTimingShutdown		(const flint::TimingShutdownNotification::Ptr&)			override;
				virtual void onTimingStep			(const flint::TimingStepNotification::Ptr&)				override;
				virtual void onTimingPreEndStep		(const flint::TimingPreEndStepNotification::Ptr&)			override;
				virtual void onTimingEndStep		(const flint::TimingEndStepNotification::Ptr&)			override;
				virtual void onTimingPostStep		(const flint::TimingPostStepNotification::Ptr&)			override;
				virtual void onOutputStep			(const flint::OutputStepNotification::Ptr&)				override;
				virtual void onDisturbanceEvent		(const flint::DisturbanceEventNotification::Ptr&)			override;
				virtual void onPostDisturbanceEvent	(const flint::PostDisturbanceEventNotification::Ptr& n)	override;
				virtual void onPostNotification		(const flint::PostNotificationNotification::Ptr&)			override;

			private:
				//typedef std::pair<int, int> event_map_key;
				typedef std::tuple<int, int> event_map_key;

				typedef std::vector<CBMDistEventTransfer::Ptr> event_vector;

				//typedef std::map<event_map_key, event_vector> event_map;
				typedef std::unordered_map<event_map_key, event_vector, hash_tuple::hash<event_map_key>> event_map;

				event_map _events;

				std::vector<CBMDistEventRef> _landUnitEvents;
			};

		}
	}
} // namespace moja::Modules::CBM
#endif // CBMDisturbanceEventModule_H_