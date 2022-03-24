#include "moja/modules/cbm/cbmpartitioningmodule.h"

#include <moja/flint/variable.h>
#include <moja/flint/timing.h>
#include <moja/flint/ipool.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    class IRecoveryRule {
    public:
        virtual ~IRecoveryRule() { }
        virtual bool isSatisfied() = 0;
    };

    class TimeRecoveryRule : public IRecoveryRule {
    public:
        TimeRecoveryRule(const flint::ILandUnitDataWrapper* landUnitData, int years, bool spinup) {
            _age = landUnitData->getVariable("age");
            _timing = landUnitData->timing();
            int yearsElapsedInSpinup = spinup ? _age->value().extract<int>() : 0;
            int currentYear = landUnitData->timing()->curStartDate().year();
            _targetYear = currentYear + (years - yearsElapsedInSpinup);
        }

        bool isSatisfied() override {
            return _timing->curStartDate().year() >= _targetYear;
        }
   
    private:
        int _targetYear;
        const flint::IVariable* _age;
        const flint::ITiming* _timing;
    };

    class CarbonRecoveryRule : public IRecoveryRule {
    public:
        CarbonRecoveryRule(const std::vector<const flint::IPool*> pools,
                           double target) : _pools(pools), _target(target) { }

        bool isSatisfied() override {
            double currentValue = 0.0;
            for (const auto pool : _pools) {
                currentValue += pool->value();
            }

            return currentValue >= _target;
        }

    private:
        double _target;
        std::vector<const flint::IPool*> _pools;
    };

	void CBMPartitioningModule::subscribe(NotificationCenter& notificationCenter) {		
        notificationCenter.subscribe(signals::LocalDomainInit,  &CBMPartitioningModule::onLocalDomainInit,  *this);
        notificationCenter.subscribe(signals::TimingInit,       &CBMPartitioningModule::onTimingInit,       *this);
        notificationCenter.subscribe(signals::DisturbanceEvent, &CBMPartitioningModule::onDisturbanceEvent, *this);
        notificationCenter.subscribe(signals::TimingStep,       &CBMPartitioningModule::onTimingStep,       *this);
    }
    
	void CBMPartitioningModule::doLocalDomainInit() {
        _partition = _landUnitData->getVariable("partition");
        _spinupParameters = _landUnitData->getVariable("spinup_parameters");
        _spu = _landUnitData->getVariable("spu");

        fetchRecoveryRules();

        const auto distMortality = _landUnitData->getVariable("disturbance_mortality");
        for (const auto& row : distMortality->value().extract<const std::vector<DynamicObject>>()) {
            std::string disturbanceType = row["disturbance_type"];
            int spu = row["spu"];
            double mortality = row["mortality"];
            _disturbanceMortality[disturbanceType][spu] = mortality;
        }

        const auto distCategories = _landUnitData->getVariable("disturbance_categories");
        for (const auto& row : distCategories->value().extract<const std::vector<DynamicObject>>()) {
            std::string disturbanceType = row["disturbance_type"];
            std::string category = row["category"];
            _disturbanceCategories[disturbanceType] = category;
        }

        for (const auto& poolName : {
            "SoftwoodMerch", "SoftwoodFoliage", "SoftwoodOther", "HardwoodMerch", "HardwoodFoliage", "HardwoodOther"
        }) {
            _agBiomassPools.push_back(_landUnitData->getPool(poolName));
        }

        for (const auto& poolName : {
            "SoftwoodMerch", "SoftwoodFoliage", "SoftwoodOther", "SoftwoodCoarseRoots", "SoftwoodFineRoots",
            "HardwoodMerch", "HardwoodFoliage", "HardwoodOther", "HardwoodCoarseRoots", "HardwoodFineRoots"
        }) {
            _totalBiomassPools.push_back(_landUnitData->getPool(poolName));
        }

        for (const auto& poolName : {
            "SoftwoodMerch", "SoftwoodFoliage", "SoftwoodOther", "SoftwoodCoarseRoots", "SoftwoodFineRoots",
            "HardwoodMerch", "HardwoodFoliage", "HardwoodOther", "HardwoodCoarseRoots", "HardwoodFineRoots",
            "AboveGroundVeryFastSoil", "BelowGroundVeryFastSoil", "AboveGroundFastSoil", "BelowGroundFastSoil",
            "MediumSoil", "AboveGroundSlowSoil", "BelowGroundSlowSoil", "SoftwoodStemSnag", "SoftwoodBranchSnag",
            "HardwoodStemSnag", "HardwoodBranchSnag"
        }) {
            _totalEcoPools.push_back(_landUnitData->getPool(poolName));
        }
    }

    void CBMPartitioningModule::doTimingInit() {
        _spuId = _spu->value();
        _cumulativeMortality = 0.0;
        _pendingRecoveryRules.clear();
        _activeRecoveryRules.clear();

        const auto& spinup = _spinupParameters->value();
        const auto& spinupParams = spinup.extract<DynamicObject>();
        const auto& lastPassDistType = spinupParams["last_pass_disturbance_type"].convert<std::string>();
        _partition->set_value(_disturbanceCategories[lastPassDistType]);

        auto lastPassRecoveryRule = createInitialRecoveryRule(lastPassDistType);
        if (lastPassRecoveryRule == nullptr) {
            return;
        }

        if (lastPassRecoveryRule->isSatisfied()) {
            _partition->set_value("A");
        } else {
            _activeRecoveryRules.push_back(lastPassRecoveryRule);
        }
    }

    void CBMPartitioningModule::doDisturbanceEvent(DynamicVar e) {
        auto& data = e.extract<const DynamicObject>();
        std::string disturbanceType = data["disturbance"];
        auto mortality = _disturbanceMortality[disturbanceType][_spuId];
        if (mortality < 0.2) {
            doSmallDisturbanceEvent(disturbanceType, mortality);
        } else {
            doLargeDisturbanceEvent(disturbanceType);
        }
    }

    void CBMPartitioningModule::doSmallDisturbanceEvent(std::string disturbanceType, double mortality) {
        auto disturbanceCategory = _disturbanceCategories[disturbanceType];
        std::string currentCategory = _partition->value();
        if (disturbanceCategory == currentCategory) {
            auto recoveryRule = createRecoveryRule(disturbanceType);
            if (recoveryRule != nullptr) {
                _activeRecoveryRules.push_back(recoveryRule);
            }

            return;
        }

        auto pendingRule = createRecoveryRule(disturbanceType);
        if (pendingRule != nullptr) {
            _pendingRecoveryRules.push_back(pendingRule);
        }

        _cumulativeMortality += mortality;
        if (_cumulativeMortality >= 0.2) {
            _activeRecoveryRules = _pendingRecoveryRules;
            _partition->set_value(disturbanceCategory);
            _pendingRecoveryRules.clear();
            _cumulativeMortality = 0.0;
        }
    }

    void CBMPartitioningModule::doLargeDisturbanceEvent(std::string disturbanceType) {
        _cumulativeMortality = 0.0;
        
        auto disturbanceCategory = _disturbanceCategories[disturbanceType];
        std::string currentCategory = _partition->value();
        if (disturbanceCategory != currentCategory) {
            _activeRecoveryRules = _pendingRecoveryRules;
            _partition->set_value(disturbanceCategory);
        }

        auto recoveryRule = createRecoveryRule(disturbanceType);
        if (recoveryRule != nullptr) {
            _activeRecoveryRules.push_back(recoveryRule);
        }

        _pendingRecoveryRules.clear();
    }

    void CBMPartitioningModule::doTimingStep() {
        if (_activeRecoveryRules.empty()) {
            return;
        }

        const auto satisfiedRules = std::remove_if(
            _activeRecoveryRules.begin(),
            _activeRecoveryRules.end(),
            [](std::shared_ptr<IRecoveryRule> rule) {
                return rule->isSatisfied();
            });

        if (satisfiedRules != _activeRecoveryRules.end()) {
            _activeRecoveryRules.erase(satisfiedRules, _activeRecoveryRules.end());
        }

        if (_activeRecoveryRules.empty()) {
            auto currentCategory = _partition->value().convert<std::string>();
            _partition->set_value(currentCategory == "A" ? "N" : "A");
            _cumulativeMortality = 0.0;
            _pendingRecoveryRules.clear();
        }
    }

    void CBMPartitioningModule::fetchRecoveryRules() {
        const auto& recoveryRules = _landUnitData->getVariable("recovery_rules")->value();
        if (recoveryRules.isVector()) {
            for (const auto& recoveryRule : recoveryRules.extract<const std::vector<DynamicObject>>()) {
                std::string disturbanceType = recoveryRule["disturbance_type"];
                int spu = recoveryRule["spu"];
                std::string ruleType = recoveryRule["rule_type"];
                auto ruleValue = recoveryRule["rule_value"];
                bool spinup = recoveryRule["spinup"];

                if (spinup) {
                    _initialRecoveryRules[disturbanceType][spu] = std::make_pair(ruleType, ruleValue);
                } else {
                    _recoveryRules[disturbanceType][spu] = std::make_pair(ruleType, ruleValue);
                }
            }
        } else {
            std::string disturbanceType = recoveryRules["disturbance_type"];
            int spu = recoveryRules["spu"];
            std::string ruleType = recoveryRules["rule_type"];
            auto ruleValue = recoveryRules["rule_value"];
            bool spinup = recoveryRules["spinup"];

            if (spinup) {
                _initialRecoveryRules[disturbanceType][spu] = std::make_pair(ruleType, ruleValue);
            } else {
                _recoveryRules[disturbanceType][spu] = std::make_pair(ruleType, ruleValue);
            }
        }
    }

    std::shared_ptr<IRecoveryRule> CBMPartitioningModule::createInitialRecoveryRule(std::string disturbanceType)
    {
        auto distTypeRulesIt = _initialRecoveryRules.find(disturbanceType);
        if (distTypeRulesIt == _initialRecoveryRules.end()) {
            return nullptr;
        }

        // Try to find an SPU-specific recovery rule first, then fall back to a general one.
        const auto& distTypeRules = distTypeRulesIt->second;
        auto distTypeRuleIt = distTypeRules.find(_spuId);
        if (distTypeRuleIt == distTypeRules.end()) {
            distTypeRuleIt = distTypeRules.find(-1);
        }

        if (distTypeRuleIt == distTypeRules.end()) {
            return nullptr;
        }

        const auto& distTypeRuleConfig = distTypeRuleIt->second;
        std::shared_ptr<IRecoveryRule> recoveryRule = nullptr;

        auto ruleType = std::get<0>(distTypeRuleConfig);
        if (ruleType == "years_since_last_pass_disturbance") {
            int target = std::get<1>(distTypeRuleConfig);
            recoveryRule = std::make_shared<TimeRecoveryRule>(_landUnitData.get(), target, true);
        }

        return recoveryRule;
    }

    std::shared_ptr<IRecoveryRule> CBMPartitioningModule::createRecoveryRule(std::string disturbanceType)
    {
        auto distTypeRulesIt = _recoveryRules.find(disturbanceType);
        if (distTypeRulesIt == _recoveryRules.end()) {
            return nullptr;
        }

        // Try to find an SPU-specific recovery rule first, then fall back to a general one.
        const auto& distTypeRules = distTypeRulesIt->second;
        auto distTypeRuleIt = distTypeRules.find(_spuId);
        if (distTypeRuleIt == distTypeRules.end()) {
            distTypeRuleIt = distTypeRules.find(-1);
        }

        if (distTypeRuleIt == distTypeRules.end()) {
            return nullptr;
        }

        const auto& distTypeRuleConfig = distTypeRuleIt->second;
        std::shared_ptr<IRecoveryRule> recoveryRule = nullptr;

        auto ruleType = std::get<0>(distTypeRuleConfig);
        if (ruleType == "years_since_disturbance") {
            int target = std::get<1>(distTypeRuleConfig);
            recoveryRule = std::make_shared<TimeRecoveryRule>(_landUnitData.get(), target, false);
        } else if (ruleType == "ag_biomass") {
            double target = 0.0;
            for (const auto pool : _agBiomassPools) {
                target += pool->value();
            }

            recoveryRule = std::make_shared<CarbonRecoveryRule>(_agBiomassPools, target);
        } else if (ruleType == "total_biomass") {
            double target = 0.0;
            for (const auto pool : _totalBiomassPools) {
                target += pool->value();
            }

            recoveryRule = std::make_shared<CarbonRecoveryRule>(_totalBiomassPools, target);
        } else if (ruleType == "total_eco") {
            double target = 0.0;
            for (const auto pool : _totalEcoPools) {
                target += pool->value();
            }

            recoveryRule = std::make_shared<CarbonRecoveryRule>(_totalEcoPools, target);
        }

        return recoveryRule;
    }

}}}
