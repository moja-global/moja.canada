#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/yieldtablegrowthmodule.h"

namespace moja {
namespace modules {
namespace cbm {	

    void YieldTableGrowthModule::configure(const DynamicObject& config) { }

    void YieldTableGrowthModule::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(
            *this, &IModule::onLocalDomainInit));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(
            *this, &IModule::onTimingStep));
        notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(
            *this, &IModule::onTimingInit));
    }

    void YieldTableGrowthModule::onTimingInit(const flint::TimingInitNotification::Ptr&) {
        // Get the stand growth curve ID associated to the pixel/svo.
        const auto& standGrowthCurveID = _landUnitData->getVariable("growth_curve_id")->value();

        if (standGrowthCurveID.isEmpty()) {
            _standGrowthCurveID = -1;
        } else {
            _standGrowthCurveID = standGrowthCurveID;
        }

        // Try to get the stand growth curve and related yield table data from memory.
        if (_standGrowthCurveID > 0) {
            int standAge = _landUnitData->getVariable("initial_age")->value();
            _age->set_value(standAge);

            bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(_standGrowthCurveID);
            if (!carbonCurveFound) {
                std::shared_ptr<StandGrowthCurve> standGrowthCurve = createStandGrowthCurve(_standGrowthCurveID);

                // Pre-process the stand growth curve here.
                standGrowthCurve->processStandYieldTables();

                // Process and convert yield volume to carbon curves.
                _volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);
            }
        }
    }

    void YieldTableGrowthModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& init) {
        _softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
        _softwoodOther = _landUnitData->getPool("SoftwoodOther");
        _softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
        _softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

        _hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
        _hardwoodOther = _landUnitData->getPool("HardwoodOther");
        _hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
        _hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
        _hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

        _atmosphere = _landUnitData->getPool("Atmosphere");		
        _age = _landUnitData->getVariable("age");
        _volumeToBioGrowth = std::make_shared<VolumeToBiomassCarbonGrowth>();
    }

    void YieldTableGrowthModule::onTimingStep(const flint::TimingStepNotification::Ptr& step) {
        if (_standGrowthCurveID < 0) {
            return;
        }

        // Get stand age for current pixel/svo/stand.
        int standAge = _age->value();

        double standSoftwoodMerch = _softwoodMerch->value();
        double standSoftwoodOther = _softwoodOther->value();
        double standSoftwoodFoliage = _softwoodFoliage->value();
        double standSWCoarseRootsCarbon = _softwoodCoarseRoots->value();
        double standSWFineRootsCarbon = _softwoodFineRoots->value();
        double standHardwoodMerch = _hardwoodMerch->value();
        double standHardwoodOther = _hardwoodOther->value();
        double standHardwoodFoliage = _hardwoodFoliage->value();
        double standHWCoarseRootsCarbon = _hardwoodCoarseRoots->value();
        double standHWFineRootsCarbon = _hardwoodFineRoots->value();

        // Get the above ground biomass carbon growth increment.
        std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement =
            _volumeToBioGrowth->getAGBiomassCarbonIncrements(_standGrowthCurveID, standAge);
    
        double swm = abIncrement->softwoodMerch();
        double swo = abIncrement->softwoodOther();
        double swf = abIncrement->softwoodFoliage();	
        double hwm = abIncrement->hardwoodMerch();
        double hwo = abIncrement->hardwoodOther();
        double hwf = abIncrement->hardwoodFoliage();
    
        // The MAX function calls below to enforce the biomass carbon changes
        // keeps a POSITIVE value for the pool value.
        swm = std::max(swm, -standSoftwoodMerch);
        swo = std::max(swo, -standSoftwoodOther);
        swf = std::max(swf, -standSoftwoodFoliage);
        hwm = std::max(hwm, -standHardwoodMerch);
        hwo = std::max(hwo, -standHardwoodOther);
        hwf = std::max(hwf, -standHardwoodFoliage);

        // Compute the total biomass carbon for softwood and hardwood component.
        double totalSWAgBioCarbon = standSoftwoodMerch + swm + standSoftwoodFoliage + swf + standSoftwoodOther + swo;
        double totalHWAgBioCarbon = standHardwoodMerch + hwm + standHardwoodFoliage + hwf + standHardwoodOther + hwo;

        // Get the root biomass carbon increment based on the total above ground biomass.
        std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement =
            _volumeToBioGrowth->getBGBiomassCarbonIncrements(
                totalSWAgBioCarbon, standSWCoarseRootsCarbon, standSWFineRootsCarbon,
                totalHWAgBioCarbon, standHWCoarseRootsCarbon, standHWFineRootsCarbon);

        double swcr = bgIncrement->softwoodFineRoots();
        double swfr = bgIncrement->softwoodFineRoots();
        double hwcr = bgIncrement->hardwoodFineRoots();
        double hwfr = bgIncrement->hardwoodFineRoots();

        auto growth = _landUnitData->createStockOperation();	

        growth
            ->addTransfer(_atmosphere, _softwoodMerch, swm)
            ->addTransfer(_atmosphere, _softwoodOther, swo)
            ->addTransfer(_atmosphere, _softwoodFoliage, swf)
            ->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr)
            ->addTransfer(_atmosphere, _softwoodFineRoots, swfr)

            ->addTransfer(_atmosphere, _hardwoodMerch, hwm)
            ->addTransfer(_atmosphere, _hardwoodOther, hwo)
            ->addTransfer(_atmosphere, _hardwoodFoliage, hwf)
            ->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr)
            ->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr);

        _landUnitData->submitOperation(growth);			
        
        _age->set_value(standAge + 1);
    }

    std::shared_ptr<StandGrowthCurve> YieldTableGrowthModule::createStandGrowthCurve(Int64 standGrowthCurveID) {
        auto standGrowthCurve = std::make_shared<StandGrowthCurve>(standGrowthCurveID);

        // Get the table of softwood merchantable volumes associated to the stand growth curve.
        std::vector<DynamicObject> softwoodYieldTable;
        const auto& swTable = _landUnitData->getVariable("softwood_yield_table")->value();
        if (!swTable.isEmpty()) {
            softwoodYieldTable = swTable.extract<const std::vector<DynamicObject>>();
        }

        auto swTreeYieldTable = std::make_shared<TreeYieldTable>(softwoodYieldTable, SpeciesType::Softwood);
        standGrowthCurve->addYieldTable(swTreeYieldTable);

        // Get the table of hardwood merchantable volumes associated to the stand growth curve.
        std::vector<DynamicObject> hardwoodYieldTable;
        const auto& hwTable = _landUnitData->getVariable("hardwood_yield_table")->value();
        if (!hwTable.isEmpty()) {
            hardwoodYieldTable = hwTable.extract<const std::vector<DynamicObject>>();
        }

        auto hwTreeYieldTable = std::make_shared<TreeYieldTable>(hardwoodYieldTable, SpeciesType::Hardwood);
        standGrowthCurve->addYieldTable(hwTreeYieldTable);
        
        // Query for the appropriate PERD factor data.
        std::vector<DynamicObject> vol2bioParams;
        const auto& vol2bio = _landUnitData->getVariable("volume_to_biomass_parameters")->value();
        if (vol2bio.isVector()) {
            vol2bioParams = vol2bio.extract<std::vector<DynamicObject>>();
        } else {
            vol2bioParams.push_back(vol2bio.extract<DynamicObject>());
        }

        for (const auto& row : vol2bioParams) {
            auto perdFactor = std::make_unique<PERDFactor>();
            perdFactor->setValue(row);

            std::string forestType = row["forest_type"].convert<std::string>();
            if (forestType == "Softwood") {
                standGrowthCurve->setPERDFactor(std::move(perdFactor), SpeciesType::Softwood);
            } else if (forestType == "Hardwood") {
                standGrowthCurve->setPERDFactor(std::move(perdFactor), SpeciesType::Hardwood);
            }
        }

        return standGrowthCurve;
    }
     
}}}
