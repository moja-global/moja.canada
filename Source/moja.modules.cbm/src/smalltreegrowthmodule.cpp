#include "moja/modules/cbm/smalltreegrowthmodule.h"
#include "moja/modules/cbm/smalltreegrowthcurve.h"

#include <moja/flint/variable.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/logging.h>

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

		_atmosphere = _landUnitData->getPool("Atmosphere");

        _softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
        _softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
        _softwoodStem = _landUnitData->getPool("SoftwoodStem");
        _softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _softwoodOther = _landUnitData->getPool("SoftwoodOther");
        _softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
        _softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");	          

		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");

		_smalltree_age = _landUnitData->getVariable("peatland_smalltree_age");
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
		if (!runPeatland) {
			return false;
		}

		int treed_peatland_bog = 2;
		int treed_peatland_poorfen = 5;
		int treed_peatland_richfen = 8;
		int treed_peatland_swamp = 10;		

		//apply to treed peatland only
		int peatlandId = _landUnitData->getVariable("peatlandId")->value();
		_treedPeatland = 
			(peatlandId == treed_peatland_bog ||		
			 peatlandId == treed_peatland_poorfen ||	
			 peatlandId == treed_peatland_richfen ||	
			 peatlandId == treed_peatland_swamp);			

		_shouldRun = (runPeatland && _treedPeatland);
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
        standSoftwoodStemSnag = _softwoodStemSnag->value();
        standSoftwoodBranchSnag = _softwoodBranchSnag->value();
		if (_smallTreeGrowthHW != nullptr){
			standHardwoodStemSnag = _hardwoodStemSnag->value();
			standHardwoodBranchSnag = _hardwoodBranchSnag->value();     
		}

		getIncrements();	  // 1) get and store the biomass carbon growth increments
        doHalfGrowth();		  // 2) transfer half of the biomass growth increment to the biomass pool
        updateBiomassPools(); // 3) update to record the current biomass pool value plus the half increment of biomass
        doMidSeasonGrowth();  // 4) the foliage and snags that grow and are turned over

		//debug to print out the removal from live biomass components
		/*printRemovals(_age->value(), 
					standSoftwoodStem * _stemAnnualTurnOverRate, 
					standSoftwoodFoliage * _softwoodFoliageFallRate, 
					standSoftwoodOther * _softwoodBranchTurnOverRate, 
					standSWCoarseRootsCarbon * _coarseRootTurnProp, 
					standSWFineRootsCarbon * _fineRootTurnProp);*/

		doPeatlandTurnover(); // 5) do biomass and snag turnover, small tree is in treed peatland only     
        doHalfGrowth();		  // 6) transfer the remaining half increment to the biomass pool
		
        int standSmallTreeAge = _smalltree_age->value();
		_smalltree_age->set_value(standSmallTreeAge + 1);
    }

	void SmallTreeGrowthModule::getIncrements() {
		//get current small tree age
		int smallTreeAge = _smalltree_age->value();

		//get the increment from this age
		if (_smallTreeGrowthSW != nullptr) {
			auto sw_increments = _smallTreeGrowthSW->getSmallTreeBiomassCarbonIncrements(standSoftwoodStem, standSoftwoodOther, 
													standSoftwoodFoliage, standSWCoarseRootsCarbon, standSWFineRootsCarbon, smallTreeAge);
			sws = sw_increments["stemwood"];
			swo = sw_increments["other"];
			swf = sw_increments["foliage"];
			swcr = sw_increments["coarseRoot"];
			swfr = sw_increments["fineRoot"];
		}

		if (_smallTreeGrowthHW != nullptr) {			
			auto hw_increments = _smallTreeGrowthHW->getSmallTreeBiomassCarbonIncrements(standHardwoodStem, standHardwoodOther, 
													standHardwoodFoliage, standHWCoarseRootsCarbon, standHWFineRootsCarbon, smallTreeAge);
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
            growth->addTransfer(_softwoodOther, _woodyFineDead, -swo * (1 - _otherToBranchSnagSplit) / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodOther, swo / 2);
        }

        if (swOvermature && swf < 0) {
            growth->addTransfer(_softwoodFoliage, _woodyFoliageDead, -swf / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodFoliage, swf / 2);
        }

        if (swOvermature && swcr < 0) {
            growth->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, -swcr / 2);
        } else {
            growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr / 2);
        }

        if (swOvermature && swfr < 0) {
            growth->addTransfer(_softwoodFineRoots, _woodyRootsDead, -swfr / 2);
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
				growth->addTransfer(_hardwoodOther, _woodyFineDead, -hwo * (1 - _otherToBranchSnagSplit) / 2);
			} else {
				growth->addTransfer(_atmosphere, _hardwoodOther, hwo / 2);
			}

			if (hwOvermature && hwf < 0) {
				growth->addTransfer(_hardwoodFoliage, _woodyFoliageDead, -hwf / 2);
			} else {
				growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf / 2);
			}

			if (hwOvermature && hwcr < 0) {
				growth->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, -hwcr / 2 );
			} else {
				growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr / 2);
			}

			if (hwOvermature && hwfr < 0) {
				growth->addTransfer(_hardwoodFineRoots, _woodyRootsDead, -hwfr / 2 );
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

		if (_smallTreeGrowthHW != nullptr) {
			standHardwoodStem = _hardwoodStem->value();
			standHardwoodOther = _hardwoodOther->value();
			standHardwoodFoliage = _hardwoodFoliage->value();
			standHWCoarseRootsCarbon = _hardwoodCoarseRoots->value();
			standHWFineRootsCarbon = _hardwoodFineRoots->value();
		}
    }

	void SmallTreeGrowthModule::doPeatlandTurnover() const {
        // Snag turnover.
        auto domTurnover = _landUnitData->createStockOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag, _woodyFineDead, standSoftwoodStemSnag * _stemSnagTurnoverRate)
			->addTransfer(_softwoodBranchSnag, _woodyFineDead, standSoftwoodBranchSnag * _branchSnagTurnoverRate);

		if (_smallTreeGrowthHW != nullptr) {
			domTurnover
				->addTransfer(_hardwoodStemSnag, _woodyFineDead, standHardwoodStemSnag * _stemSnagTurnoverRate)
				->addTransfer(_hardwoodBranchSnag, _woodyFineDead, standHardwoodBranchSnag * _branchSnagTurnoverRate);
		}
			_landUnitData->submitOperation(domTurnover);

        
        // Biomass turnover as stock operation.
        auto bioTurnover = _landUnitData->createStockOperation();
		bioTurnover
			->addTransfer(_softwoodStem, _softwoodBranchSnag, standSoftwoodStem * _stemAnnualTurnOverRate)
			->addTransfer(_softwoodFoliage, _woodyFoliageDead, standSoftwoodFoliage * _softwoodFoliageFallRate)
			->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _otherToBranchSnagSplit * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodOther, _woodyFineDead, standSoftwoodOther * (1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, standSWCoarseRootsCarbon * _coarseRootTurnProp)
			->addTransfer(_softwoodFineRoots, _woodyRootsDead, standSWFineRootsCarbon * _fineRootTurnProp);
		
		if (_smallTreeGrowthHW != nullptr) {
			bioTurnover
				->addTransfer(_hardwoodStem, _hardwoodBranchSnag, standHardwoodStem * _stemAnnualTurnOverRate)
					->addTransfer(_hardwoodFoliage, _woodyFoliageDead, standHardwoodFoliage * _hardwoodFoliageFallRate)
					->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _otherToBranchSnagSplit * _hardwoodBranchTurnOverRate)
					->addTransfer(_hardwoodOther, _woodyFineDead, standHardwoodOther * (1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
					->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, standHWCoarseRootsCarbon * _coarseRootTurnProp)
					->addTransfer(_hardwoodFineRoots, _woodyRootsDead, standHWFineRootsCarbon * _fineRootTurnProp);
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
