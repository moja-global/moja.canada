#ifndef MOJA_MODULES_CBM_CBMDISTURBANCELISTENER_H_
#define MOJA_MODULES_CBM_CBMDISTURBANCELISTENER_H_

#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/hash.h"
#include "moja/flint/ivariable.h"

#include <unordered_map>
#include <unordered_set>

namespace moja {
namespace modules {
namespace cbm {
	
    class IDisturbanceCondition {
    public:
        virtual ~IDisturbanceCondition() = default;
        
        virtual bool check() const = 0;
    };

    enum class DisturbanceConditionType {
        LessThan,
        EqualTo,
        AtLeast
    };

    class VariableDisturbanceCondition : public IDisturbanceCondition {
    public:
        VariableDisturbanceCondition(flint::IVariable* var, DisturbanceConditionType type, const DynamicVar& target)
            : _var(var), _type(type), _target(target) { }
        
        bool check() const override {
            return _type == DisturbanceConditionType::LessThan ? _var->value() - _target < 0
                 : _type == DisturbanceConditionType::EqualTo  ? _var->value() == _target
                 : _type == DisturbanceConditionType::AtLeast  ? _var->value() - _target >= 0
                 : false;
        }

    private:
        const flint::IVariable* _var;
        const DisturbanceConditionType _type;
        const DynamicVar _target;
    };

    class VariablePropertyDisturbanceCondition : public IDisturbanceCondition {
    public:
        VariablePropertyDisturbanceCondition(flint::IVariable* var, std::string prop, DisturbanceConditionType type, const DynamicVar& target)
            : _var(var), _property(prop), _type(type), _target(target) { }

        bool check() const override {
            return _type == DisturbanceConditionType::LessThan ? _var->value()[_property] - _target < 0
                : _type == DisturbanceConditionType::EqualTo ? _var->value()[_property] == _target
                : _type == DisturbanceConditionType::AtLeast ? _var->value()[_property] - _target >= 0
                : false;
        }

    private:
        const flint::IVariable* _var;
        const std::string _property;
        const DisturbanceConditionType _type;
        const DynamicVar _target;
    };

    class CBMDistEventRef {
	public:
		CBMDistEventRef() = default;

		explicit CBMDistEventRef(
			std::string& disturbanceType, int dmId, int year,
			int transitionId, const std::string& landClassTransition,
            std::vector<std::shared_ptr<IDisturbanceCondition>> conditions,
            const DynamicObject& metadata) :
				_disturbanceType(disturbanceType), _disturbanceMatrixId(dmId), _year(year),
				_transitionRuleId(transitionId), _landClassTransition(landClassTransition),
                _disturbanceConditions(conditions), _metadata(metadata) {

			if (landClassTransition != "") {
				_hasLandClassTransition = true;
			}
		}

		std::string disturbanceType() const { return _disturbanceType; }
        int transitionRuleId() const { return _transitionRuleId; }
		int disturbanceMatrixId() const { return _disturbanceMatrixId; }
		double year() const { return _year; }
		std::string landClassTransition() const { return _landClassTransition; }
		bool hasLandClassTransition() const { return _hasLandClassTransition; }
        const DynamicObject& metadata() { return _metadata; }

        bool checkConditions() {
            for (const auto condition : _disturbanceConditions) {
                if (!condition->check()) {
                    return false;
                }
            }

            return true;
        }

	private:
		std::string _disturbanceType;
		int _disturbanceMatrixId;
        int _transitionRuleId;
		int	_year;
		bool _hasLandClassTransition = false;
		std::string _landClassTransition;
        DynamicObject _metadata;
        std::vector<std::shared_ptr<IDisturbanceCondition>> _disturbanceConditions;
    };

	class CBMDistEventTransfer {
	public:
		CBMDistEventTransfer() = default;

		CBMDistEventTransfer(flint::ILandUnitDataWrapper& landUnitData, const DynamicObject& data) :			
			_disturbanceMatrixId(data["disturbance_matrix_id"]),
			_sourcePool(landUnitData.getPool(data["source_pool_name"].convert<std::string>())), 
			_destPool(landUnitData.getPool(data["dest_pool_name"].convert<std::string>())),
			_proportion(data["proportion"]) { }

		CBMDistEventTransfer(flint::ILandUnitDataWrapper& landUnitData, const std::string& sourcePool, 
			                 const std::string& destPool, double proportion) :			
			_sourcePool(landUnitData.getPool(sourcePool)),
			_destPool(landUnitData.getPool(destPool)),
			_proportion(proportion) { }

		int disturbanceMatrixId() const { return _disturbanceMatrixId; }
		const flint::IPool* sourcePool() const { return _sourcePool; }
		const flint::IPool* destPool() const { return _destPool; }
		double proportion() const { return _proportion; }
		void setProportion(double proportion) { _proportion = proportion; }

	private:		
		int _disturbanceMatrixId;
		const flint::IPool* _sourcePool;
		const flint::IPool* _destPool;
		double _proportion;
	};

	class CBMDisturbanceListener : public CBMModuleBase {
	public:
        CBMDisturbanceListener() : CBMModuleBase() {}
		virtual ~CBMDisturbanceListener() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

		virtual void doLocalDomainInit() override;
		virtual void doSystemShutdown() override;
		virtual void doTimingInit() override;
		virtual void doTimingStep() override;

	private:
		typedef std::vector<CBMDistEventTransfer> EventVector;
		typedef std::unordered_map<int, EventVector> EventMap;

		NotificationCenter* _notificationCenter;
		std::vector<std::string> _layerNames;
		std::vector<const flint::IVariable*> _layers;
		flint::IVariable* _landClass;
		flint::IVariable* _spu;
        flint::IVariable* _classifierSet;
		EventMap _matrices;
		std::unordered_map<std::pair<std::string, int>, int> _dmAssociations;
		std::unordered_map<std::string, std::string> _landClassTransitions;
		std::map<int, std::vector<CBMDistEventRef>> _landUnitEvents;
		std::unordered_map<std::string, int> _distTypeCodes;
		std::unordered_map<int, std::string> _distTypeNames;
		std::unordered_set<std::string> _errorLayers;
        std::unordered_set<std::string> _classifierNames;

		void fetchMatrices();
		void fetchDMAssociations();
		void fetchLandClassTransitions();
		void fetchDistTypeCodes();
		std::string getDisturbanceTypeName(const DynamicObject& eventData);
		bool addLandUnitEvent(const DynamicVar& eventData);
	};

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMDISTURBANCELISTENER_H_
