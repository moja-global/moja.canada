#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/observer.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMDecayModule::configure(const DynamicObject& config) { }

    void CBMDecayModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(
            *this, &IModule::onLocalDomainInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(
            *this, &IModule::onTimingInit));

        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(
            *this, &IModule::onTimingStep));
    }

    void CBMDecayModule::getTransfer(flint::IOperation* operation,
                                     double meanAnnualTemperature,
                                     const std::string& domPool,
                                     flint::IPool::ConstPtr poolSrc,
                                     flint::IPool::ConstPtr poolDest) {
        double decayRate = _decayParameters[domPool].getDecayRate(meanAnnualTemperature);
        double propToAtmosphere = _decayParameters[domPool].pAtm;
        operation
            ->addTransfer(poolSrc, poolDest, decayRate * (1 - propToAtmosphere))
            ->addTransfer(poolSrc, _atmosphere, decayRate * propToAtmosphere);
    }

    void CBMDecayModule::getTransfer(flint::IOperation* operation,
                                     double meanAnnualTemperature,
                                     const std::string& domPool,
                                     flint::IPool::ConstPtr pool) {
        double decayRate = _decayParameters[domPool].getDecayRate(meanAnnualTemperature);
        double propToAtmosphere = _decayParameters[domPool].pAtm;
        operation->addTransfer(pool, _atmosphere, decayRate * propToAtmosphere);
    }

    void CBMDecayModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& init) {
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
        _atmosphere = _landUnitData->getPool("Atmosphere");

        const auto decayParameterTable = _landUnitData->getVariable("decay_parameters")->value()
            .extract<const std::vector<DynamicObject>>();

        _decayParameters.clear();
        for (const auto row : decayParameterTable) {
            _decayParameters.emplace(row["pool"].convert<std::string>(),
                                     PoolDecayParameters(row));
        }
    }

    void CBMDecayModule::onTimingInit(const flint::TimingInitNotification::Ptr&) {
        _T = _landUnitData->getVariable("mean_annual_temperature")->value();
        _slowMixingRate = _landUnitData->getVariable("other_to_branch_snag_split")->value();
    }

    void CBMDecayModule::onTimingStep(const flint::TimingStepNotification::Ptr& step) {
        auto domDecay = _landUnitData->createProportionalOperation();
        getTransfer(domDecay.get(), _T, "AboveGroundVeryFastSoil", _aboveGroundVeryFastSoil, _aboveGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "BelowGroundVeryFastSoil", _belowGroundVeryFastSoil, _belowGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "AboveGroundFastSoil", _aboveGroundFastSoil, _aboveGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "BelowGroundFastSoil", _belowGroundFastSoil, _belowGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "MediumSoil", _mediumSoil, _aboveGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "SoftwoodStemSnag", _softwoodStemSnag, _aboveGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "SoftwoodBranchSnag", _softwoodBranchSnag, _aboveGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "HardwoodStemSnag", _hardwoodStemSnag, _aboveGroundSlowSoil);
        getTransfer(domDecay.get(), _T, "HardwoodBranchSnag", _hardwoodBranchSnag, _aboveGroundSlowSoil);
        _landUnitData->submitOperation(domDecay);

        auto soilDecay = _landUnitData->createProportionalOperation();
        getTransfer(soilDecay.get(), _T, "AboveGroundSlowSoil", _aboveGroundSlowSoil);
        getTransfer(soilDecay.get(), _T, "BelowGroundSlowSoil", _belowGroundSlowSoil);
        _landUnitData->submitOperation(soilDecay);

        auto soilTurnover = _landUnitData->createProportionalOperation();
        soilTurnover->addTransfer(_aboveGroundSlowSoil, _belowGroundSlowSoil, _slowMixingRate);
        _landUnitData->submitOperation(soilTurnover);
    }

}}} // namespace moja::modules::cbm
