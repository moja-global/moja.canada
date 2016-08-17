#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/logging.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMDecayModule::configure(const DynamicObject& config) {
        if (config.contains("extra_decay_removals")) {
            _extraDecayRemovals = config["extra_decay_removals"];
        }
    }

    void CBMDecayModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit	, &CBMDecayModule::onLocalDomainInit	, *this);
		notificationCenter.subscribe(signals::TimingInit		, &CBMDecayModule::onTimingInit			, *this);
		notificationCenter.subscribe(signals::TimingStep		, &CBMDecayModule::onTimingStep			, *this);
	}

    void CBMDecayModule::getTransfer(flint::IOperation::Ptr operation,
                                     double meanAnnualTemperature,
                                     const std::string& domPool,
                                     flint::IPool::ConstPtr poolSrc,
                                     flint::IPool::ConstPtr poolDest) {
        double decayRate = _decayParameters[domPool].getDecayRate(meanAnnualTemperature);
        double propToAtmosphere = _decayParameters[domPool].pAtm;
        operation->addTransfer(poolSrc, poolDest, decayRate * (1 - propToAtmosphere))
            ->addTransfer(poolSrc, _atmosphere, decayRate * propToAtmosphere);
    }

    void CBMDecayModule::getTransfer(flint::IOperation::Ptr operation,
                                     double meanAnnualTemperature,
                                     const std::string& domPool,
                                     flint::IPool::ConstPtr pool) {
        double decayRate = _decayParameters[domPool].getDecayRate(meanAnnualTemperature);
        double propToAtmosphere = _decayParameters[domPool].pAtm;

        // Decay a proportion of a pool to the atmosphere as well as any additional
        // removals (dissolved organic carbon, etc.) - additional removals are subtracted
        // from the amount decayed to the atmosphere.
        double propRemovals = 0.0;
        const auto removals = _decayRemovals.find(domPool);
        if (removals != _decayRemovals.end()) {
            for (const auto removal : (*removals).second) {
                const auto dstPool = _landUnitData->getPool(removal.first);
                const auto dstProp = removal.second;
                propRemovals += dstProp;
                operation->addTransfer(pool, dstPool, decayRate * dstProp);
            }
        }

        operation->addTransfer(pool, _atmosphere, decayRate * (propToAtmosphere - propRemovals));
    }

    void CBMDecayModule::onLocalDomainInit() {
        _aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
        _belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
        _aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
        _belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");
        _mediumSoil = _landUnitData->getPool("MediumSoil");
        _aboveGroundSlowSoil = _landUnitData->getPool("AboveGroundSlowSoil");
        _belowGroundSlowSoil = _landUnitData->getPool("BelowGroundSlowSoil");
        _softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
        _softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
        _hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
        _hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
        _atmosphere = _landUnitData->getPool("CO2");

        _spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
        _growthCurveId = _landUnitData->getVariable("growth_curve_id");
        _currentLandClass = _landUnitData->getVariable("current_land_class");

        const auto decayParameterTable = _landUnitData->getVariable("decay_parameters")->value()
            .extract<const std::vector<DynamicObject>>();

        _decayParameters.clear();
        for (const auto row : decayParameterTable) {
            _decayParameters.emplace(row["pool"].convert<std::string>(),
                                     PoolDecayParameters(row));
        }
    }

    void CBMDecayModule::onTimingInit() {
        _T = _landUnitData->getVariable("mean_annual_temperature")->value();
		_slowMixingRate = _landUnitData->getVariable("slow_ag_to_bg_mixing_rate")->value();

        if (_extraDecayRemovals) {
            const auto decayRemovalsTable = _landUnitData->getVariable("decay_removals")->value()
                .extract<const std::vector<DynamicObject>>();

            _decayRemovals.clear();
            for (const auto row : decayRemovalsTable) {
                _decayRemovals[row["from_pool"]][row["to_pool"]] = row["proportion"];
            }
        }
    }

    bool CBMDecayModule::shouldRun() {
        // When moss module is spinning up, nothing to grow, turnover and decay.
        bool spinupMossOnly = _spinupMossOnly->value();
        if (spinupMossOnly) {
            return false;
        }

        const auto& standGrowthCurveID = _growthCurveId->value();
        int gcid = standGrowthCurveID.isEmpty() ? -1 : standGrowthCurveID;
        if (gcid == -1) {
            return false;
        }

        const auto& landClass = _currentLandClass->value();
        auto lc = landClass.convert<std::string>();
        if (lc != "FL") {
            return false;
        }

        return true;
    }

    void CBMDecayModule::onTimingStep() {
        if (!shouldRun()) {
            return;
        }

        auto domDecay = _landUnitData->createProportionalOperation();
        getTransfer(domDecay, _T, "AboveGroundVeryFastSoil", _aboveGroundVeryFastSoil, _aboveGroundSlowSoil);
        getTransfer(domDecay, _T, "BelowGroundVeryFastSoil", _belowGroundVeryFastSoil, _belowGroundSlowSoil);
        getTransfer(domDecay, _T, "AboveGroundFastSoil", _aboveGroundFastSoil, _aboveGroundSlowSoil);
        getTransfer(domDecay, _T, "BelowGroundFastSoil", _belowGroundFastSoil, _belowGroundSlowSoil);
        getTransfer(domDecay, _T, "MediumSoil", _mediumSoil, _aboveGroundSlowSoil);
        getTransfer(domDecay, _T, "SoftwoodStemSnag", _softwoodStemSnag, _aboveGroundSlowSoil);
        getTransfer(domDecay, _T, "SoftwoodBranchSnag", _softwoodBranchSnag, _aboveGroundSlowSoil);
        getTransfer(domDecay, _T, "HardwoodStemSnag", _hardwoodStemSnag, _aboveGroundSlowSoil);
        getTransfer(domDecay, _T, "HardwoodBranchSnag", _hardwoodBranchSnag, _aboveGroundSlowSoil);
        _landUnitData->submitOperation(domDecay);
		_landUnitData->applyOperations();			
       
		auto soilDecay = _landUnitData->createProportionalOperation();
        getTransfer(soilDecay, _T, "AboveGroundSlowSoil", _aboveGroundSlowSoil);
        getTransfer(soilDecay, _T, "BelowGroundSlowSoil", _belowGroundSlowSoil);
        _landUnitData->submitOperation(soilDecay);		
		_landUnitData->applyOperations();
		
		auto soilTurnover = _landUnitData->createProportionalOperation();
        soilTurnover->addTransfer(_aboveGroundSlowSoil, _belowGroundSlowSoil, _slowMixingRate);
        _landUnitData->submitOperation(soilTurnover);
		_landUnitData->applyOperations();
    }

}}} // namespace moja::modules::cbm
