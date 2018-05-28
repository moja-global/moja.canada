#include "moja/modules/cbm/peatlandpreparemodule.h"

#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/algorithm/string.hpp> 
#include <moja/timeseries.h>

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandPrepareModule::configure(const DynamicObject& config) { 		
	}

    void PeatlandPrepareModule::subscribe(NotificationCenter& notificationCenter) { 		
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandPrepareModule::onLocalDomainInit,  *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandPrepareModule::onTimingStep, *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandPrepareModule::onTimingInit, *this);
	}    

	void PeatlandPrepareModule::doLocalDomainInit(){
		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");
		_atmosphere = _landUnitData->getPool("Atmosphere");
		
		_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
		_softwoodOther = _landUnitData->getPool("SoftwoodOther");		
		_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");		
		_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
		_hardwoodOther = _landUnitData->getPool("HardwoodOther");	
		_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");	
		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyStemsBranchesDead = _landUnitData->getPool("WoodyStemsBranchesDead");
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");	
	}

    void PeatlandPrepareModule::doTimingInit() {
		_runPeatland = _landUnitData->getVariable("run_peatland")->value();

		// if the land unit is eligible to run as peatland
		// get the initial pool values, and long term water table depth
		if (_runPeatland) {
			int peatlandId = _landUnitData->getVariable("peatlandId")->value();
			_isForestPeatland = isForestPeatland(peatlandId);

			auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();

			if (loadInitialFlag){			
				const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_stocks")->value();
				loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());		
			}

			//get the DC (drought code), and then compute the wtd parameter
			double lnMeanDroughtCode = _landUnitData->getVariable("drought_class")->value();			

			//get long term water table depth, and set the variable value
			double lwtd = computeLongtermWTD(lnMeanDroughtCode, peatlandId);
			_landUnitData->getVariable("peatland_longterm_wtd")->set_value(lwtd);

			//auto annualDC = _landUnitData->getVariable("annual_drought_class")->value();
			auto annualDC = _landUnitData->getVariable("annual_drought_class")->value();
			auto annualDroughtCode = annualDC.isEmpty() ? 0
				: annualDC.type() == typeid(TimeSeries) ? annualDC.extract<TimeSeries>().value()
				: annualDC.convert<double>();

			double currentYearWtd = computeLongtermWTD(annualDroughtCode, peatlandId);

			//get the current annual wtd which is not updated yet
			double currentWtd = _landUnitData->getVariable("peatland_current_annual_wtd")->value();

			if (currentWtd == 0) {
				//for each peatland landunit, set the initial previous annual wtd as the lwtd
				_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(lwtd);
			} else{
			//for each peatland landunit, set the previous annual wtd as the currentWtd
			_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(currentWtd);
			}
			//for each peatland landunit, set the current annual wtd 		
			_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(currentYearWtd);
		}
    }

	void PeatlandPrepareModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {	
		double tic = data["acrotelm"] + data["catotelm"];//total initial carbon
		_landUnitData->getVariable("peatland_total_initial_carbon")->set_value(tic);
		
		auto init = _landUnitData->createStockOperation();	
		init->addTransfer(_atmosphere, _acrotelm_o, data["acrotelm"])
			->addTransfer(_atmosphere, _catotelm_a, data["catotelm"]);

		//MOJA_LOG_INFO << "Acrotelm: " << (double)data["acrotelm"] << " catotelm: " << (double)data["catotelm"];
		_landUnitData->submitOperation(init);		
	}

	void PeatlandPrepareModule::doTimingStep() {
		_runPeatland = _landUnitData->getVariable("run_peatland")->value();

		// if the land unit is eligible to run as peatland
		// and the peatland is foresty type peatland
		if (_runPeatland && _isForestPeatland) {
			//transfer some of CBM pools to peatland pools accordingly
			transferCBMPoolToPeatland();
		}
	}

	void PeatlandPrepareModule::transferCBMPoolToPeatland() {
		double cbmToPeatlandRate = _landUnitData->getVariable("cbm_to_peatland_rate")->value();	

		//check if growth curve is associated for the forested peatland
		bool isGrowthCurveDefined =  _landUnitData->getVariable("growth_curve_id")->value() > 0;		

		if (isGrowthCurveDefined) {
			//MOJA_LOG_INFO << "Peatland ID: " << peatlandId <<  ">> to transfer CBM pools to peatland pools";			
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

	bool PeatlandPrepareModule::isForestPeatland(int peatlandId) {
		//if peatland is of foresty type, aka, peatland_id is one of the following number		
		int forest_peatland_bog = 3;
		int forest_peatland_poorfen = 6;
		int forest_peatland_richfen = 9;
		int forest_peatland_swamp = 11;

		if (peatlandId == forest_peatland_bog
			|| peatlandId == forest_peatland_poorfen
			|| peatlandId == forest_peatland_richfen
			|| peatlandId == forest_peatland_swamp) {
			return true;
		}

		return false;
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
	
		if (retVal > 0) { 
			retVal = 0; 
		}

		return retVal;
	}
}}}


