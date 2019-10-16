#include "moja/modules/cbm/peatlandpreparemodule.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandPrepareModule::configure(const DynamicObject& config) {}

    void PeatlandPrepareModule::subscribe(NotificationCenter& notificationCenter) { 		
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandPrepareModule::onLocalDomainInit,  *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandPrepareModule::onTimingInit, *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandPrepareModule::onTimingStep, *this);
	}    

	void PeatlandPrepareModule::doLocalDomainInit(){
		_atmosphere = _landUnitData->getPool("Atmosphere");
		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");				

		baseWTDParameters = _landUnitData->getVariable("base_wtd_parameters")->value().extract<DynamicObject>();		
	}

    void PeatlandPrepareModule::doTimingInit() {
		//for forward run test and debug purpose
		//_landUnitData->getVariable("peatland_shrub_age")->set_value(0);

		//for each landunit pixel, reset water table depth variables
		resetWaterTableDepthValue();

		_runPeatland = _landUnitData->getVariable("run_peatland")->value();

		// if the land unit is eligible to run as peatland		
		if (_runPeatland) {
			peatlandID = _landUnitData->getVariable("peatlandId")->value();			
			checkTreedOrForestPeatland(peatlandID);

			//load initial peat pool values if it is enabled
			auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();
			if (loadInitialFlag){			
				const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_stocks")->value();
				loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());		
			}

			//get the long term average DC (drought code), compute long term water table depth
			double lnMeanDroughtCode = _landUnitData->getVariable("drought_class")->value();			
			double lwtd = computeWaterTableDepth(lnMeanDroughtCode, peatlandID);

			//set the long term water table depth variable value			
			_landUnitData->getVariable("peatland_longterm_wtd")->set_value(lwtd);				
			
			//for each peatland pixel, set the initial previous annual wtd same as the lwtd
			_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(lwtd);			 
		}
    }		

	void PeatlandPrepareModule::doTimingStep() {
		if (_runPeatland) {
			//get the current annual drought code;
			auto annualDC = _landUnitData->getVariable("annual_drought_class")->value();
			auto annualDroughtCode = annualDC.isEmpty() ? 0
				: annualDC.type() == typeid(TimeSeries) ? annualDC.extract<TimeSeries>().value()
				: annualDC.convert<double>();

			//compute the water table depth parameter to be used in current step
			double newCurrentYearWtd = computeWaterTableDepth(annualDroughtCode, peatlandID);			

			//get the potential annual water table modifer
			if (_landUnitData->hasVariable("peatland_annual_wtd_modifiers")) {			
				auto wtdModifier = _landUnitData->getVariable("peatland_annual_wtd_modifiers");
				std::string modifierStr = wtdModifier->value();
				bool modifyAnualWTD = modifierStr.size() > 1;

				if (modifyAnualWTD) {
					std::size_t firstPos = modifierStr.find_first_of(";");					
					std::string currentModifer;				
					std::string remainingModifiers;

					if (firstPos != std::string::npos) {
						std::string newModifier = modifierStr.substr(0, firstPos);
						currentModifer = newModifier.substr(newModifier.find_first_of("_") + 1);

						remainingModifiers = modifierStr.substr(firstPos + 1);						
					}
					else {
						currentModifer = modifierStr.substr(modifierStr.find_first_of("_") + 1);
						remainingModifiers = "";
					}

					int modifier = std::stoi(currentModifer);

					newCurrentYearWtd += modifier;

					_landUnitData->getVariable("peatland_annual_wtd_modifiers")->set_value(remainingModifiers);
				}
			}

			//get the current water table depth which was used in last step, but not yet updated for current step
			auto currentWtdVar = _landUnitData->getVariable("peatland_current_annual_wtd");
			double currentWtd = currentWtdVar->value();
			if (currentWtd != 0) {
				//set the previous annual wtd with the not updated annual water table depth value
				_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(currentWtd);
			}

			//update the current annual wtd 		
			_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(newCurrentYearWtd);
		}
	}
	
	void PeatlandPrepareModule::resetWaterTableDepthValue() {		
		_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(0.0);
		_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(0.0);
		_landUnitData->getVariable("peatland_longterm_wtd")->set_value(0.0);

		//also reset water table modifiers 
		_landUnitData->getVariable("peatland_annual_wtd_modifiers")->set_value("");
	}

	void PeatlandPrepareModule::checkTreedOrForestPeatland(int peatlandId) {
		switch (peatlandId) {
			case 3:		// forested bog
			case 6:		// forested poor fen
			case 9:		// forested rich fen
			case 11:	// forested swamp
				_isForestPeatland = true;
				break;
			case 2:		// treed bog
			case 5:		// treed poor fen
			case 8:		// treed rich fen
			case 10:	// treed swamp
				_isTreedPeatland = true;
				break;
			default:
				_isForestPeatland = false;
				_isTreedPeatland = false;
		}
	}

	double PeatlandPrepareModule::computeWaterTableDepth(double dc, int peatlandID) {
		std::string peatlandIDStr = std::to_string(peatlandID);
		double wtdBaseValue = baseWTDParameters[peatlandIDStr];
		double retVal = -0.045 * dc + wtdBaseValue;
		return retVal;
	}

	void PeatlandPrepareModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {		
		auto init = _landUnitData->createStockOperation();

		init->addTransfer(_atmosphere, _acrotelm_o, data["acrotelm"])
			->addTransfer(_atmosphere, _catotelm_a, data["catotelm"]);

		//MOJA_LOG_INFO << "Acrotelm: " << (double)data["acrotelm"] << " catotelm: " << (double)data["catotelm"];
		_landUnitData->submitOperation(init);
	}		
}}}