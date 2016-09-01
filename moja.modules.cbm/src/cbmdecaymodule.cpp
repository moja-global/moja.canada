#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/logging.h"
#include "moja/modules/cbm/printpools.h"
namespace moja {
namespace modules {
namespace cbm {

    void CBMDecayModule::configure(const DynamicObject& config) { }

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
        operation->addTransfer(pool, _atmosphere, decayRate * propToAtmosphere);
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

        const auto decayParameterTable = _landUnitData->getVariable("decay_parameters")->value()
            .extract<const std::vector<DynamicObject>>();

        _decayParameters.clear();
        for (const auto row : decayParameterTable) {
            _decayParameters.emplace(row["pool"].convert<std::string>(),
                                     PoolDecayParameters(row));
        }
    }

    void CBMDecayModule::onTimingInit() {

		auto mat = _landUnitData->getVariable("mean_annual_temperature")->value();
		_T = mat.isEmpty() ? 0
			: mat.isTimeSeries() ? mat.extract<TimeSeries>().value()
			: mat.extract<float>();

		_slowMixingRate = _landUnitData->getVariable("slow_ag_to_bg_mixing_rate")->value();
    }

    void CBMDecayModule::onTimingStep() {	
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();

		//when moss module is spinning up, nothing to grow, turnover and decay
		if (spinupMossOnly) return;

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

		//PrintPools p;
		//p.printForestPools("", _landUnitData.operator*());

    }

}}} // namespace moja::modules::cbm
