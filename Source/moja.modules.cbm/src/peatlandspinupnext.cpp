#include "moja/modules/cbm/peatlandspinupnext.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandSpinupNext::configure(const DynamicObject& config) { }

	void PeatlandSpinupNext::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandSpinupNext::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandSpinupNext::onTimingInit, *this);		
	}   

	void PeatlandSpinupNext::doLocalDomainInit() {
		_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");	
		_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
		_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");
		_softwoodOther = _landUnitData->getPool("SoftwoodOther");
		_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
		_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
		_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");	
		_hardwoodOther = _landUnitData->getPool("HardwoodOther");

		_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
		_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
		_softwoodStem = _landUnitData->getPool("SoftwoodStem");
		_hardwoodStem = _landUnitData->getPool("HardwoodStem");

		_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
		_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
		_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");
		_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
		_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");
		_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
		_featherMossLive = _landUnitData->getPool("FeatherMossLive");

		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");
		_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
		_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");
		_feathermossDead = _landUnitData->getPool("FeathermossDead");

		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");
		_acrotelm_a = _landUnitData->getPool("Acrotelm_A");		
		_catotelm_o = _landUnitData->getPool("Catotelm_O");		

		_atmosphere = _landUnitData->getPool("Atmosphere");
    }

	void PeatlandSpinupNext::doTimingInit() {
		auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();
		if (loadInitialFlag) {
			//PrintPools::printPeatlandPools("Year ", *_landUnitData);

			//in case of loading initial peat pool value, this step is skipped. 
			//initial acrotelm and catotelm pool values will be loadded in peatland prepare module

			//now, load initial value is not preferred
			return;
		}

		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }

		//get the current peatland ID
		int peatlandId = _landUnitData->getVariable("peatlandId")->value();

		//get the mean anual temperture variable
		meanAnnualTemperature = _landUnitData->getVariable("mean_annual_temperature")->value();	

		//get fire return interval
		auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
		int fireReturnIntervalValue = fireReturnInterval.isEmpty() ? -1 : fireReturnInterval.convert<int>();
		fireReturnReciprocal = 1.0 / fireReturnIntervalValue;

		// get turnover rate
		getTreeTurnoverRate();

		// get related parameters
		getAndUpdateParameter();

		// prepare for speeding peatland spinup
		preparePeatlandSpinupSpeedup(peatlandId);

		//check values in current peat pools
		getCurrentDeadPoolValues();

		//reset some of the dead pools
		resetSlowPools();

		// transfer carbon between pools
		populatePeatlandDeadPools();

		//PrintPools::printPeatlandPools("Year ", *_landUnitData);
    }

	void PeatlandSpinupNext::getAndUpdateParameter() {
		// get the data by variable "peatland_decay_parameters"
		const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();

		//create the PeaglandDecayParameters, set the value from the variable
		decayParas = std::make_shared<PeatlandDecayParameters>();
		decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

		//compute the applied parameters
		decayParas->updateAppliedDecayParameters(meanAnnualTemperature);

		// get the data by variable "peatland_turnover_parameters"
		const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

		//create the PeaglandGrowthParameters, set the value from the variable
		turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
		turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());

		// get the data by variable "peatland_growth_parameters"
		const auto& peatlandGrowthParams = _landUnitData->getVariable("peatland_growth_parameters")->value();

		//create the PeatlandGrowthParameters, set the value from the variable
		growthParas = std::make_shared<PeatlandGrowthParameters>();
		growthParas->setValue(peatlandGrowthParams.extract<DynamicObject>());

		// get the data by variable "peatland_fire_parameters"
		const auto& peatlandFireParams = _landUnitData->getVariable("peatland_fire_parameters")->value();

		//create the PeatlandFireParameters, set the value from the variable
		fireParas = std::make_shared<PeatlandFireParameters>();
		if (!peatlandFireParams.isEmpty()) {
			fireParas->setValue(peatlandFireParams.extract<DynamicObject>());
		}
	}
	
	void PeatlandSpinupNext::preparePeatlandSpinupSpeedup(int peatlandId)
	{		
		switch (peatlandId) {
			case 2:
			case 5:
			case 8:
			case 10:
				smallTreeOn = 1;			
				smallTreeFoliage = _softwoodFoliageFallRate * _softwoodFoliage->value() + _hardwoodFoliageFallRate * _hardwoodFoliage->value();
				smallTreeStem = _stemAnnualTurnOverRate * (_softwoodStem->value() + _hardwoodStem->value());
				smallTreeOther = _softwoodBranchTurnOverRate * _softwoodOther->value() + _hardwoodFoliageFallRate * _hardwoodOther->value();
				smallTreeCoarseRoot = _coarseRootTurnProp* ( _softwoodCoarseRoots->value() + _hardwoodCoarseRoots->value());
				smallTreeFineRoot = _fineRootTurnProp * (_softwoodFineRoots->value() + _hardwoodFineRoots->value());
				break;
			case 3:
			case 6:
			case 9:
			case 11:
				largeTreeOn = 1;
				largeTreeFoliage = _softwoodFoliageFallRate * _softwoodFoliage->value() + _hardwoodFoliageFallRate * _hardwoodFoliage->value();
				largeTreeMerchant = _stemAnnualTurnOverRate * (_softwoodMerch->value() + _hardwoodMerch->value());
				largeTreeOther = _softwoodBranchTurnOverRate * _softwoodOther->value() + _hardwoodFoliageFallRate * _hardwoodOther->value();
				largeTreeCoarseRoot = _coarseRootTurnProp * (_softwoodCoarseRoots->value() + _hardwoodCoarseRoots->value());
				largeTreeFineRoot = _fineRootTurnProp * (_softwoodFineRoots->value() + _hardwoodFineRoots->value());
				break;
		}
	}

	void PeatlandSpinupNext::getCurrentDeadPoolValues() {
		auto wdyFoliageDead = _woodyFoliageDead->value();
		auto wdyStemBranchDead = _woodyFineDead->value();
		auto wdyRootsDead = _woodyRootsDead->value();
		auto sedgeFoliageDead = _sedgeFoliageDead->value();
		auto sedgeRootsDead = _sedgeRootsDead->value();
		auto featherMossDead = _feathermossDead->value();
		auto actotelm = _acrotelm_o->value();
		auto catotelm = _catotelm_a->value();
	}

	void PeatlandSpinupNext::populatePeatlandDeadPools() {
		auto wdyFoliageLive = _woodyFoliageLive->value();
		auto denominator1 = (turnoverParas->Pfe() * decayParas->kwfe() * (modifyQ10(decayParas->Q10wf())) +
			turnoverParas->Pfn() * decayParas->kwfne() * (modifyQ10(decayParas->Q10wf())) + 
			fireReturnReciprocal * fireParas->CCdwf());

		auto wdyFoliageDead = (
			(wdyFoliageLive * (turnoverParas->Pfe()  * turnoverParas->Pel() + turnoverParas->Pnl() * turnoverParas->Pfn())) +
			(smallTreeOn * turnoverParas->Mstf() * smallTreeFoliage * (1 - fireReturnReciprocal)) +
			(largeTreeOn * largeTreeFoliage * (1 - fireReturnReciprocal))) / denominator1;			

		auto wdyStemBranchLive = _woodyStemsBranchesLive->value();
		auto denominator2 = (decayParas->kwsb() * (modifyQ10(decayParas->Q10wsb())) + 
			fireReturnReciprocal * fireParas->CCdwsb());
		auto wdyStemBranchDead = (
			(wdyStemBranchLive * growthParas->NPPagls() / growthParas->Bagls()) +
			(smallTreeOn * (turnoverParas->Msto() * smallTreeOther + turnoverParas->Msts() * smallTreeStem) * (1 - fireReturnReciprocal)) +
			(largeTreeOn * (largeTreeMerchant + largeTreeOther) * (1 - fireReturnReciprocal))) / denominator2;

		auto wdyRootsLive = _woodyRootsLive->value();
		auto denominator3 = (decayParas->kwr() * (modifyQ10(decayParas->Q10wr())) + 
			fireReturnReciprocal * fireParas->CCdwr());
		auto wdyRootsDead = (
			(wdyRootsLive * turnoverParas->Mbgls())+
			(smallTreeOn * (turnoverParas->Mstfr() * smallTreeFineRoot + turnoverParas->Mstcr() * smallTreeCoarseRoot) * (1 - fireReturnReciprocal)) +
			(largeTreeOn * (largeTreeFineRoot + largeTreeCoarseRoot) * (1 - fireReturnReciprocal))) /denominator3;

		auto sedgeFoliageLive = _sedgeFoliageLive->value();
		auto denominator4 = (decayParas->ksf() * (modifyQ10(decayParas->Q10sf())) + 
			fireReturnReciprocal * fireParas->CCdsf());
		auto sedgeFoliageDead = sedgeFoliageLive * turnoverParas->Mags() / denominator4;			

		auto sedgeRootsLive = _sedgeRootsLive->value();
		auto denominator5 = (decayParas->ksr() *  (modifyQ10(decayParas->Q10sr())) + 
			fireReturnReciprocal * fireParas->CCdsr());
		auto sedgeRootsDead = sedgeRootsLive * turnoverParas->Mbgs() / denominator5;			

		auto featherMossLive = _featherMossLive->value();
		auto denominator6 = (decayParas->kfm() * (modifyQ10(decayParas->Q10fm())) + 
			fireReturnReciprocal * fireParas->CCdfm());
		auto featherMossDead = featherMossLive / denominator6;
	
		auto wdyFoliageDeadToAcrotelm = decayParas->Pt() * wdyFoliageDead * 
			(turnoverParas->Pfe() * decayParas->kwfe() * (modifyQ10(decayParas->Q10wf())) + 
			turnoverParas->Pfn() * decayParas->kwfne() * (modifyQ10(decayParas->Q10wf())));

		auto wdyStemBranchDeadToAcrotelm = decayParas->Pt() * wdyStemBranchDead * 
			decayParas->kwsb() * (modifyQ10(decayParas->Q10wsb()));

		auto wdyRootsDeadToAcrotelm = decayParas->Pt() * wdyRootsDead * 
			decayParas->kwr() * (modifyQ10(decayParas->Q10wr()));

		auto sedgeFoliageDeadToAcrotelm = decayParas->Pt() * sedgeFoliageDead * 
			decayParas->ksf() * (modifyQ10(decayParas->Q10sf()));

		auto sedgeRootsDeadToAcrotelm = decayParas->Pt() * sedgeRootsDead * 
			decayParas->ksr() * (modifyQ10(decayParas->Q10sr()));

		auto featherMossLiveToAcrotelm = featherMossLive;

		auto featherMossDeadToAcrotelm = featherMossDead * decayParas->kfm() * (modifyQ10(decayParas->Q10fm()));

		auto wdyRootsLiveToAcrotelm = wdyRootsLive * fireReturnReciprocal * fireParas->CTwr();

		auto sedgeRootsLiveToAcrotelm = sedgeRootsLive * fireReturnReciprocal * fireParas->CTsr();

		auto denominator7 = (decayParas->ka() * (modifyQ10(decayParas->Q10a())) + (fireReturnReciprocal) * fireParas->Cca());
		auto toAcrotelm = (wdyFoliageDeadToAcrotelm +
							wdyStemBranchDeadToAcrotelm +
							wdyRootsDeadToAcrotelm +
							sedgeFoliageDeadToAcrotelm +
							sedgeRootsDeadToAcrotelm +
							featherMossLiveToAcrotelm +
							featherMossDeadToAcrotelm +
							wdyRootsLiveToAcrotelm +
							sedgeRootsLiveToAcrotelm) / denominator7;		

		// transfer carbon from acrotelm to catotelm 	
		auto ac2caAmount = (decayParas->Pt() * toAcrotelm * decayParas->ka()  * (modifyQ10(decayParas->Q10a())) - 0.3) /
			(decayParas->kc() * (modifyQ10(decayParas->Q10c())));
		
		//make sure ac2caAmount >=0
		ac2caAmount = ac2caAmount > 0 ? ac2caAmount : 0;

		// transfer carbons to peatland dead pool by stock amount
		auto peatlandSpinnupOne = _landUnitData->createStockOperation();
		peatlandSpinnupOne->addTransfer(_atmosphere, _woodyFoliageDead, wdyFoliageDead)
			->addTransfer(_atmosphere, _woodyFineDead, wdyStemBranchDead)
			->addTransfer(_atmosphere, _woodyRootsDead, wdyRootsDead)
			->addTransfer(_atmosphere, _sedgeFoliageDead, sedgeFoliageDead)
			->addTransfer(_atmosphere, _sedgeRootsDead, sedgeRootsDead)
			->addTransfer(_atmosphere, _feathermossDead, featherMossDead)
			->addTransfer(_atmosphere, _acrotelm_o, toAcrotelm)
			->addTransfer(_atmosphere, _catotelm_a, ac2caAmount);

		_landUnitData->submitOperation(peatlandSpinnupOne);
		_landUnitData->applyOperations();
	}		

	void PeatlandSpinupNext::getTreeTurnoverRate() {
		_turnoverRates = _landUnitData->getVariable("turnover_rates");
		const auto& turnoverRates = _turnoverRates->value().extract<DynamicObject>();	

		_stemAnnualTurnOverRate = turnoverRates["stem_annual_turnover_rate"];	
		_softwoodFoliageFallRate = turnoverRates["softwood_foliage_fall_rate"];
		_hardwoodFoliageFallRate = turnoverRates["hardwood_foliage_fall_rate"];
		_softwoodBranchTurnOverRate = turnoverRates["softwood_branch_turnover_rate"];		
		_hardwoodBranchTurnOverRate = turnoverRates["hardwood_branch_turnover_rate"];		
		_coarseRootTurnProp = turnoverRates["coarse_root_turn_prop"];
		_fineRootTurnProp = turnoverRates["fine_root_turn_prop"];
	}

	//all of the slow dead pools are directly assigned by above computing based on live pools
	//reset current slow pool value to receive the new computed value
	void PeatlandSpinupNext::resetSlowPools() {
		auto peatlandDeadPoolReset = _landUnitData->createProportionalOperation();
		peatlandDeadPoolReset
			->addTransfer(_woodyFoliageDead, _atmosphere, 1.0)
			->addTransfer(_woodyFineDead, _atmosphere, 1.0)
			->addTransfer(_woodyRootsDead, _atmosphere, 1.0)
			->addTransfer(_sedgeFoliageDead, _atmosphere, 1.0)
			->addTransfer(_sedgeRootsDead, _atmosphere, 1.0)
			->addTransfer(_feathermossDead, _atmosphere, 1.0)
			->addTransfer(_acrotelm_o, _atmosphere, 1.0)
			->addTransfer(_catotelm_a, _atmosphere, 1.0);
		_landUnitData->submitOperation(peatlandDeadPoolReset);
		_landUnitData->applyOperations();
	}
}}} // namespace moja::modules::cbm
