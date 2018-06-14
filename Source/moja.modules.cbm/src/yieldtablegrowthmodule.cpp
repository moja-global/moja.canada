#include "moja/modules/cbm/yieldtablegrowthmodule.h"

#include <moja/flint/variable.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {	

    void YieldTableGrowthModule::configure(const DynamicObject& config) { }

    void YieldTableGrowthModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &YieldTableGrowthModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit,      &YieldTableGrowthModule::onTimingInit,      *this);
		notificationCenter.subscribe(signals::TimingStep,      &YieldTableGrowthModule::onTimingStep,      *this);
	}

    void YieldTableGrowthModule::getYieldCurve() {
        // Get the stand growth curve ID associated to the pixel/svo.
        const auto& standGrowthCurveID = _gcId->value();
        _standGrowthCurveID = standGrowthCurveID.isEmpty() ? -1 : standGrowthCurveID.convert<Int64>();
        _isDecaying->set_value(_standGrowthCurveID != -1);

        // Try to get the stand growth curve and related yield table data from memory.
        bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(
            _standGrowthCurveID, _standSPUID);

        if (!carbonCurveFound) {
			//call the stand growth curve factory to create the stand growth curve
			auto standGrowthCurve = _gcFactory->createStandGrowthCurve(
					_standGrowthCurveID, _standSPUID, *_landUnitData);
   
            // Process and convert yield volume to carbon curves.
            _volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);
        }
    }

    void YieldTableGrowthModule::doLocalDomainInit() {
		_growthMultipliersEnabled = _landUnitData->hasVariable("current_growth_multipliers");
		if (_growthMultipliersEnabled) {
			_growthMultipliers = _landUnitData->getVariable("current_growth_multipliers");
		}

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
        _gcId = _landUnitData->getVariable("growth_curve_id");
        _spuId = _landUnitData->getVariable("spatial_unit_id");
        _turnoverRates = _landUnitData->getVariable("turnover_rates");
        _regenDelay = _landUnitData->getVariable("regen_delay");
        _spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
        _isForest = _landUnitData->getVariable("is_forest");
        _isDecaying = _landUnitData->getVariable("is_decaying");

        auto rootParams = _landUnitData->getVariable("root_parameters")->value().extract<DynamicObject>();
        _volumeToBioGrowth = std::make_shared<VolumeToBiomassCarbonGrowth>(std::vector<ForestTypeConfiguration>{
            ForestTypeConfiguration{
                "Softwood",
                _age,
                std::make_shared<SoftwoodRootBiomassEquation>(
                    rootParams["sw_a"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]),
                _softwoodMerch, _softwoodOther, _softwoodFoliage, _softwoodCoarseRoots, _softwoodFineRoots
            },
            ForestTypeConfiguration{
                "Hardwood",
                _age,
                std::make_shared<HardwoodRootBiomassEquation>(
                    rootParams["hw_a"], rootParams["hw_b"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]),
                _hardwoodMerch, _hardwoodOther, _hardwoodFoliage, _hardwoodCoarseRoots, _hardwoodFineRoots
            }
        });
    }

    bool YieldTableGrowthModule::shouldRun() const {
        bool isForest = _isForest->value();
        bool hasGrowthCurve = _standGrowthCurveID != -1;

        return isForest && hasGrowthCurve;
    }

    void YieldTableGrowthModule::doTimingInit() {
        const auto& turnoverRates = _turnoverRates->value().extract<DynamicObject>();
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
        _standSPUID = _spuId->value();
		initPeatland();
    }

	void YieldTableGrowthModule::initPeatland() {
		_skipForPeatland = false;
		if (!_landUnitData->hasVariable("run_peatland")) {
			return;
		}

		bool isPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!isPeatland) {
			return;
		}

		int peatlandId = _landUnitData->getVariable("peatlandId")->value();

		//if peatland is of foresty type, aka, peatland_id is one of the following number
		//run the forest growth module
		int forest_peatland_bog = 3;
		int forest_peatland_poorfen = 6;
		int forest_peatland_richfen = 9;
		int forest_peatland_swamp = 11;

		bool forestedPeatland = (peatlandId == forest_peatland_bog
			|| peatlandId == forest_peatland_poorfen
			|| peatlandId == forest_peatland_richfen
			|| peatlandId == forest_peatland_swamp);

		_skipForPeatland = (isPeatland && (!forestedPeatland));
	}


	void YieldTableGrowthModule::doTimingStep() {
		if (_skipForPeatland) {
			return;
		}

        int regenDelay = _regenDelay->value();
        if (regenDelay > 0) {
            _regenDelay->set_value(--regenDelay);
            return;
        }

        // When moss module is spinning up, nothing to grow, turnover and decay.
        bool spinupMossOnly = _spinupMossOnly->value();
        if (spinupMossOnly) {
            return;
        }

        getYieldCurve();

        // Get current biomass pool values.
        updateBiomassPools();
        softwoodStemSnag = _softwoodStemSnag->value();
        softwoodBranchSnag = _softwoodBranchSnag->value();
        hardwoodStemSnag = _hardwoodStemSnag->value();
        hardwoodBranchSnag = _hardwoodBranchSnag->value();

        if (_landUnitData->hasVariable("delay")) { // Following delay and isLastRotation are for spinup only.
            int delay = _landUnitData->getVariable("delay")->value();
            bool runDelay = _landUnitData->getVariable("run_delay")->value();

            // When the last rotation is done, and the delay is defined, do turnover and following decay.
            if (runDelay && delay > 0) {
                updateBiomassPools();
                doMidSeasonGrowth();
                doTurnover();

                // No growth in delay period.
                return;
            }
        }

        if (!shouldRun()) {
            return;
        }

		getIncrements();	  // 1) get and store the biomass carbon growth increments
        doHalfGrowth();		  // 2) transfer half of the biomass growth increment to the biomass pool
        updateBiomassPools(); // 3) update to record the current biomass pool value plus the half increment of biomass
        doMidSeasonGrowth();  // 4) the foliage and snags that grow and are turned over
        doTurnover();		  // 5) do biomass and snag turnover
        doHalfGrowth();		  // 6) transfer the remaining half increment to the biomass pool

        int standAge = _age->value();
        _age->set_value(standAge + 1);
    }

	void YieldTableGrowthModule::getIncrements() {
		auto increments = _volumeToBioGrowth->getBiomassCarbonIncrements(
			_standGrowthCurveID, _standSPUID);

		swm = increments["SoftwoodMerch"];
		swo = increments["SoftwoodOther"];
		swf = increments["SoftwoodFoliage"];
		swcr = increments["SoftwoodCoarseRoots"];
		swfr = increments["SoftwoodFineRoots"];

		hwm = increments["HardwoodMerch"];
		hwo = increments["HardwoodOther"];
		hwf = increments["HardwoodFoliage"];
		hwcr = increments["HardwoodCoarseRoots"];
		hwfr = increments["HardwoodFineRoots"];

		if (!_growthMultipliersEnabled) {
			return;
		}

		auto growthMultipliers = _growthMultipliers->value();
		if (growthMultipliers.size() == 0) {
			return;
		}

		auto multipliers = growthMultipliers.extract<
			std::unordered_map<std::string, double>>();

		auto newSwMult = multipliers.find("Softwood");
		if (newSwMult != multipliers.end()) {
			double swMultiplier = newSwMult->second;
			swm  *= swMultiplier;
			swo  *= swMultiplier;
			swf  *= swMultiplier;
			swcr *= swMultiplier;
			swfr *= swMultiplier;
		}

		auto newHwMult = multipliers.find("Hardwood");
		if (newHwMult != multipliers.end()) {
			double hwMultiplier = newHwMult->second;
			hwm  *= hwMultiplier;
			hwo  *= hwMultiplier;
			hwf  *= hwMultiplier;
			hwcr *= hwMultiplier;
			hwfr *= hwMultiplier;
		}
	}

    void YieldTableGrowthModule::doHalfGrowth() const {
        static double tolerance = -0.0001;
        auto growth = _landUnitData->createStockOperation();

        double swOvermature = swm + swo + swf + swcr + swfr < tolerance;
        if (swOvermature && swm < 0) {
            growth->addTransfer(_softwoodMerch, _softwoodStemSnag, -swm / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodMerch, swm / 2);
        }

        if (swOvermature && swo < 0) {
            growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * _otherToBranchSnagSplit / 2);
            growth->addTransfer(_softwoodOther, _aboveGroundFastSoil, -swo * (1 - _otherToBranchSnagSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodOther, swo / 2);
        }

        if (swOvermature && swf < 0) {
            growth->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, -swf / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodFoliage, swf / 2);
        }

        if (swOvermature && swcr < 0) {
            growth->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, -swcr * _coarseRootSplit / 2);
            growth->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, -swcr * (1 - _coarseRootSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr / 2);
        }

        if (swOvermature && swfr < 0) {
            growth->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, -swfr * _fineRootAGSplit / 2);
            growth->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, -swfr * (1 - _fineRootAGSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodFineRoots, swfr / 2);
        }

        double hwOvermature = hwm + hwo + hwf + hwcr + hwfr < tolerance;
        if (hwOvermature && hwm < 0) {
            growth->addTransfer(_hardwoodMerch, _hardwoodStemSnag, -hwm / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodMerch, hwm / 2);
        }

        if (hwOvermature && hwo < 0) {
            growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * _otherToBranchSnagSplit / 2);
            growth->addTransfer(_hardwoodOther, _aboveGroundFastSoil, -hwo * (1 - _otherToBranchSnagSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodOther, hwo / 2);
        }

        if (hwOvermature && hwf < 0) {
            growth->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, -hwf / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf / 2);
        }

        if (hwOvermature && hwcr < 0) {
            growth->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, -hwcr * _coarseRootSplit / 2);
            growth->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, -hwcr * (1 - _coarseRootSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr / 2);
        }

        if (hwOvermature && hwfr < 0) {
            growth->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, -hwfr * _fineRootAGSplit / 2);
            growth->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, -hwfr * (1 - _fineRootAGSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr / 2);
        }

        _landUnitData->submitOperation(growth);
        _landUnitData->applyOperations();
    }

    void YieldTableGrowthModule::updateBiomassPools() {
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
        auto bioTurnover = _landUnitData->createStockOperation();
        bioTurnover
            ->addTransfer(_softwoodMerch, _softwoodStemSnag, standSoftwoodMerch * _stemAnnualTurnOverRate)
            ->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, standSoftwoodFoliage * _softwoodFoliageFallRate)
            ->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _otherToBranchSnagSplit * _softwoodBranchTurnOverRate)
            ->addTransfer(_softwoodOther, _aboveGroundFastSoil, standSoftwoodOther * (1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
            ->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
            ->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
            ->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
            ->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp)

            ->addTransfer(_hardwoodMerch, _hardwoodStemSnag, standHardwoodMerch * _stemAnnualTurnOverRate)
            ->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage *_hardwoodFoliageFallRate)
            ->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _otherToBranchSnagSplit * _hardwoodBranchTurnOverRate)
            ->addTransfer(_hardwoodOther, _aboveGroundFastSoil, standHardwoodOther * (1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
            ->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
            ->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
            ->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
            ->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp);
        _landUnitData->submitOperation(bioTurnover);
    }

    void YieldTableGrowthModule::doMidSeasonGrowth() const {
        auto seasonalGrowth = _landUnitData->createStockOperation();
        seasonalGrowth
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
        _landUnitData->submitOperation(seasonalGrowth);
    }	

    std::shared_ptr<StandGrowthCurve> YieldTableGrowthModule::createStandGrowthCurve(
        Int64 standGrowthCurveID, Int64 spuID) const {

        auto standGrowthCurve = std::make_shared<StandGrowthCurve>(standGrowthCurveID, spuID);
        return standGrowthCurve;
    }    

}}}
