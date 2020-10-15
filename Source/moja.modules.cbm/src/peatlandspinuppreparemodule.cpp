#include "moja/modules/cbm/peatlandspinuppreparemodule.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>
#include <moja/exception.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/modules/cbm/peatlands.h>

namespace moja {
namespace modules {
namespace cbm {

    void PeatlandSpinupPrepareModule::configure(const DynamicObject& config) {}

    void PeatlandSpinupPrepareModule::subscribe(NotificationCenter& notificationCenter) { 		
		notificationCenter.subscribe(signals::LocalDomainInit,  &PeatlandSpinupPrepareModule::onLocalDomainInit,  *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandSpinupPrepareModule::onTimingInit, *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandSpinupPrepareModule::onTimingStep, *this);
	}    

	void PeatlandSpinupPrepareModule::doLocalDomainInit(){
		_atmosphere = _landUnitData->getPool("Atmosphere");
		_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
		_catotelm_a = _landUnitData->getPool("Catotelm_A");				

		//get the basic water table modifier parameters for each peatland, which is defined in varible.json
		baseWTDParameters = _landUnitData->getVariable("base_wtd_parameters")->value().extract<DynamicObject>();	
	}

    void PeatlandSpinupPrepareModule::doTimingInit() {		
		_runPeatland = _landUnitData->getVariable("run_peatland")->value();

		// if the land unit is eligible to run as peatland		
		if (_runPeatland) {
			peatlandID = _landUnitData->getVariable("peatlandId")->value();			

			//load initial peat pool values if it is enabled
			auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();
			if (loadInitialFlag){			
				const auto& peatlandInitials = _landUnitData->getVariable("peatland_initial_stocks")->value();
				loadPeatlandInitialPoolValues(peatlandInitials.extract<DynamicObject>());		
			}

			//get the long term average DC (drought code), compute long term water table depth
			auto& lnMDroughtCode = _landUnitData->getVariable("spinup_drought_class")->value();
			auto& defaultLMDC = _landUnitData->getVariable("default_spinup_drought_class")->value();
			auto lnMeanDroughtCode = lnMDroughtCode.isEmpty() ? defaultLMDC : lnMDroughtCode;
			double lwtd = computeWaterTableDepth(lnMeanDroughtCode, peatlandID);

			//set identical water table depth values for three water table variables in spinup phase			
			_landUnitData->getVariable("peatland_longterm_wtd")->set_value(lwtd);		
			_landUnitData->getVariable("peatland_previous_annual_wtd")->set_value(lwtd);
			_landUnitData->getVariable("peatland_current_annual_wtd")->set_value(lwtd);			
		}
    }		

	double PeatlandSpinupPrepareModule::computeWaterTableDepth(double dc, int peatlandID) {
		std::string peatlandIDStr = std::to_string(peatlandID);
		double wtdBaseValue = baseWTDParameters[peatlandIDStr];
		double retVal = -0.045 * dc + wtdBaseValue;
		return retVal;
	}

	void PeatlandSpinupPrepareModule::loadPeatlandInitialPoolValues(const DynamicObject& data) {		
		auto init = _landUnitData->createStockOperation();

		init->addTransfer(_atmosphere, _acrotelm_o, data["acrotelm"])
			->addTransfer(_atmosphere, _catotelm_a, data["catotelm"]);
		_landUnitData->submitOperation(init);
	}		
}}}