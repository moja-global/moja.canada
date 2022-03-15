#include "moja/modules/cbm/peatlandturnovermodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandTurnoverModule::configure(const DynamicObject& config) { }

	void PeatlandTurnoverModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandTurnoverModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandTurnoverModule::onTimingInit, *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandTurnoverModule::onTimingStep, *this);
	}   

	void PeatlandTurnoverModule::doLocalDomainInit() {
		_atmosphere = _landUnitData->getPool("Atmosphere");

		_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
		_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
		_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");

		_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
		_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");

		_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
		_featherMossLive = _landUnitData->getPool("FeatherMossLive");

		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
		_woodyCoarseDead = _landUnitData->getPool("WoodyCoarseDead");
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");

		_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
		_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");

		_feathermossDead = _landUnitData->getPool("FeathermossDead");

		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");
		_acrotelm_a = _landUnitData->getPool("Acrotelm_A");
		_catotelm_o = _landUnitData->getPool("Catotelm_O");				
		_shrubAge = _landUnitData->getVariable("peatland_shrub_age");

		_regenDelay = _landUnitData->getVariable("regen_delay");
    }

	void PeatlandTurnoverModule::doTimingInit() {
		_runPeatland = _landUnitData->getVariable("run_peatland")->value().convert<bool>();
		if (!_runPeatland){ return; }

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

	void PeatlandTurnoverModule::doTimingStep() {
		if (!_runPeatland){ return; }

		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value().convert<bool>();
		if (spinupMossOnly) { return; }		

		int regenDelay = _regenDelay->value();
		if (regenDelay > 0) {
			//in delay period, no any growth
			//do flux between catotelm and acrotelm due to water table changes
			doWaterTableFlux();

			return;
		}

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

		//Special implementation - no moss turnover in the first few years (by Rsp and Rfm, current 5).
		int shrubAge = _shrubAge->value();
		double sphagnumMossLiveTurnover = (shrubAge - 1) <= growthParas->Rsp() ? 0 : growthParas->GCsp() * growthParas->NPPsp();
		double featherMossLiveTurnover = (shrubAge - 1) <= growthParas->Rfm() ? 0 : growthParas->GCfm() * growthParas->NPPfm();
		
		//the first two, source is atmospher, it is particularly modeled, no problem.
		peatlandTurnover
			->addTransfer(_atmosphere, _woodyFoliageDead, woodyFoliageLive* (turnoverParas->Pfe() * turnoverParas->Pel() + 	turnoverParas->Pfn() * turnoverParas->Pnl()))
			->addTransfer(_atmosphere, _woodyFineDead, woodyStemsBranchesLive * growthParas->Magls())
			->addTransfer(_woodyRootsLive, _woodyRootsDead, woodyRootsLive * turnoverParas->Mbgls())
			->addTransfer(_sedgeFoliageLive, _sedgeFoliageDead, sedgeFoliageLive * turnoverParas->Mags())
			->addTransfer(_sedgeRootsLive, _sedgeRootsDead, sedgeRootsLive * turnoverParas->Mbgs())
			->addTransfer(_featherMossLive, _feathermossDead, featherMossLiveTurnover)
			->addTransfer(_sphagnumMossLive, _acrotelm_o, sphagnumMossLiveTurnover);

		_landUnitData->submitOperation(peatlandTurnover);
	}

	void PeatlandTurnoverModule::doWaterTableFlux(){
		//get current annual water table depth
 		double currentAwtd = _landUnitData->getVariable("peatland_current_annual_wtd")->value().convert<double>();

		//get previous annual water table depth
		double previousAwtd = _landUnitData->getVariable("peatland_previous_annual_wtd")->value().convert<double>();		

		//get long term annual water table depth
		double longtermWtd = _landUnitData->getVariable("peatland_longterm_wtd")->value().convert<double>();
		currentAwtd = currentAwtd > 0 ? 0 : currentAwtd;
		previousAwtd = previousAwtd > 0 ? 0 : previousAwtd;
		longtermWtd = longtermWtd > 0 ? 0 : longtermWtd;

		double a = turnoverParas->a();
		double b = turnoverParas->b();

		auto peatlandWaterTableFlux = _landUnitData->createStockOperation();

		double coPoolValue = _catotelm_o->value();
		double caPoolValue = _catotelm_a->value();
		double aoPoolValue = _acrotelm_o->value();
		double aaPoolValue = _acrotelm_a->value();

		double fluxAmount = computeCarbonTransfers(previousAwtd, currentAwtd, a, b);

		if (currentAwtd < longtermWtd  &&  previousAwtd < longtermWtd) {
			if (currentAwtd >= previousAwtd) {
				//Catotelm_O -> Catotelm_A 		
				if (fluxAmount > coPoolValue) fluxAmount = coPoolValue;
				peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, fluxAmount);
			}
			else if (currentAwtd <= previousAwtd) {
				//Catotelm_A -> Catotelm_O
				if (fluxAmount > caPoolValue) fluxAmount = caPoolValue;
				peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, fluxAmount);
			}
		}
		else if (currentAwtd > longtermWtd  &&  previousAwtd > longtermWtd) {
			if (currentAwtd >= previousAwtd) {
				//Acrotelm_O -> Acrotelm_A 				
				if (fluxAmount > aoPoolValue) fluxAmount = aoPoolValue;
				peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, fluxAmount);
			}
			else if (currentAwtd <= previousAwtd) {
				//Acrotelm_A -> Acrotelm_O 
				if (fluxAmount > aaPoolValue) fluxAmount = aaPoolValue;
				peatlandWaterTableFlux->addTransfer(_acrotelm_a, _acrotelm_o, fluxAmount);
			}
		}				
		else if (currentAwtd >= longtermWtd &&  previousAwtd <= longtermWtd) {
			if (currentAwtd >= previousAwtd){
				double ao2aa = computeCarbonTransfers(longtermWtd, currentAwtd, a, b);
				if (ao2aa > aoPoolValue) ao2aa = aoPoolValue;
				peatlandWaterTableFlux->addTransfer(_acrotelm_o, _acrotelm_a, ao2aa);

				double co2ca = computeCarbonTransfers(longtermWtd, previousAwtd, a, b);
				if (co2ca > coPoolValue) co2ca = coPoolValue;
				peatlandWaterTableFlux->addTransfer(_catotelm_o, _catotelm_a, co2ca);
			}
		}
		else if (currentAwtd <= longtermWtd &&  previousAwtd >= longtermWtd) {
			if (currentAwtd <= previousAwtd) {
				double aa2ao = computeCarbonTransfers(longtermWtd, previousAwtd, a, b);
				if (aa2ao > aaPoolValue) aa2ao = aaPoolValue;
				peatlandWaterTableFlux->addTransfer(_acrotelm_a, _acrotelm_o, aa2ao);

				double ca2co = computeCarbonTransfers(longtermWtd, currentAwtd, a, b);
				if (ca2co > caPoolValue) ca2co = caPoolValue;
				peatlandWaterTableFlux->addTransfer(_catotelm_a, _catotelm_o, ca2co);
			}
		}

		_landUnitData->submitOperation(peatlandWaterTableFlux);
	}

	double PeatlandTurnoverModule::computeCarbonTransfers(double previousAwtd, double currentAwtd, double a, double b) {
		double transferAmount = 0;
		
		//at this moment, the water table depth value should be <=0, make it >=0
		double val1 = pow(fabs(previousAwtd), b);
		double val2 = pow(fabs(currentAwtd), b);		
		
		transferAmount = 10 * fabs(a * (val1 - val2));
	
		return transferAmount;
	}
}}} // namespace moja::modules::cbm
