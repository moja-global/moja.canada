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

		if (standGrowthCurveID.isEmpty()) {
			_standGrowthCurveID = -1;
		}
		else {
			_standGrowthCurveID = standGrowthCurveID;
		}

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

		doTurnover(); // do biomass and snag turnover
		addbackBiomassTurnoverAmount(); // temporary adding back the turnover amount		

        handleGrowthLoss(totalSWAgBioCarbon, totalHWAgBioCarbon, abIncrement, bgIncrement);
        doHalfGrowth(); // transfer the remaining half increment to the biomass pool

		_age->set_value(standAge + 1);
	}

	std::shared_ptr<OvermatureDeclineLosses> YieldTableGrowthModule::getOvermatrueDeclineLosses(
		double merchCarbonChanges, double foliageCarbonChanges, double otherCarbonChanges,
		double coarseRootCarbonChanges, double fineRootCarbonChanges) {

		auto losses = std::make_shared<OvermatureDeclineLosses>();
		double changes = merchCarbonChanges + foliageCarbonChanges + otherCarbonChanges
			+ coarseRootCarbonChanges + fineRootCarbonChanges;

		if (changes < 0) {
			losses->setLossesPresent(true);
		}

		if (merchCarbonChanges < 0) {
			losses->setMerchToStemSnags(-merchCarbonChanges);
		}

		if (foliageCarbonChanges < 0) {
			losses->setFoliageToAGVeryFast(-foliageCarbonChanges);
		}

		if (otherCarbonChanges < 0) {
			losses->setOtherToBranchSnag(-otherCarbonChanges * 0.25);
			losses->setOtherToAGFast(-otherCarbonChanges * (1- 0.25));
		}

		if (coarseRootCarbonChanges < 0) {
			losses->setCoarseRootToAGFast(-coarseRootCarbonChanges * 0.5);
			losses->setCoarseRootToBGFast(-coarseRootCarbonChanges * (1 - 0.5));
		}

		if (fineRootCarbonChanges < 0) {
			losses->setFineRootToAGVeryFast(-fineRootCarbonChanges * 0.5);
			losses->setFineRootToBGVeryFast(-fineRootCarbonChanges * (1 - 0.5));
		}

		return losses;
	};

    void YieldTableGrowthModule::handleGrowthLoss(
        double totalSWAgBioCarbon,
        double totalHWAgBioCarbon,
        std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement,
        std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement) {

        // Handle overmature turnover right after the growth module run.
        auto overmatureLoss = _landUnitData->createStockOperation();

        auto swlosses = getOvermatrueDeclineLosses(
            abIncrement->softwoodMerch(), abIncrement->softwoodFoliage(),
            abIncrement->softwoodOther(), bgIncrement->softwoodCoarseRoots(),
            bgIncrement->softwoodFineRoots());

        auto hwlosses = getOvermatrueDeclineLosses(
            abIncrement->hardwoodMerch(), abIncrement->hardwoodFoliage(),
            abIncrement->hardwoodOther(), bgIncrement->hardwoodCoarseRoots(),
            bgIncrement->hardwoodFineRoots());

        if (totalSWAgBioCarbon > 0) {
            if (swlosses->lossesPresent()) {
                // Handle softwood overmature decline - turnover.
                overmatureLoss
                    ->addTransfer(_softwoodMerch, _softwoodStemSnag, swlosses->merchToStemSnags())
                    ->addTransfer(_softwoodOther, _softwoodBranchSnag, swlosses->otherToBranchSnag())
                    ->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, swlosses->fineRootToAGVeryFast())
                    ->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, swlosses->foliageToAGVeryFast())
                    ->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, swlosses->fineRootToBGVeryFast())
                    ->addTransfer(_softwoodOther, _aboveGroundFastSoil, swlosses->otherToAGFast())
                    ->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, swlosses->coarseRootToAGFast())
                    ->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, swlosses->coarseRootToBGFast())
                    ->addTransfer(_atmosphere, _softwoodMerch, swlosses->merchToStemSnags())
                    ->addTransfer(_atmosphere, _softwoodOther, swlosses->otherToBranchSnag())
                    ->addTransfer(_atmosphere, _softwoodFineRoots, swlosses->fineRootToAGVeryFast())
                    ->addTransfer(_atmosphere, _softwoodFoliage, swlosses->foliageToAGVeryFast())
                    ->addTransfer(_atmosphere, _softwoodFineRoots, swlosses->fineRootToBGVeryFast())
                    ->addTransfer(_atmosphere, _softwoodOther, swlosses->otherToAGFast())
                    ->addTransfer(_atmosphere, _softwoodCoarseRoots, swlosses->coarseRootToAGFast())
                    ->addTransfer(_atmosphere, _softwoodCoarseRoots, swlosses->coarseRootToBGFast());
            }
        }

        if (totalHWAgBioCarbon > 0) {
            if (hwlosses->lossesPresent()) {
                // Handle hardwood overmature decline - turnover.			
                overmatureLoss
                    ->addTransfer(_hardwoodMerch, _hardwoodStemSnag, hwlosses->merchToStemSnags())
                    ->addTransfer(_hardwoodOther, _hardwoodBranchSnag, hwlosses->otherToBranchSnag())
                    ->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, hwlosses->fineRootToAGVeryFast())
                    ->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, hwlosses->foliageToAGVeryFast())
                    ->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, hwlosses->fineRootToBGVeryFast())
                    ->addTransfer(_hardwoodOther, _aboveGroundFastSoil, hwlosses->otherToAGFast())
                    ->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, hwlosses->coarseRootToAGFast())
                    ->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, hwlosses->coarseRootToBGFast())
                    ->addTransfer(_atmosphere, _hardwoodMerch, hwlosses->merchToStemSnags())
                    ->addTransfer(_atmosphere, _hardwoodOther, hwlosses->otherToBranchSnag())
                    ->addTransfer(_atmosphere, _hardwoodFineRoots, hwlosses->fineRootToAGVeryFast())
                    ->addTransfer(_atmosphere, _hardwoodFoliage, hwlosses->foliageToAGVeryFast())
                    ->addTransfer(_atmosphere, _hardwoodFineRoots, hwlosses->fineRootToBGVeryFast())
                    ->addTransfer(_atmosphere, _hardwoodOther, hwlosses->otherToAGFast())
                    ->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwlosses->coarseRootToAGFast())
                    ->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwlosses->coarseRootToBGFast());
            }
        }

        if (   (totalSWAgBioCarbon > 0 && swlosses->lossesPresent())
            || (totalHWAgBioCarbon > 0 && hwlosses->lossesPresent())) {
            _landUnitData->submitOperation(overmatureLoss);
            _landUnitData->applyOperations();
        }
    }

	void YieldTableGrowthModule::doHalfGrowth()
	{
		auto growth = _landUnitData->createStockOperation();
		growth
			->addTransfer(_atmosphere, _softwoodMerch, swm * 0.50)
			->addTransfer(_atmosphere, _softwoodOther, swo * 0.50)
			->addTransfer(_atmosphere, _softwoodFoliage, swf * 0.50)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr * 0.50)
			->addTransfer(_atmosphere, _softwoodFineRoots, swfr * 0.50)
			->addTransfer(_atmosphere, _hardwoodMerch, hwm * 0.50)
			->addTransfer(_atmosphere, _hardwoodOther, hwo * 0.50)
			->addTransfer(_atmosphere, _hardwoodFoliage, hwf * 0.50)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr * 0.50)
			->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr * 0.50);
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

	void YieldTableGrowthModule::doTurnover()
	{			
		// Snag turnover.
		auto domTurnover = _landUnitData->createProportionalOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag, _mediumSoil, _stemSnagTurnoverRate)
			->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, _branchSnagTurnoverRate)
			->addTransfer(_hardwoodStemSnag, _mediumSoil, _stemSnagTurnoverRate)
			->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, _branchSnagTurnoverRate);
		_landUnitData->submitOperation(domTurnover);
		_landUnitData->applyOperations();
		
		// Biomass turnover as stock operation.
        bool runDelay = _landUnitData->getVariable("run_delay")->value();
        if (!runDelay){
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

	void YieldTableGrowthModule::addbackBiomassTurnoverAmount()
	{
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
		_landUnitData->applyOperations();
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
