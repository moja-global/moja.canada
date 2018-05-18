#include "moja/modules/cbm/peatlandaftercbmmodule.h"

#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandAfterCBMModule::configure(const DynamicObject& config) { 		
	}

	void PeatlandAfterCBMModule::subscribe(NotificationCenter& notificationCenter) {		
		notificationCenter.subscribe(signals::TimingStep,       &PeatlandAfterCBMModule::onTimingStep,      *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandAfterCBMModule::onLocalDomainInit, *this);
	}
    
	void PeatlandAfterCBMModule::doLocalDomainInit() {
		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");
		_atmosphere = _landUnitData->getPool("Atmosphere"); 		

		_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
		_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");

		_softwoodOther = _landUnitData->getPool("SoftwoodOther");
		_hardwoodOther = _landUnitData->getPool("HardwoodOther");

		_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");
		_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyStemsBranchesDead = _landUnitData->getPool("WoodyStemsBranchesDead");
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");
	}  

	void PeatlandAfterCBMModule::doTimingStep() {
		// When moss module is spinning up, nothing to grow, turnover and decay.
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
		if (spinupMossOnly) { return; }

		transferCBMPoolToPeatland();
	}
	
	void PeatlandAfterCBMModule::transferCBMPoolToPeatland() {
		int peatland_id = _landUnitData->getVariable("peatlandId")->value();

		//if peatland is of foresty type, aka, peatland_id is one of the following number
		//to be implemented further
		int forest_peatland_bog = 7;
		int forest_peatland_poorfen = 8;
		int forest_peatland_richfen = 9;

		double cbmToPeatlandRate = 0.5;

		if (peatland_id == forest_peatland_bog
			|| peatland_id == forest_peatland_poorfen
			|| peatland_id == forest_peatland_richfen) {

			MOJA_LOG_INFO << "Transfer CBM pools to peatland pools";

			/*
			auto cbmToPeatland = _landUnitData->createProportionalOperation();
			cbmToPeatland
			->addTransfer(_softwoodFoliage, _woodyFoliageDead, cbmToPeatlandRate)
			->addTransfer(_hardwoodFoliage, _woodyFoliageDead, cbmToPeatlandRate)
			->addTransfer(_softwoodOther, _woodyStemsBranchesDead, cbmToPeatlandRate)
			->addTransfer(_hardwoodOther, _woodyStemsBranchesDead, cbmToPeatlandRate)
			->addTransfer(_softwoodFineRoots, _woodyRootsDead, cbmToPeatlandRate)
			->addTransfer(_hardwoodFineRoots, _woodyRootsDead, cbmToPeatlandRate);

			_landUnitData->submitOperation(cbmToPeatland);
			*/
		}
	}
}}}
