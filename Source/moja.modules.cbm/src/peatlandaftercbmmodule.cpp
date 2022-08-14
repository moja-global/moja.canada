#include "moja/modules/cbm/peatlandaftercbmmodule.h"

#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * @brief Configuration function
	 * 
	 * @return void
	 * *************/
    void PeatlandAfterCBMModule::configure(const DynamicObject& config) { 		
	}

	/**
	 * @brief Subscribe to signals TimingStep and LocalDomainInit
	 * 
	 * @return void
	 * *************/
	void PeatlandAfterCBMModule::subscribe(NotificationCenter& notificationCenter) {		
		notificationCenter.subscribe(signals::TimingStep,       &PeatlandAfterCBMModule::onTimingStep,      *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandAfterCBMModule::onLocalDomainInit, *this);
	}

	/**
	 * @brief doLocalDomainInit
	 * 
	 * Assign pools PeatlandAfterCBMModule._acrotelm_o, PeatlandAfterCBMModule._catotelm_a, PeatlandAfterCBMModule._atmosphere, \n 
	 * PeatlandAfterCBMModule._softwoodFoliage, PeatlandAfterCBMModule._hardwoodFoliage \n,
	 * PeatlandAfterCBMModule._softwoodOther, PeatlandAfterCBMModule._hardwoodOther, PeatlandAfterCBMModule._softwoodFineRoots, \n
	 * PeatlandAfterCBMModule._hardwoodFineRoots, PeatlandAfterCBMModule._woodyFoliageDead, PeatlandAfterCBMModule._woodyStemsBranchesDead, \n 
	 * PeatlandAfterCBMModule._woodyRootsDead from _landUnitData
	 * 
	 * @return void
	 * *****************/
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

	/**
	 * @brief Perform on every timing step
	 * 
	 * Return if the value of the variable "spinup_moss_only" in _landUnitData is true, \n
	 * else, invoke PeatlandAfterCBMModule.transferCBMPoolToPeatland()
	 * 
	 * @return void
	 * **********************/
	void PeatlandAfterCBMModule::doTimingStep() {
		// When moss module is spinning up, nothing to grow, turnover and decay.
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
		if (spinupMossOnly) { return; }

		transferCBMPoolToPeatland();
	}
	
	/**
	 * @brief Transfer CBM pools to Peatland pools
	 * 
	 * If the value of the variable "peatlandId" in _landUnitData is 7, 8 or 9, i.e \n 
	 * if it is of forestry type, then there is a transfer from CBM to peatland pools
	 * 
	 *@return void
	 * ******************/
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
