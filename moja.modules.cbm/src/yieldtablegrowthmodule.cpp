#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/yieldtablegrowthmodule.h"
#include "moja/logging.h"

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
        _standGrowthCurveID = standGrowthCurveID.isEmpty() ? -1 : standGrowthCurveID;

        // Try to get the stand growth curve and related yield table data from memory.
        if (_standGrowthCurveID > 0) {
            bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(_standGrowthCurveID);
            if (!carbonCurveFound) {
                std::shared_ptr<StandGrowthCurve> standGrowthCurve = createStandGrowthCurve(_standGrowthCurveID);

                // Pre-process the stand growth curve here.
                standGrowthCurve->processStandYieldTables();

                // Process and convert yield volume to carbon curves.
                _volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);
            }
        }

        const auto& turnoverRates = _landUnitData->getVariable("turnover_rates")->value()
            .extract<DynamicObject>();

        _softwoodFoliageFallRate = turnoverRates["softwood_foliage_fall_rate"];
        _hardwoodFoliageFallRate = turnoverRates["hardwood_foliage_fall_rate"];
        _stemAnnualTurnOverRate = turnoverRates["stem_annual_turnover_rate"];
        _softwoodBranchTurnOverRate = turnoverRates["softwood_branch_turnover_rate"];
        _hardwoodBranchTurnOverRate = turnoverRates["hardwood_branch_turnover_rate"];

        _otherToBranchSnagSplit = turnoverRates["other_to_branch_snag_split"];
        _stemSnagTurnoverRate = turnoverRates["stem_snag_turnover_rate"];
        _branchSnagTurnoverRate = turnoverRates["branch_snag_turnover_rate"];

        _coarseRootSplit = turnoverRates["coarse_root_split"];
        _coarseRootTurnProp = turnoverRates["coarse_root_turn_prop"];
        _fineRootAGSplit = turnoverRates["fine_root_ag_split"];
        _fineRootTurnProp = turnoverRates["fine_root_turn_prop"];
    }

    void YieldTableGrowthModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& init) {
        _softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
        _softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
        _softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
        _softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _softwoodOther = _landUnitData->getPool("SoftwoodOther");
        _softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
        _softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

        _hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
        _hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
        _hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
        _hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
        _hardwoodOther = _landUnitData->getPool("HardwoodOther");
        _hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
        _hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

        _aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
        _aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
        _belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
        _belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");

        _mediumSoil = _landUnitData->getPool("MediumSoil");

        _atmosphere = _landUnitData->getPool("Atmosphere");
        _age = _landUnitData->getVariable("age");
        _volumeToBioGrowth = std::make_shared<VolumeToBiomassCarbonGrowth>();
    }

    void YieldTableGrowthModule::onTimingStep(const flint::TimingStepNotification::Ptr& step) {
        // Get stand age for current pixel/svo/stand.
        int standAge = _age->value();

        // Get current biomass pool values.
        updateBiomassPools();

        try { // Following delay and isLastRotation are for spinup only.
            int delay = _landUnitData->getVariable("delay")->value();
            bool runDelay = _landUnitData->getVariable("run_delay")->value();

            // When the last rotation is done, and the delay is defined, do turnover and following decay.
            if (runDelay && delay > 0) {
                updateBiomassPools();
                doTurnover();

                // No growth in delay period.
                return;
            }
        }
        catch (...) {}

        if (_standGrowthCurveID < 0) {
            return;
        }

        // Get the above ground biomass carbon growth increment.
        std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement =
            _volumeToBioGrowth->getAGBiomassCarbonIncrements(_standGrowthCurveID, standAge);

        // The MAX function calls below to enforce the biomass carbon changes
        // keeps a POSITIVE value for the pool value.
        swm = std::max(abIncrement->softwoodMerch(), -standSoftwoodMerch);
        swo = std::max(abIncrement->softwoodOther(), -standSoftwoodOther);
        swf = std::max(abIncrement->softwoodFoliage(), -standSoftwoodFoliage);
        hwm = std::max(abIncrement->hardwoodMerch(), -standHardwoodMerch);
        hwo = std::max(abIncrement->hardwoodOther(), -standHardwoodOther);
        hwf = std::max(abIncrement->hardwoodFoliage(), -standHardwoodFoliage);

        // Compute the total biomass carbon for softwood and hardwood component.
        double totalSWAgBioCarbon = standSoftwoodMerch + swm + standSoftwoodFoliage + swf + standSoftwoodOther + swo;
        double totalHWAgBioCarbon = standHardwoodMerch + hwm + standHardwoodFoliage + hwf + standHardwoodOther + hwo;

        // Get the root biomass carbon increment based on the total above ground biomass.
        std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement =
            _volumeToBioGrowth->getBGBiomassCarbonIncrements(
            totalSWAgBioCarbon, standSWCoarseRootsCarbon, standSWFineRootsCarbon,
            totalHWAgBioCarbon, standHWCoarseRootsCarbon, standHWFineRootsCarbon);

        swcr = bgIncrement->softwoodCoarseRoots();
        swfr = bgIncrement->softwoodFineRoots();
        hwcr = bgIncrement->hardwoodCoarseRoots();
        hwfr = bgIncrement->hardwoodFineRoots();

        doHalfGrowth(); // transfer half of the biomass growth increment to the biomass pool		
        updateBiomassPools(); // update to record the current biomass pool value	plus the half increment of biomass		

        addbackBiomassTurnoverAmount(); // temporary adding back the turnover amount		
        doTurnover(); // do biomass and snag turnover

        doHalfGrowth(); // transfer the remaining half increment to the biomass pool

        _age->set_value(standAge + 1);
    }

    void YieldTableGrowthModule::doHalfGrowth() const {
        bool swOverMature = swm + swo + swf + swcr + swfr < 0;
        bool hwOverMature = hwm + hwo + hwf + hwcr + hwfr < 0;

        auto growth = _landUnitData->createStockOperation();
        if (swOverMature) {
            growth->addTransfer(_softwoodMerch, _softwoodStemSnag, -swm * 0.5);
            growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * 0.25 * 0.5);
            growth->addTransfer(_softwoodOther, _aboveGroundFastSoil, -swo * (1 - 0.25) * 0.5);
            growth->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, -swf * 0.5);
            growth->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, -swcr * 0.5 * 0.5);
            growth->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, -swcr * (1 - 0.5) * 0.5);
            growth->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, -swfr * 0.5 * 0.5);
            growth->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, -swfr * (1 - 0.5) * 0.5);
        } else {
            growth->addTransfer(_atmosphere, _softwoodMerch, swm * 0.50);
            growth->addTransfer(_atmosphere, _softwoodOther, swo * 0.50);
            growth->addTransfer(_atmosphere, _softwoodFoliage, swf * 0.50);
            growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr * 0.50);
            growth->addTransfer(_atmosphere, _softwoodFineRoots, swfr * 0.50);
        }

        if (hwOverMature) {
            growth->addTransfer(_hardwoodMerch, _hardwoodStemSnag, -hwm * 0.5);
            growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * 0.25 * 0.5);
            growth->addTransfer(_hardwoodOther, _aboveGroundFastSoil, -hwo * (1 - 0.25) * 0.5);
            growth->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, -hwf * 0.5);
            growth->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, -hwcr * 0.5 * 0.5);
            growth->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, -hwcr * (1 - 0.5) * 0.5);
            growth->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, -hwfr * 0.5 * 0.5);
            growth->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, -hwfr * (1 - 0.5) * 0.5);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodMerch, hwm * 0.50);
            growth->addTransfer(_atmosphere, _hardwoodOther, hwo * 0.50);
            growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf * 0.50);
            growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr * 0.50);
            growth->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr * 0.50);
        }

        _landUnitData->submitOperation(growth);
        _landUnitData->applyOperations();
    }

    void YieldTableGrowthModule::updateBiomassPools()
    {
        standSoftwoodMerch = _softwoodMerch->value();
        standSoftwoodOther = _softwoodOther->value();
        standSoftwoodFoliage = _softwoodFoliage->value();
        standSWCoarseRootsCarbon = _softwoodCoarseRoots->value();
        standSWFineRootsCarbon = _softwoodFineRoots->value();
        standHardwoodMerch = _hardwoodMerch->value();
        standHardwoodOther = _hardwoodOther->value();
        standHardwoodFoliage = _hardwoodFoliage->value();
        standHWCoarseRootsCarbon = _hardwoodCoarseRoots->value();
        standHWFineRootsCarbon = _hardwoodFineRoots->value();
        softwoodStemSnag = _softwoodStemSnag->value();
        softwoodBranchSnag = _softwoodBranchSnag->value();
        hardwoodStemSnag = _hardwoodStemSnag->value();
        hardwoodBranchSnag = _hardwoodBranchSnag->value();	
    }

    void YieldTableGrowthModule::doTurnover() const {			
        // Snag turnover.
        auto domTurnover = _landUnitData->createStockOperation();
        domTurnover
            ->addTransfer(_softwoodStemSnag, _mediumSoil, softwoodStemSnag * _stemSnagTurnoverRate)
            ->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, softwoodBranchSnag * _branchSnagTurnoverRate)
            ->addTransfer(_hardwoodStemSnag, _mediumSoil, hardwoodStemSnag * _stemSnagTurnoverRate)
            ->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, hardwoodBranchSnag * _branchSnagTurnoverRate);
        _landUnitData->submitOperation(domTurnover);
        
        // Biomass turnover as stock operation.
        bool runDelay = _landUnitData->getVariable("run_delay")->value();
        if (!runDelay) {
            auto bioTurnover = _landUnitData->createStockOperation();
            bioTurnover
                ->addTransfer(_softwoodMerch, _softwoodStemSnag, standSoftwoodMerch * _stemAnnualTurnOverRate)
                ->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, standSoftwoodFoliage * _softwoodFoliageFallRate)
                ->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _otherToBranchSnagSplit * _softwoodBranchTurnOverRate)
                ->addTransfer(_softwoodOther, _aboveGroundFastSoil, standSoftwoodOther * (1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
                ->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
                ->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
                ->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon *  _fineRootAGSplit * _fineRootTurnProp)
                ->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp)

                ->addTransfer(_hardwoodMerch, _hardwoodStemSnag, standHardwoodMerch * _stemAnnualTurnOverRate)
                ->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage *_hardwoodFoliageFallRate)
                ->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther *_otherToBranchSnagSplit * _hardwoodBranchTurnOverRate)
                ->addTransfer(_hardwoodOther, _aboveGroundFastSoil, standHardwoodOther *(1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
                ->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon *_coarseRootSplit * _coarseRootTurnProp)
                ->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon *(1 - _coarseRootSplit) * _coarseRootTurnProp)
                ->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon *_fineRootAGSplit * _fineRootTurnProp)
                ->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp);
            _landUnitData->submitOperation(bioTurnover);
            _landUnitData->applyOperations();
        }
    }

    void YieldTableGrowthModule::addbackBiomassTurnoverAmount() const {
        auto addbackTurnover = _landUnitData->createStockOperation();
        addbackTurnover
            ->addTransfer(_atmosphere, _softwoodMerch, standSoftwoodMerch * _stemAnnualTurnOverRate)
            ->addTransfer(_atmosphere, _softwoodOther, standSoftwoodOther * _softwoodBranchTurnOverRate)
            ->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _softwoodFoliageFallRate)
            ->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _coarseRootTurnProp)
            ->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _fineRootTurnProp)
            ->addTransfer(_atmosphere, _hardwoodMerch, standHardwoodMerch * _stemAnnualTurnOverRate)
            ->addTransfer(_atmosphere, _hardwoodOther, standHardwoodOther * _hardwoodBranchTurnOverRate)
            ->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _hardwoodFoliageFallRate)
            ->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _coarseRootTurnProp)
            ->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _fineRootTurnProp);		
        _landUnitData->submitOperation(addbackTurnover);
    }	

   
    std::shared_ptr<StandGrowthCurve> YieldTableGrowthModule::createStandGrowthCurve(Int64 standGrowthCurveID) const {
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
