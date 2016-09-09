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

		_acrotelm = _landUnitData->getPool("Acrotelm");
		_catotelm = _landUnitData->getPool("Catotelm");

		_peatlandAge = _landUnitData->getVariable("age");		
    }

	void PeatlandTurnoverModule::onTimingInit() {
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
    }

	void PeatlandTurnoverModule::onTimingStep() {
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();		

		//update the current pool value
		updatePeatlandLivePoolValue();		

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
			->addTransfer(_sphagnumMossLive, _acrotelm, sphagnumMossLive * 1.0);	

		_landUnitData->submitOperation(peatlandTurnover);		
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

}}} // namespace moja::modules::cbm