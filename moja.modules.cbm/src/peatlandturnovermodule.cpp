#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/logging.h"

#include "moja/modules/cbm/peatlandturnovermodule.h"
#include "moja/modules/cbm/printpools.h"

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandTurnoverModule::configure(const DynamicObject& config) { }

	void PeatlandTurnoverModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandTurnoverModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandTurnoverModule::onTimingInit, *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandTurnoverModule::onTimingStep, *this);
	}   

	void PeatlandTurnoverModule::onLocalDomainInit() {
		_atmosphere = _landUnitData->getPool("Atmosphere");

		_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
		_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
		_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");

		_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
		_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");

		_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
		_featherMossLive = _landUnitData->getPool("FeatherMossLive");

		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyStemsBranchesDead = _landUnitData->getPool("WoodyStemsBranchesDead");		
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");

		_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
		_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");

		_feathermossDead = _landUnitData->getPool("FeathermossDead");

		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");
		_acrotelm_a = _landUnitData->getPool("Acrotelm_A");
		_catotelm_o = _landUnitData->getPool("Catotelm_O");

		_peatlandAge = _landUnitData->getVariable("age");		
    }

	void PeatlandTurnoverModule::onTimingInit() {
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }

		// get the data by variable "peatland_turnover_parameters"
		const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

		//create the PeaglandGrowthParameters, set the value from the variable
		turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
		turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());

		// get the data by variable "peatland_growth_parameters"
		const auto& peatlandGrowthParams = _landUnitData->getVariable("peatland_growth_parameters")->value();

		//create the PeatlandGrowthParameters, set the value from the variable
		growthParas = std::make_shared<PeatlandGrowthParameters>();
		if (!peatlandGrowthParams.isEmpty()) {
			growthParas->setValue(peatlandGrowthParams.extract<DynamicObject>());
		}
    }

	void PeatlandTurnoverModule::onTimingStep() {
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }

		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
		if (spinupMossOnly) { return; }

		//update the current pool value
		updatePeatlandLivePoolValue();				

		//turnover on live pools
		doLivePoolTurnover();

		//flux between catotelm and acrotelm due to water table changes
		doWaterTableFlux();
    }

	void PeatlandTurnoverModule::updatePeatlandLivePoolValue(){
		woodyFoliageLive = _woodyFoliageLive->value();
		woodyStemsBranchesLive = _woodyStemsBranchesLive->value();
		woodyRootsLive = _woodyRootsLive->value();
		sedgeFoliageLive = _sedgeFoliageLive->value();
		sedgeRootsLive = _sedgeRootsLive->value();
		featherMossLive = _featherMossLive->value();
		sphagnumMossLive = _sphagnumMossLive->value();			
	}

	void PeatlandTurnoverModule::doLivePoolTurnover(){
		//live to dead pool turnover transfers
		//for live woody layer, woodyRootsLive does transfer and can be deducted from source.
		auto peatlandTurnover = _landUnitData->createStockOperation();

		peatlandTurnover
			->addTransfer(_atmosphere, _woodyFoliageDead, woodyFoliageLive* (
			turnoverParas->Pfe() * turnoverParas->Pel() +
			turnoverParas->Pfn() * turnoverParas->Pnl()))
			->addTransfer(_atmosphere, _woodyStemsBranchesDead, woodyStemsBranchesLive * growthParas->Magls())
			->addTransfer(_woodyRootsLive, _woodyRootsDead, woodyRootsLive * turnoverParas->Mbgls())
			->addTransfer(_sedgeFoliageLive, _sedgeFoliageDead, sedgeFoliageLive * turnoverParas->Mags())
			->addTransfer(_sedgeRootsLive, _sedgeRootsDead, sedgeRootsLive * turnoverParas->Mbgs())
			->addTransfer(_featherMossLive, _feathermossDead, featherMossLive * 1.0)
			->addTransfer(_sphagnumMossLive, _acrotelm_o, sphagnumMossLive * 1.0);

		_landUnitData->submitOperation(peatlandTurnover);
	}

	void PeatlandTurnoverModule::doWaterTableFlux(){
		//get current annual water table depth
		double currentAwtd = _landUnitData->getVariable("peatland_current_annual_wtd")->value();

		//get previous annual water table depth
		double previousAwtd = _landUnitData->getVariable("peatland_previous_annual_wtd")->value();

		//get long term annual water table depth
		double longtermWtd = _landUnitData->getVariable("peatland_longterm_wtd")->value();

		auto peatlandWaterTableFlux = _landUnitData->createStockOperation();
		double fluxAmount = 0;

		if (currentAwtd <= longtermWtd  &&  previousAwtd <= longtermWtd) {
			if (previousAwtd >= currentAwtd) {
				//Catotelm_O -> Catotelm_A : (Awtd(t-1) - Awtd(t))*BDc
				fluxAmount = (previousAwtd - currentAwtd) * turnoverParas->BDc();
				peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, fluxAmount);
			}
			else if (currentAwtd >= previousAwtd) {
				//Catotelm_O -> Catotelm_A : (Awtd(t-1) - lwtd)*BDc
				fluxAmount = (currentAwtd - previousAwtd) * turnoverParas->BDc();
				peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, fluxAmount);
			}			
		}
		else if (currentAwtd >= longtermWtd  &&  previousAwtd >= longtermWtd) {
			if (previousAwtd >= currentAwtd) {
				//Acrotelm_O -> Acrotelm_A : (Awtd(t-1) - Awtd(t))*BDa
				fluxAmount = (previousAwtd - currentAwtd) * turnoverParas->BDa();
				peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, fluxAmount);
			}
			else if (currentAwtd >= previousAwtd) {
				//Acrotelm_O -> Acrotelm_A : (Lwtd - Awtd(t-1))*BDa
				fluxAmount = (currentAwtd - currentAwtd) * turnoverParas->BDa();
				peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, fluxAmount);
			}			
		}

		_landUnitData->submitOperation(peatlandWaterTableFlux);
	}

	/*
	void PeatlandTurnoverModule::doWaterTableFlux(){
		//get current annual water table depth
		double currentAwtd = _landUnitData->getVariable("peatland_current_annual_wtd")->value();

		//get previous annual water table depth
		double previousAwtd = _landUnitData->getVariable("peatland_previous_annual_wtd")->value();

		//get long term annual water table depth
		double longtermWtd = _landUnitData->getVariable("peatland_longterm_wtd")->value();

		auto peatlandWaterTableFlux = _landUnitData->createStockOperation();
		double fluxAmount = 0;

		if (currentAwtd < longtermWtd  &&  previousAwtd < longtermWtd) {
			if (currentAwtd < previousAwtd) {
				//Catotelm_O -> Catotelm_A : (Awtd(t-1) - Awtd(t))*BDc
				fluxAmount = (previousAwtd - currentAwtd) * turnoverParas->BDc();
				peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, fluxAmount);
			}
			else if (currentAwtd <= longtermWtd && longtermWtd <= previousAwtd) {
				//Catotelm_O -> Catotelm_A : (Awtd(t-1) - lwtd)*BDc
				fluxAmount = (previousAwtd - longtermWtd) * turnoverParas->BDc();
				peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, fluxAmount);
			}

			if (currentAwtd > previousAwtd) {
				//catotelm_A -> catotelm_O : (Awtd(t) - Awtd(t-1))*BDc
				fluxAmount = (currentAwtd - previousAwtd) * turnoverParas->BDc();
				peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, fluxAmount);
			}
			else if (currentAwtd >= longtermWtd && longtermWtd >= previousAwtd) {
				//catotelm_A -> catotelm_O : (Awtd(t-1) - Lwtd)*BDc
				fluxAmount = (previousAwtd - longtermWtd) * turnoverParas->BDc();
				peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, fluxAmount);
			}			
		}
		else if (currentAwtd > longtermWtd  &&  previousAwtd > longtermWtd) {
			if (currentAwtd < previousAwtd) {
				//Acrotelm_O -> Acrotelm_A : (Awtd(t-1) - Awtd(t))*BDa
				fluxAmount = (previousAwtd - currentAwtd) * turnoverParas->BDa();
				peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, fluxAmount);
			}
			else if (currentAwtd <= longtermWtd && longtermWtd <= previousAwtd) {
				//Acrotelm_O -> Acrotelm_A : (Lwtd - Awtd(t-1))*BDa
				fluxAmount = (longtermWtd - currentAwtd) * turnoverParas->BDa();
				peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, fluxAmount);
			}
			if (currentAwtd > previousAwtd) {
				//Acrotelm_A -> Acrotelm_O : (Awtd(t) - Awtd(t-1))*BDa
				fluxAmount = (longtermWtd - currentAwtd) * turnoverParas->BDa();
				peatlandWaterTableFlux->addTransfer(_acrotelm_a, _acrotelm_o, fluxAmount);
			}
			else if (currentAwtd >= longtermWtd && longtermWtd >= previousAwtd) {
				//Acrotelm_A -> Acrotelm_O : (Lwtd - Awtd(t-1))*BDa
				fluxAmount = (longtermWtd - previousAwtd) * turnoverParas->BDa();
				peatlandWaterTableFlux->addTransfer(_acrotelm_a, _acrotelm_o, fluxAmount);
			}
		}

		_landUnitData->submitOperation(peatlandWaterTableFlux);
	}
	*/

}}} // namespace moja::modules::cbm
