#include "moja/flint/variable.h"
#include "moja/logging.h"

#include "moja/modules/cbm/peatlandpreparemodule.h"

#include <boost/algorithm/string.hpp> 

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandPrepareModule::configure(const DynamicObject& config) { 		
	}

    void PeatlandPrepareModule::subscribe(NotificationCenter& notificationCenter) { 
		notificationCenter.subscribe(signals::TimingInit,       &PeatlandPrepareModule::onTimingInit,       *this);
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandPrepareModule::onLocalDomainInit,  *this);
	}
    

	void PeatlandPrepareModule::doLocalDomainInit(){
		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");
		_atmosphere = _landUnitData->getPool("Atmosphere");
	}

    void PeatlandPrepareModule::doTimingInit() {		
		bool enablePeatland = _landUnitData->getVariable("enable_peatland")->value();
		if (!enablePeatland) { return; }

		auto peatlandId = _landUnitData->getVariable("peatland_class")->value();
		int peatland_id = peatlandId.isEmpty() ? -1 : peatlandId;

		if (peatland_id > 0){
			MOJA_LOG_INFO << "Found peatland with id: " << peatland_id;
		}

		//set the peatland id for current land unit
		_landUnitData->getVariable("peatlandId")->set_value(peatland_id);
	
		//run peatland when it is enabled, and the peatland id is valid
		_runPeatland = (_landUnitData->getVariable("enable_peatland")->value()) && (peatland_id > 0);
		_landUnitData->getVariable("run_peatland")->set_value(_runPeatland);		

		// if the land unit is eligible to run as peatland
		// get the initial pool values, and long term water table depth
		if (_runPeatland) {
			const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_stocks")->value();
			loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());		

			//get the DC (drought code), and then compute the wtd parameter
			double dc = _landUnitData->getVariable("drought_class")->value();

			//get long term water table depth, and set the variable value
			double lwtd = computeLongtermWTD(dc, peatland_id);
			_landUnitData->getVariable("peatland_longterm_wtd")->set_value(lwtd);

			//for each peatland landunit, set the initial previous annual wtd as the lwtd
			_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(lwtd);

			//for each peatland landunit, set the current annual wtd as the lwtd temporarily		
			_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(lwtd);

			//transfer some of CBM pools to peatland pools accordingly
			transferCBMPoolToPeatland();

			int cbmAge = _landUnitData->getVariable("age")->value();
			_landUnitData->getVariable("peatland_age")->set_value(cbmAge);
			MOJA_LOG_INFO << "CBM stand / Peatland initial age: " << cbmAge;
		}
    }

	void PeatlandPrepareModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {	
		double tic = data["acrotelm"] + data["catotelm"];//total initial carbon
		_landUnitData->getVariable("peatland_total_initial_carbon")->set_value(tic);
		
		auto init = _landUnitData->createStockOperation();	
		init->addTransfer(_atmosphere, _acrotelm_o, data["acrotelm"])
			->addTransfer(_atmosphere, _catotelm_a, data["catotelm"]);

		MOJA_LOG_INFO << "Acrotelm: " << (double)data["acrotelm"] << " catotelm: " << (double)data["catotelm"];

		_landUnitData->submitOperation(init);		
	}

	void PeatlandPrepareModule::transferCBMPoolToPeatland() {
		int peatland_id = _landUnitData->getVariable("peatlandId")->value();

		//if peatland is of foresty type, aka, peatland_id is one of the following number		
		int forest_peatland_bog = 7;
		int forest_peatland_poorfen = 8;
		int forest_peatland_richfen = 9;

		double cbmToPeatlandRate = 0.5;

		if (peatland_id == forest_peatland_bog
			|| peatland_id == forest_peatland_poorfen
			|| peatland_id == forest_peatland_richfen) {

			auto cbmToPeatland = _landUnitData->createProportionalOperation();
			cbmToPeatland
				->addTransfer(_softwoodFoliage, _woodyFoliageDead, cbmToPeatlandRate)
				->addTransfer(_hardwoodFoliage, _woodyFoliageDead, cbmToPeatlandRate)
				->addTransfer(_softwoodOther, _woodyStemsBranchesDead, cbmToPeatlandRate)
				->addTransfer(_hardwoodOther, _woodyStemsBranchesDead, cbmToPeatlandRate)
				->addTransfer(_softwoodFineRoots, _woodyRootsDead, cbmToPeatlandRate)
				->addTransfer(_hardwoodFineRoots, _woodyRootsDead, cbmToPeatlandRate);

			_landUnitData->submitOperation(cbmToPeatland);
		}
	}

	/**
	Function code is now based on peatland id
	*/
	double PeatlandPrepareModule::computeLongtermWTD(double dc, int peatlandID) {
		double retVal = 0;

		switch (peatlandID)
		{
		case 1:
		case 2:
		case 3:
			retVal = -20.8 - 0.054 * dc;
			break;
		case 4:
		case 5:
		case 8:
			retVal = -20.8 - 0.054 * dc + 19.2;
			break;
		case 6:
		case 9:
		case 10:
		case 11:
			retVal = -20.8 - 0.054 * dc + 12.3;
			break;
		case 7:
			retVal = -20.8 - 0.054 * dc + 33.2;
			break;
		}

		MOJA_LOG_INFO << "Drought Code: " << dc << " Peatland ID: " << peatlandID << " LWTD: " << retVal;
		return retVal;
	}
}}}

