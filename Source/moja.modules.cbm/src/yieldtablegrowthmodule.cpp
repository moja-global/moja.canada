#include "moja/modules/cbm/yieldtablegrowthmodule.h"

#include <moja/flint/variable.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/itiming.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {	

    void YieldTableGrowthModule::configure(const DynamicObject& config) {
        if (config.contains("smoother_enabled")) {
            _smootherEnabled = config["smoother_enabled"];
            _volumeToBioGrowth->setSmoothing(_smootherEnabled);
        }

        if (config.contains("debugging_enabled")) {
            _debuggingEnabled = config["debugging_enabled"];
        }
    }

    void YieldTableGrowthModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &YieldTableGrowthModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit,      &YieldTableGrowthModule::onTimingInit,      *this);
		notificationCenter.subscribe(signals::TimingStep,      &YieldTableGrowthModule::onTimingStep,      *this);
	}

    void YieldTableGrowthModule::getYieldCurve() {
        // Get the stand growth curve ID associated to the pixel/svo.
        const auto& standGrowthCurveID = _gcId->value();
        _standGrowthCurveID = standGrowthCurveID.isEmpty() ? -1 : standGrowthCurveID.convert<Int64>();
        if (_standGrowthCurveID == -1) {
            _isDecaying->set_value(false);
        }

        // Try to get the stand growth curve and related yield table data from memory.
        bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(
            _standGrowthCurveID, _standSPUID);

        if (!carbonCurveFound) {
			// Call the stand growth curve factory to create the stand growth curve.
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
	
        if (_landUnitData->hasVariable("enable_peatland") && 
			_landUnitData->getVariable("enable_peatland")->value().extract<bool>()) {
            _woodyFineDead = _landUnitData->getPool("WoodyFineDead");
            _woodyCoarseDead = _landUnitData->getPool("WoodyCoarseDead");
            _woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
            _woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");
        }

        _age = _landUnitData->getVariable("age");
        _gcId = _landUnitData->getVariable("growth_curve_id");
        _spuId = _landUnitData->getVariable("spatial_unit_id");
        _turnoverRates = _landUnitData->getVariable("turnover_rates");
        _regenDelay = _landUnitData->getVariable("regen_delay");
        _spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
        _isForest = _landUnitData->getVariable("is_forest");
        _isDecaying = _landUnitData->getVariable("is_decaying");
    }

    bool YieldTableGrowthModule::shouldRun() const {
        bool isForest = _isForest->value();
        bool hasGrowthCurve = _standGrowthCurveID != -1;

        return isForest && hasGrowthCurve;
    }

    void YieldTableGrowthModule::doTimingInit() {
        _standSPUID = _spuId->value();

		initPeatland();
    }

	void YieldTableGrowthModule::initPeatland() {	
		if (!_landUnitData->hasVariable("run_peatland")) {
			return;
		}

		bool isPeatland = _landUnitData->getVariable("run_peatland")->value();		
		int peatlandId = _landUnitData->getVariable("peatlandId")->value();

		//if peatland is of foresty type, aka, peatland_id is one of the following IDs	
		int forest_peatland_bog = 3;
		int forest_peatland_poorfen = 6;
		int forest_peatland_richfen = 9;
		int forest_peatland_swamp = 11;

		_forestedPeatland = 
			(peatlandId == forest_peatland_bog ||
			 peatlandId == forest_peatland_poorfen ||
			 peatlandId == forest_peatland_richfen ||
			 peatlandId == forest_peatland_swamp);

		_skipForPeatland = (isPeatland && (!_forestedPeatland));
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
                getTurnoverRates();
                updateBiomassPools();
                doMidSeasonGrowth();
				switchTurnover();

                // No growth in delay period.
                return;
            }
        }

        if (!shouldRun()) {
            return;
        }

		getIncrements();		// 1) get and store the biomass carbon growth increments
        getTurnoverRates();		// 2) get and store the ecoboundary/genus-specific turnover rates
        switchHalfGrowth();		// 3) transfer half of the biomass growth increment to the biomass pool
        updateBiomassPools();	// 4) update to record the current biomass pool value plus the half increment of biomass
        doMidSeasonGrowth();	// 5) the foliage and snags that grow and are turned over
		switchTurnover();		// 6) switch to do biomass and snag turnover for peatland or regular forest land 
		switchHalfGrowth();		// 7) transfer the remaining half increment to the biomass pool

        int standAge = _age->value();
        _age->set_value(standAge + 1);
    }

	void YieldTableGrowthModule::getIncrements() {
		auto increments = _volumeToBioGrowth->getBiomassCarbonIncrements(
			_landUnitData.get(), _standGrowthCurveID, _standSPUID);

        if (increments.size() == 0) {
            MOJA_LOG_INFO << "Warning - growth curve with no increments found: " << _standGrowthCurveID;
        }

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

            if (_debuggingEnabled) {
                MOJA_LOG_DEBUG << (boost::format("Applied softwood multiplier of %1% in year %2%")
                    % swMultiplier % _landUnitData->timing()->curStartDate().year()).str();
            }
		}

		auto newHwMult = multipliers.find("Hardwood");
		if (newHwMult != multipliers.end()) {
			double hwMultiplier = newHwMult->second;
			hwm  *= hwMultiplier;
			hwo  *= hwMultiplier;
			hwf  *= hwMultiplier;
			hwcr *= hwMultiplier;
			hwfr *= hwMultiplier;

            if (_debuggingEnabled) {
                MOJA_LOG_DEBUG << (boost::format("Applied hardwood multiplier of %1% in year %2%")
                    % hwMultiplier % _landUnitData->timing()->curStartDate().year()).str();
            }
        }
	}

    void YieldTableGrowthModule::getTurnoverRates() {
        const auto& turnoverRates = _volumeToBioGrowth->getTurnoverRates(_standGrowthCurveID, _standSPUID);
        _swFoliageTurnover = turnoverRates.swFoliageTurnover();
        _swStemTurnover = turnoverRates.swStemTurnover();
        _swBranchTurnover = turnoverRates.swBranchTurnover();
        _swStemSnagTurnover = turnoverRates.swStemSnagTurnover();
        _swBranchSnagTurnover = turnoverRates.swBranchSnagTurnover();
        _swCoarseRootTurnover = turnoverRates.swCoarseRootTurnover();
        _swFineRootTurnover = turnoverRates.swFineRootTurnover();
        _swBranchSnagSplit = turnoverRates.swBranchSnagSplit();
        _swCoarseRootSplit = turnoverRates.swCoarseRootSplit();
        _swFineRootSplit = turnoverRates.swFineRootSplit();
        _hwFoliageTurnover = turnoverRates.hwFoliageTurnover();
        _hwStemTurnover = turnoverRates.hwStemTurnover();
        _hwBranchTurnover = turnoverRates.hwBranchTurnover();
        _hwStemSnagTurnover = turnoverRates.hwStemSnagTurnover();
        _hwBranchSnagTurnover = turnoverRates.hwBranchSnagTurnover();
        _hwCoarseRootTurnover = turnoverRates.hwCoarseRootTurnover();
        _hwFineRootTurnover = turnoverRates.hwFineRootTurnover();
        _hwBranchSnagSplit = turnoverRates.hwBranchSnagSplit();
        _hwCoarseRootSplit = turnoverRates.hwCoarseRootSplit();
        _hwFineRootSplit = turnoverRates.hwFineRootSplit();
    }

    void YieldTableGrowthModule::doHalfGrowth() const {
        static double tolerance = -0.0001;
        auto growth = _landUnitData->createStockOperation();

        bool swOvermature = swm + swo + swf + swcr + swfr < tolerance;
        if (swOvermature && swm < 0) {
            growth->addTransfer(_softwoodMerch, _softwoodStemSnag, -swm / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodMerch, swm / 2);
        }

        if (swOvermature && swo < 0) {
            growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * _swBranchSnagSplit / 2);
            growth->addTransfer(_softwoodOther, _aboveGroundFastSoil, -swo * (1 - _swBranchSnagSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodOther, swo / 2);
        }

        if (swOvermature && swf < 0) {
            growth->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, -swf / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodFoliage, swf / 2);
        }

        if (swOvermature && swcr < 0) {
            growth->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, -swcr * _swCoarseRootSplit / 2);
            growth->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, -swcr * (1 - _swCoarseRootSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr / 2);
        }

        if (swOvermature && swfr < 0) {
            growth->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, -swfr * _swFineRootSplit / 2);
            growth->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, -swfr * (1 - _swFineRootSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodFineRoots, swfr / 2);
        }

        bool hwOvermature = hwm + hwo + hwf + hwcr + hwfr < tolerance;
        if (hwOvermature && hwm < 0) {
            growth->addTransfer(_hardwoodMerch, _hardwoodStemSnag, -hwm / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodMerch, hwm / 2);
        }

        if (hwOvermature && hwo < 0) {
            growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * _hwBranchSnagSplit / 2);
            growth->addTransfer(_hardwoodOther, _aboveGroundFastSoil, -hwo * (1 - _hwBranchSnagSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodOther, hwo / 2);
        }

        if (hwOvermature && hwf < 0) {
            growth->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, -hwf / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf / 2);
        }

        if (hwOvermature && hwcr < 0) {
            growth->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, -hwcr * _hwCoarseRootSplit / 2);
            growth->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, -hwcr * (1 - _hwCoarseRootSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr / 2);
        }

        if (hwOvermature && hwfr < 0) {
            growth->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, -hwfr * _hwFineRootSplit / 2);
            growth->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, -hwfr * (1 - _hwFineRootSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr / 2);
        }

        _landUnitData->submitOperation(growth);
        _landUnitData->applyOperations();
    }

	void YieldTableGrowthModule::doPeatlandHalfGrowth() const {
		static double tolerance = -0.0001;
		auto growth = _landUnitData->createStockOperation();

		double swOvermature = swm + swo + swf + swcr + swfr < tolerance;
		if (swOvermature && swm < 0) {
			growth->addTransfer(_softwoodMerch, _softwoodStemSnag, -swm / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _softwoodMerch, swm / 2);
		}

		if (swOvermature && swo < 0) {
			growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * _swBranchSnagSplit / 2);
			growth->addTransfer(_softwoodOther, _woodyFineDead, -swo * (1 - _swBranchSnagSplit) / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _softwoodOther, swo / 2);
		}

		if (swOvermature && swf < 0) {
			growth->addTransfer(_softwoodFoliage, _woodyFoliageDead, -swf / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _softwoodFoliage, swf / 2);
		}

		if (swOvermature && swcr < 0) {
			growth->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, -swcr / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr / 2);
		}

		if (swOvermature && swfr < 0) {
			growth->addTransfer(_softwoodFineRoots, _woodyRootsDead, -swfr / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _softwoodFineRoots, swfr / 2);
		}

		double hwOvermature = hwm + hwo + hwf + hwcr + hwfr < tolerance;
		if (hwOvermature && hwm < 0) {
			growth->addTransfer(_hardwoodMerch, _hardwoodStemSnag, -hwm / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _hardwoodMerch, hwm / 2);
		}

		if (hwOvermature && hwo < 0) {
			growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * _hwBranchSnagSplit / 2);
			growth->addTransfer(_hardwoodOther, _woodyFineDead, -hwo * (1 - _hwBranchSnagSplit) / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _hardwoodOther, hwo / 2);
		}

		if (hwOvermature && hwf < 0) {
			growth->addTransfer(_hardwoodFoliage, _woodyFoliageDead, -hwf / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf / 2);
		}

		if (hwOvermature && hwcr < 0) {
			growth->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, -hwcr / 2);
		}
		else {
			growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr / 2);
		}

		if (hwOvermature && hwfr < 0) {
			growth->addTransfer(_hardwoodFineRoots, _woodyRootsDead, -hwfr / 2);
		}
		else {
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

	void YieldTableGrowthModule::switchTurnover() const{
		if (_forestedPeatland) {
			doPeatlandTurnover();
		}
		else {
			doTurnover();
		}
	}

	void YieldTableGrowthModule::switchHalfGrowth() const {
		if (_forestedPeatland) {
			doPeatlandHalfGrowth();
		}
		else {
			doHalfGrowth();
		}
	}

    void YieldTableGrowthModule::doTurnover() const {			
        // Snag turnover.
        auto domTurnover = _landUnitData->createStockOperation();
        domTurnover
            ->addTransfer(_softwoodStemSnag, _mediumSoil, softwoodStemSnag * _swStemSnagTurnover)
            ->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, softwoodBranchSnag * _swBranchSnagTurnover)
            ->addTransfer(_hardwoodStemSnag, _mediumSoil, hardwoodStemSnag * _hwStemSnagTurnover)
            ->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, hardwoodBranchSnag * _hwBranchSnagTurnover);
        _landUnitData->submitOperation(domTurnover);
        
        // Biomass turnover as stock operation.
        auto bioTurnover = _landUnitData->createStockOperation();
        bioTurnover
            ->addTransfer(_softwoodMerch, _softwoodStemSnag, standSoftwoodMerch * _swStemTurnover)
            ->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, standSoftwoodFoliage * _swFoliageTurnover)
            ->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _swBranchSnagSplit * _swBranchTurnover)
            ->addTransfer(_softwoodOther, _aboveGroundFastSoil, standSoftwoodOther * (1 - _swBranchSnagSplit) * _swBranchTurnover)
            ->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _swCoarseRootSplit * _swCoarseRootTurnover)
            ->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _swCoarseRootSplit) * _swCoarseRootTurnover)
            ->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon * _swFineRootSplit * _swFineRootTurnover)
            ->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _swFineRootSplit) * _swFineRootTurnover)

            ->addTransfer(_hardwoodMerch, _hardwoodStemSnag, standHardwoodMerch * _hwStemTurnover)
            ->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage * _hwFoliageTurnover)
            ->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _hwBranchSnagSplit * _hwBranchTurnover)
            ->addTransfer(_hardwoodOther, _aboveGroundFastSoil, standHardwoodOther * (1 - _hwBranchSnagSplit) * _hwBranchTurnover)
            ->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon * _hwCoarseRootSplit * _hwCoarseRootTurnover)
            ->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon * (1 - _hwCoarseRootSplit) * _hwCoarseRootTurnover)
            ->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon * _hwFineRootSplit * _hwFineRootTurnover)
            ->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _hwFineRootSplit) * _hwFineRootTurnover);
        _landUnitData->submitOperation(bioTurnover);
    }

	void YieldTableGrowthModule::doPeatlandTurnover() const {
		auto domTurnover = _landUnitData->createStockOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag, _woodyCoarseDead, softwoodStemSnag * _swStemSnagTurnover)
			->addTransfer(_softwoodBranchSnag, _woodyFineDead, softwoodBranchSnag * _swBranchSnagTurnover)
			->addTransfer(_hardwoodStemSnag, _woodyCoarseDead, hardwoodStemSnag * _hwStemSnagTurnover)
			->addTransfer(_hardwoodBranchSnag, _woodyFineDead, hardwoodBranchSnag * _hwBranchSnagTurnover);
		_landUnitData->submitOperation(domTurnover);

		auto bioTurnover = _landUnitData->createStockOperation();
		bioTurnover
			->addTransfer(_softwoodMerch, _softwoodStemSnag, standSoftwoodMerch * _swStemTurnover)
			->addTransfer(_softwoodFoliage, _woodyFoliageDead, standSoftwoodFoliage * _swFoliageTurnover)
			->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _swBranchSnagSplit * _swBranchTurnover)
			->addTransfer(_softwoodOther, _woodyFineDead, standSoftwoodOther * (1 - _swBranchSnagSplit) * _swBranchTurnover)
			->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, standSWCoarseRootsCarbon * _swCoarseRootTurnover)			
			->addTransfer(_softwoodFineRoots, _woodyRootsDead, standSWFineRootsCarbon * _swFineRootTurnover)
			->addTransfer(_hardwoodMerch, _hardwoodStemSnag, standHardwoodMerch * _hwStemTurnover)
			->addTransfer(_hardwoodFoliage, _woodyFoliageDead, standHardwoodFoliage * _hwFoliageTurnover)
			->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _hwBranchSnagSplit * _hwBranchTurnover)
			->addTransfer(_hardwoodOther, _woodyFineDead, standHardwoodOther * (1 - _hwBranchSnagSplit) * _hwBranchTurnover)
			->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, standHWCoarseRootsCarbon * _hwCoarseRootTurnover)
			->addTransfer(_hardwoodFineRoots, _woodyRootsDead, standHWFineRootsCarbon * _hwFineRootTurnover);
		_landUnitData->submitOperation(bioTurnover);
	}

    void YieldTableGrowthModule::doMidSeasonGrowth() const {
        auto seasonalGrowth = _landUnitData->createStockOperation();
        seasonalGrowth
            ->addTransfer(_atmosphere, _softwoodMerch, standSoftwoodMerch * _swStemTurnover)
            ->addTransfer(_atmosphere, _softwoodOther, standSoftwoodOther * _swBranchTurnover)
            ->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _swFoliageTurnover)
            ->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _swCoarseRootTurnover)
            ->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _swFineRootTurnover)
            ->addTransfer(_atmosphere, _hardwoodMerch, standHardwoodMerch * _hwStemTurnover)
            ->addTransfer(_atmosphere, _hardwoodOther, standHardwoodOther * _hwBranchTurnover)
            ->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _hwFoliageTurnover)
            ->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _hwCoarseRootTurnover)
            ->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _hwFineRootTurnover);		
        _landUnitData->submitOperation(seasonalGrowth);
    }	

    std::shared_ptr<StandGrowthCurve> YieldTableGrowthModule::createStandGrowthCurve(
        Int64 standGrowthCurveID, Int64 spuID) const {

        auto standGrowthCurve = std::make_shared<StandGrowthCurve>(standGrowthCurveID, spuID);
        return standGrowthCurve;
    }  
}}}