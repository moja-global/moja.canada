#include "moja/modules/cbm/smalltreegrowthmodule.h"
#include "moja/modules/cbm/smalltreegrowthcurve.h"

#include <moja/flint/variable.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {	

    void SmallTreeGrowthModule::configure(const DynamicObject& config) { }

    void SmallTreeGrowthModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &SmallTreeGrowthModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit,      &SmallTreeGrowthModule::onTimingInit,      *this);
		notificationCenter.subscribe(signals::TimingStep,      &SmallTreeGrowthModule::onTimingStep,      *this);
	}

    void SmallTreeGrowthModule::getYieldCurve() {        
		_smallTreeGrowthSW = std::make_shared<SmallTreeGrowthCurve>(SpeciesType::Softwood);
		_smallTreeGrowthHW = nullptr;
    }

    void SmallTreeGrowthModule::doLocalDomainInit() {
		// there is only one softwood small tree growth curve now.
		// it appears as a set of parameters, create one grwoth curve.
		// it should be eco-zone based, and parameters should be updated in doTimingInit routine		
		getYieldCurve();

        _softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
        _softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
        _softwoodStem = _landUnitData->getPool("SoftwoodStem");
        _softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _softwoodOther = _landUnitData->getPool("SoftwoodOther");
        _softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
        _softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");	

		_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
		_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
		_hardwoodStem= _landUnitData->getPool("HardwoodStem");
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

        _age = _landUnitData->getVariable("peatland_smalltree_age");
        _gcId = _landUnitData->getVariable("growth_curve_id");      
        
        _regenDelay = _landUnitData->getVariable("regen_delay");
        _spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
        _isForest = _landUnitData->getVariable("is_forest");
        _isDecaying = _landUnitData->getVariable("is_decaying");

        auto rootParams = _landUnitData->getVariable("root_parameters")->value().extract<DynamicObject>();		

		if (_smallTreeGrowthSW != nullptr) {
			auto swRootEquation = std::make_shared<SoftwoodRootBiomassEquation>(
				rootParams["sw_a"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
			_smallTreeGrowthSW->setRootBiomassEquation(swRootEquation);
		}

		if (_smallTreeGrowthHW != nullptr) {
			_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
			_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
			_hardwoodStem = _landUnitData->getPool("HardwoodStem");
			_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
			_hardwoodOther = _landUnitData->getPool("HardwoodOther");
			_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
			_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

			auto hwRootEquation = std::make_shared<HardwoodRootBiomassEquation>(
				rootParams["hw_a"], rootParams["hw_b"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
			_smallTreeGrowthHW->setRootBiomassEquation(hwRootEquation);
		}
    }

    bool SmallTreeGrowthModule::shouldRun() {	
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();

		int treed_peatland_bog = 2;
		int treed_peatland_poorfen = 5;
		int treed_peatland_richfen = 8;
		int treed_peatland_swamp = 10;		

		bool treedPeatland = true;

		//apply to treed peatland only
		int peatlandId = _landUnitData->getVariable("peatlandId")->value();
		if (!(peatlandId == treed_peatland_bog			// treed bog
			|| peatlandId == treed_peatland_poorfen		// treed poor fen
			|| peatlandId == treed_peatland_richfen		// treed rich fen
			|| peatlandId == treed_peatland_swamp)) {	// treed swamp
			treedPeatland = false;
		}

		_shouldRun = (runPeatland && treedPeatland);
		return _shouldRun;
    }

    void SmallTreeGrowthModule::doTimingInit() {
		if (!shouldRun()) {
			return;
		}
		//The small tree parameters are eco-zone based, get the current eco_boundary variable name
		//if eco_boundary name changed or just set, small tree growth curve parameter needs to be updated
		auto ecoBoundaryName = _landUnitData->getVariable("eco_boundary")->value();

		const auto& sw_smallTreeGrowthParams = _landUnitData->getVariable("sw_smallTree_growth_parameters")->value();
		_smallTreeGrowthSW->checkUpdateEcoParameters(ecoBoundaryName, sw_smallTreeGrowthParams.extract<DynamicObject>());
		
		_turnoverRates = _landUnitData->getVariable("turnover_rates");
        const auto& turnoverRates = _turnoverRates->value().extract<DynamicObject>();
        _otherToBranchSnagSplit = turnoverRates["other_to_branch_snag_split"];
		_stemAnnualTurnOverRate = turnoverRates["stem_annual_turnover_rate"];
        _stemSnagTurnoverRate = turnoverRates["stem_snag_turnover_rate"];
        _branchSnagTurnoverRate = turnoverRates["branch_snag_turnover_rate"];
        _coarseRootSplit = turnoverRates["coarse_root_split"];
        _coarseRootTurnProp = turnoverRates["coarse_root_turn_prop"];
        _fineRootAGSplit = turnoverRates["fine_root_ag_split"];
        _fineRootTurnProp = turnoverRates["fine_root_turn_prop"];
        
		_softwoodFoliageFallRate = turnoverRates["softwood_foliage_fall_rate"];
		_softwoodBranchTurnOverRate = turnoverRates["softwood_branch_turnover_rate"];

		_hardwoodFoliageFallRate = turnoverRates["hardwood_foliage_fall_rate"];
		_hardwoodBranchTurnOverRate = turnoverRates["hardwood_branch_turnover_rate"];		
    }	

	void SmallTreeGrowthModule::doTimingStep() {
		if (!_shouldRun || (_spinupMossOnly->value() == true)) {
			return;
		}			

        // Get current biomass pool values.
        updateBiomassPools();
        softwoodStemSnag = _softwoodStemSnag->value();
        softwoodBranchSnag = _softwoodBranchSnag->value();
        hardwoodStemSnag = _hardwoodStemSnag->value();
        hardwoodBranchSnag = _hardwoodBranchSnag->value();       

		getIncrements();	  // 1) get and store the biomass carbon growth increments
        doHalfGrowth();		  // 2) transfer half of the biomass growth increment to the biomass pool
        updateBiomassPools(); // 3) update to record the current biomass pool value plus the half increment of biomass
        doMidSeasonGrowth();  // 4) the foliage and snags that grow and are turned over

		//temp to print out the removal from live biomass components
		/*printRemovals(_age->value(), 
					standSoftwoodStem * _stemAnnualTurnOverRate, 
					standSoftwoodFoliage * _softwoodFoliageFallRate, 
					standSoftwoodOther * _softwoodBranchTurnOverRate, 
					standSWCoarseRootsCarbon * _coarseRootTurnProp, 
					standSWFineRootsCarbon * _fineRootTurnProp);*/

        doTurnover();		  // 5) do biomass and snag turnover
        doHalfGrowth();		  // 6) transfer the remaining half increment to the biomass pool

		//TODO, get the age of the leading tree species, update it 
        int standAge = _age->value();
        _age->set_value(standAge + 1);
    }

	void SmallTreeGrowthModule::getIncrements() {
		//get current stand age
		int age = _age->value();

		//get the increment from this age
		if (_smallTreeGrowthSW != nullptr) {
			auto sw_increments = _smallTreeGrowthSW->getSmallTreeBiomassCarbonIncrements(standSoftwoodStem, standSoftwoodOther, 
													standSoftwoodFoliage, standSWCoarseRootsCarbon, standSWFineRootsCarbon, age);
			sws = sw_increments["stemwood"];
			swo = sw_increments["other"];
			swf = sw_increments["foliage"];
			swcr = sw_increments["coarseRoot"];
			swfr = sw_increments["fineRoot"];
		}

		if (_smallTreeGrowthHW != nullptr) {			
			auto hw_increments = _smallTreeGrowthHW->getSmallTreeBiomassCarbonIncrements(standHardwoodStem, standHardwoodOther, 
													standHardwoodFoliage, standHWCoarseRootsCarbon, standHWFineRootsCarbon, age);
			hws = hw_increments["stemwood"];
			hwo = hw_increments["other"];
			hwf = hw_increments["foliage"];
			hwcr = hw_increments["coarseRoot"];
			hwfr = hw_increments["fineRoot"];
		}
	}

    void SmallTreeGrowthModule::doHalfGrowth() const {
        static double tolerance = -0.0001;
        auto growth = _landUnitData->createStockOperation();

        double swOvermature = sws + swo + swf + swcr + swfr < tolerance;
        if (swOvermature && sws < 0) {
            growth->addTransfer(_softwoodStem, _softwoodStemSnag, -sws / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodStem, sws / 2);
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

		if (_smallTreeGrowthHW != nullptr){
			double hwOvermature = hws + hwo + hwf + hwcr + hwfr < tolerance;
			if (hwOvermature && hws < 0) {
				growth->addTransfer(_hardwoodStem, _hardwoodStemSnag, -hws / 2);
			} else {
				growth->addTransfer(_atmosphere, _hardwoodStem, hws / 2);
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
		}
        _landUnitData->submitOperation(growth);
        _landUnitData->applyOperations();
    }

    void SmallTreeGrowthModule::updateBiomassPools() {
		standSoftwoodStem = _softwoodStem->value();
        standSoftwoodOther = _softwoodOther->value();
        standSoftwoodFoliage = _softwoodFoliage->value();
        standSWCoarseRootsCarbon = _softwoodCoarseRoots->value();
        standSWFineRootsCarbon = _softwoodFineRoots->value();

        standHardwoodStem = _hardwoodStem->value();
        standHardwoodOther = _hardwoodOther->value();
        standHardwoodFoliage = _hardwoodFoliage->value();
        standHWCoarseRootsCarbon = _hardwoodCoarseRoots->value();
        standHWFineRootsCarbon = _hardwoodFineRoots->value();
    }

    void SmallTreeGrowthModule::doTurnover() const {			
        // Snag turnover.
        auto domTurnover = _landUnitData->createStockOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag, _mediumSoil, softwoodStemSnag * _stemSnagTurnoverRate)
			->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, softwoodBranchSnag * _branchSnagTurnoverRate);

		if (_smallTreeGrowthHW != nullptr) {
			domTurnover
				->addTransfer(_hardwoodStemSnag, _mediumSoil, hardwoodStemSnag * _stemSnagTurnoverRate)
				->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, hardwoodBranchSnag * _branchSnagTurnoverRate);
			_landUnitData->submitOperation(domTurnover);
		}
        
        // Biomass turnover as stock operation.
        auto bioTurnover = _landUnitData->createStockOperation();
		bioTurnover
			->addTransfer(_softwoodStem, _softwoodStemSnag, standSoftwoodStem * _stemAnnualTurnOverRate)
			->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, standSoftwoodFoliage * _softwoodFoliageFallRate)
			->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _otherToBranchSnagSplit * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodOther, _aboveGroundFastSoil, standSoftwoodOther * (1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp);		

		if (_smallTreeGrowthHW != nullptr) {
			bioTurnover
				->addTransfer(_hardwoodStem, _hardwoodStemSnag, standHardwoodStem * _stemAnnualTurnOverRate)
				->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage *_hardwoodFoliageFallRate)
				->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _otherToBranchSnagSplit * _hardwoodBranchTurnOverRate)
				->addTransfer(_hardwoodOther, _aboveGroundFastSoil, standHardwoodOther * (1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
				->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
				->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
				->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
				->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp);
		}
        _landUnitData->submitOperation(bioTurnover);
    }

    void SmallTreeGrowthModule::doMidSeasonGrowth() const {
        auto seasonalGrowth = _landUnitData->createStockOperation();
		seasonalGrowth
			->addTransfer(_atmosphere, _softwoodStem, standSoftwoodStem * _stemAnnualTurnOverRate)
			->addTransfer(_atmosphere, _softwoodOther, standSoftwoodOther * _softwoodBranchTurnOverRate)
			->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _softwoodFoliageFallRate)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _coarseRootTurnProp)
			->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _fineRootTurnProp);

		if (_smallTreeGrowthHW != nullptr) {
			seasonalGrowth
			->addTransfer(_atmosphere, _hardwoodStem, standHardwoodStem * _stemAnnualTurnOverRate)
				->addTransfer(_atmosphere, _hardwoodOther, standHardwoodOther * _hardwoodBranchTurnOverRate)
				->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _hardwoodFoliageFallRate)
				->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _coarseRootTurnProp)
				->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _fineRootTurnProp);
		}
        _landUnitData->submitOperation(seasonalGrowth);
    }	

	void SmallTreeGrowthModule::printRemovals(int age, double standSoftwoodStem, double standSoftwoodFoliage, double standSoftwoodOther, double standSWCoarseRootsCarbon, double standSWFineRootsCarbon) {
		MOJA_LOG_INFO <<age <<"," << standSoftwoodStem << "," << standSoftwoodFoliage << "," << standSoftwoodOther << "," << standSWCoarseRootsCarbon << "," << standSWFineRootsCarbon;
	}
}}}
