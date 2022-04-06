#include "moja/modules/cbm/smalltreegrowthmodule.h"
#include "moja/modules/cbm/smalltreegrowthcurve.h"
#include "moja/modules/cbm/turnoverrates.h"

#include <moja/flint/variable.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ioperation.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <boost/format.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			void SmallTreeGrowthModule::configure(const DynamicObject& config) { }

			void SmallTreeGrowthModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &SmallTreeGrowthModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &SmallTreeGrowthModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &SmallTreeGrowthModule::onTimingStep, *this);
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

				_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
				_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
				_hardwoodStem = _landUnitData->getPool("HardwoodStem");
				_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
				_hardwoodOther = _landUnitData->getPool("HardwoodOther");
				_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
				_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

				_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
				_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
				_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");

				_smalltreeAge = _landUnitData->getVariable("peatland_smalltree_age");

				_regenDelay = _landUnitData->getVariable("regen_delay");
				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
				_isForest = _landUnitData->getVariable("is_forest");
				_isDecaying = _landUnitData->getVariable("is_decaying");


				_spuId = _landUnitData->getVariable("spatial_unit_id");
				_ecoBoundary = _landUnitData->getVariable("eco_boundary");
				_smallTreeGCParameters = _landUnitData->getVariable("smalltree_growth_parameters");
				_appliedGrowthCurveID = _landUnitData->getVariable("growth_curve_id");

				if (_smallTreeGrowthHW != nullptr) {
					_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
					_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
					_hardwoodStem = _landUnitData->getPool("HardwoodStem");
					_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
					_hardwoodOther = _landUnitData->getPool("HardwoodOther");
					_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
					_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");
				}

				if (_landUnitData->hasVariable("output_removal")) {
					_outputRemoval = _landUnitData->getVariable("output_removal");
				}
				else {
					_outputRemoval = nullptr;
				}
			}

			bool SmallTreeGrowthModule::shouldRun() {
				_shouldRun = false;

				if (_spuId->value().isEmpty()) {
					return false;
				}

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					//apply to treed peatland only
					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					bool treedPeatland = (
						_peatlandId == (int)Peatlands::TREED_PEATLAND_BOG ||
						_peatlandId == (int)Peatlands::TREED_PEATLAND_POORFEN ||
						_peatlandId == (int)Peatlands::TREED_PEATLAND_RICHFEN ||
						_peatlandId == (int)Peatlands::TREED_PEATLAND_SWAMP);

					//run this module only for treed-peatland
					_shouldRun = treedPeatland;
				}

				return _shouldRun;
			}

			void SmallTreeGrowthModule::doTimingInit() {
				if (!shouldRun()) {
					return;
				}

				//There is no small tree growth curve ID, but small tree is of black spruce
				//There is a peatland pre-defined forest growth curve of black spruce
				const auto& appliedGcIdValue = _appliedGrowthCurveID->value();
				Int64 blackSpruceTreeGCID = appliedGcIdValue.isEmpty() ? -1 : appliedGcIdValue.convert<Int64>();
				Int64 SPUID = _spuId->value();

				getTurnoverRates(blackSpruceTreeGCID, SPUID);

				//The small tree parameters are eco-zone based, get the current eco_boundary variable name
				//If eco_boundary name changed or just set, small tree growth curve parameter needs to be updated
				auto ecoBoundaryName = _ecoBoundary->value();

				auto& sw_smallTreeGrowthParams = _smallTreeGCParameters->value();
				_smallTreeGrowthSW->checkUpdateEcoParameters(ecoBoundaryName, sw_smallTreeGrowthParams.extract<DynamicObject>());
			}

			void SmallTreeGrowthModule::doTimingStep() {
				if (!_shouldRun || (_spinupMossOnly->value() == true)) {
					return;
				}

				int regenDelay = _regenDelay->value();
				if (regenDelay > 0) {
					return;
				}

				// Get current biomass pool values.
				updateBiomassPools();
				standSoftwoodStemSnag = _softwoodStemSnag->value();
				standSoftwoodBranchSnag = _softwoodBranchSnag->value();
				if (_smallTreeGrowthHW != nullptr) {
					standHardwoodStemSnag = _hardwoodStemSnag->value();
					standHardwoodBranchSnag = _hardwoodBranchSnag->value();
				}

				getIncrements();	  // 1) get and store the biomass carbon growth increments
				doHalfGrowth();		  // 2) transfer half of the biomass growth increment to the biomass pool
				updateBiomassPools(); // 3) update to record the current biomass pool value plus the half increment of biomass
				doMidSeasonGrowth();  // 4) the foliage and snags that grow and are turned over

				int standSmallTreeAge = _smalltreeAge->value();

				if (_outputRemoval != nullptr && _outputRemoval->value()) {
					//debug to print out the removal from live biomass components
					double smallTreeFoliageRemoval = _currentTurnoverRates->swFoliageTurnover() * _softwoodFoliage->value();
					double smallTreeStemSnagRemoval = _softwoodStemSnag->value() * _currentTurnoverRates->swStemSnagTurnover();
					double smallTreeBranchSnagRemoval = _currentTurnoverRates->swBranchSnagTurnover() * _softwoodBranchSnag->value();
					double smallTreeOtherRemovalToWFD = (1 - _currentTurnoverRates->swBranchSnagSplit()) * _softwoodOther->value() * _currentTurnoverRates->swBranchSnagTurnover();
					double smallTreeCoarseRootRemoval = _currentTurnoverRates->swCoarseRootTurnover() * _softwoodCoarseRoots->value();
					double smallTreeFineRootRemoval = _currentTurnoverRates->swFineRootTurnover() * _softwoodFineRoots->value();
					double smallTreeOtherToBranchSnag = standSoftwoodOther * _currentTurnoverRates->swBranchSnagSplit() * _currentTurnoverRates->swBranchSnagTurnover();
					double smallTreeStemRemoval = standSoftwoodStem * _currentTurnoverRates->swStemTurnover();

					printRemovals(standSmallTreeAge,
						smallTreeFoliageRemoval,
						smallTreeStemSnagRemoval,
						smallTreeBranchSnagRemoval,
						smallTreeOtherRemovalToWFD,
						smallTreeCoarseRootRemoval,
						smallTreeFineRootRemoval,
						smallTreeOtherToBranchSnag,
						smallTreeStemRemoval);
				}

				doPeatlandTurnover(); // 5) do biomass and snag turnover, small tree is in treed peatland only     
				doHalfGrowth();		  // 6) transfer the remaining half increment to the biomass pool		

				_smalltreeAge->set_value(standSmallTreeAge + 1);
			}

			void SmallTreeGrowthModule::getIncrements() {
				//get current small tree age
				int smallTreeAge = _smalltreeAge->value();

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
            growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * _currentTurnoverRates->swBranchSnagSplit() / 2);
            growth->addTransfer(_softwoodOther, _woodyFineDead, -swo * (1 - _currentTurnoverRates->swBranchSnagSplit()) / 2);
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
				growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * _currentTurnoverRates->hwBranchSnagSplit() / 2);
				growth->addTransfer(_hardwoodOther, _woodyFineDead, -hwo * (1 - _currentTurnoverRates->hwBranchSnagSplit()) / 2);
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
					->addTransfer(_softwoodStemSnag, _woodyFineDead, standSoftwoodStemSnag * _currentTurnoverRates->swStemSnagTurnover())
					->addTransfer(_softwoodBranchSnag, _woodyFineDead, standSoftwoodBranchSnag * _currentTurnoverRates->swBranchSnagTurnover());

				if (_smallTreeGrowthHW != nullptr) {
					domTurnover
						->addTransfer(_hardwoodStemSnag, _woodyFineDead, standHardwoodStemSnag * _currentTurnoverRates->hwStemSnagTurnover())
						->addTransfer(_hardwoodBranchSnag, _woodyFineDead, standHardwoodBranchSnag * _currentTurnoverRates->hwBranchSnagTurnover());
				}
				_landUnitData->submitOperation(domTurnover);

				// Biomass turnover as stock operation.
				auto bioTurnover = _landUnitData->createStockOperation();
				bioTurnover
					->addTransfer(_softwoodStem, _softwoodBranchSnag, standSoftwoodStem * _currentTurnoverRates->swStemTurnover())
					->addTransfer(_softwoodFoliage, _woodyFoliageDead, standSoftwoodFoliage * _currentTurnoverRates->swFoliageTurnover())
					->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _currentTurnoverRates->swBranchSnagSplit() * _currentTurnoverRates->swBranchTurnover())
					->addTransfer(_softwoodOther, _woodyFineDead, standSoftwoodOther * (1 - _currentTurnoverRates->swBranchSnagSplit()) * _currentTurnoverRates->swBranchTurnover())
					->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, standSWCoarseRootsCarbon * _currentTurnoverRates->swCoarseRootTurnover())
					->addTransfer(_softwoodFineRoots, _woodyRootsDead, standSWFineRootsCarbon * _currentTurnoverRates->swFineRootTurnover());

				if (_smallTreeGrowthHW != nullptr) {
					bioTurnover
						->addTransfer(_hardwoodStem, _hardwoodBranchSnag, standHardwoodStem * _currentTurnoverRates->hwStemTurnover())
						->addTransfer(_hardwoodFoliage, _woodyFoliageDead, standHardwoodFoliage * _currentTurnoverRates->hwFoliageTurnover())
						->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _currentTurnoverRates->hwBranchSnagSplit() * _currentTurnoverRates->hwBranchTurnover())
						->addTransfer(_hardwoodOther, _woodyFineDead, standHardwoodOther * (1 - _currentTurnoverRates->hwBranchSnagSplit()) * _currentTurnoverRates->hwBranchTurnover())
						->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, standHWCoarseRootsCarbon * _currentTurnoverRates->hwCoarseRootTurnover())
						->addTransfer(_hardwoodFineRoots, _woodyRootsDead, standHWFineRootsCarbon * _currentTurnoverRates->hwFineRootTurnover());
				}
				_landUnitData->submitOperation(bioTurnover);
				_landUnitData->applyOperations();
			}

			void SmallTreeGrowthModule::doMidSeasonGrowth() const {
				auto seasonalGrowth = _landUnitData->createStockOperation();
				seasonalGrowth
					->addTransfer(_atmosphere, _softwoodStem, standSoftwoodStem * _currentTurnoverRates->swStemTurnover())
					->addTransfer(_atmosphere, _softwoodOther, standSoftwoodOther * _currentTurnoverRates->swBranchTurnover())
					->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _currentTurnoverRates->swFoliageTurnover())
					->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _currentTurnoverRates->swCoarseRootTurnover())
					->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _currentTurnoverRates->swFineRootTurnover());

				if (_smallTreeGrowthHW != nullptr) {
					seasonalGrowth
						->addTransfer(_atmosphere, _hardwoodStem, standHardwoodStem * _currentTurnoverRates->hwStemTurnover())
						->addTransfer(_atmosphere, _hardwoodOther, standHardwoodOther * _currentTurnoverRates->hwBranchTurnover())
						->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _currentTurnoverRates->hwFoliageTurnover())
						->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _currentTurnoverRates->hwCoarseRootTurnover())
						->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _currentTurnoverRates->hwFineRootTurnover());
				}
				_landUnitData->submitOperation(seasonalGrowth);
			}

			void SmallTreeGrowthModule::getTurnoverRates(int smalltreeGCID, int spuID) {
				auto key = std::make_tuple(smalltreeGCID, spuID);
				auto turnoverRates = _cachedTurnoverRates.find(key);
				if (turnoverRates != _cachedTurnoverRates.end()) {
					_currentTurnoverRates = turnoverRates->second;
				}
				else {
					_turnoverRates = _landUnitData->getVariable("turnover_rates");
					_currentTurnoverRates = std::make_shared<TurnoverRates>(_turnoverRates->value().extract<DynamicObject>());
					_cachedTurnoverRates[key] = _currentTurnoverRates;
				}
			}

			void SmallTreeGrowthModule::printRemovals(int standSmallTreeAge,
				double smallTreeFoliageRemoval,
				double smallTreeStemSnagRemoval,
				double smallTreeBranchSnagRemoval,
				double smallTreeOtherRemovalToWFD,
				double smallTreeCoarseRootRemoval,
				double smallTreeFineRootRemoval,
				double smallTreeOtherToBranchSnag,
				double smallTreeStemRemoval) {
				MOJA_LOG_INFO << standSmallTreeAge << ", " << smallTreeFoliageRemoval << ", " << smallTreeStemSnagRemoval << ", " << smallTreeBranchSnagRemoval << ", " << smallTreeOtherRemovalToWFD << ", "
			<< smallTreeCoarseRootRemoval << ", " << smallTreeFineRootRemoval << ", " << smallTreeOtherToBranchSnag << ", " << smallTreeStemRemoval;
	}
}}}
