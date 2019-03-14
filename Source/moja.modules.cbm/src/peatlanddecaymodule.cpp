#include "moja/modules/cbm/peatlanddecaymodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandDecayModule::configure(const DynamicObject& config) { }

	void PeatlandDecayModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandDecayModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandDecayModule::onTimingInit, *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandDecayModule::onTimingStep, *this);
	}   

	void PeatlandDecayModule::doLocalDomainInit() {
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

		_co2 = _landUnitData->getPool("CO2");
		_ch4 = _landUnitData->getPool("CH4");			
    }

	void PeatlandDecayModule::doTimingInit() {
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }
		// 1) get the data by variable "peatland_decay_parameters"
		const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();

		//get the mean anual temperture variable
		double meanAnnualTemperature = _landUnitData->getVariable("mean_annual_temperature")->value();

		//create the PeaglandDecayParameters, set the value from the variable
		decayParas = std::make_shared<PeatlandDecayParameters>();
		decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

		//compute the applied parameters
		decayParas->updateAppliedDecayParameters(meanAnnualTemperature);

		// 2) get the data by variable "peatland_turnover_parameters"
		const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

		//create the PeaglandGrowthParameters, set the value from the variable
		turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
		if (!peatlandTurnoverParams.isEmpty()) {
		turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());
		}

		// 3) get the DC (drought code), and then compute the wtd parameter
		awtd = _landUnitData->getVariable("peatland_current_annual_wtd")->value();
    }

	void PeatlandDecayModule::doTimingStep() {
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
		if (spinupMossOnly) { return; }
				
		//test degug output, time to print the pool values to check
		//PrintPools::printPeatlandPools("Year ", *_landUnitData);
		double deadPoolTurnoverRate = decayParas->Pt(); 	

		doDeadPoolTurnover(deadPoolTurnoverRate);
		doPeatlandDecay(deadPoolTurnoverRate);				
    }
	
	void PeatlandDecayModule::doDeadPoolTurnover(double deadPoolTurnoverRate) {
		auto peatlandDeadPoolTurnover = _landUnitData->createProportionalOperation();
		peatlandDeadPoolTurnover
			->addTransfer(_woodyFoliageDead, _acrotelm_o, (turnoverParas->Pfe() * decayParas->akwfe() + 
														turnoverParas->Pfn() * decayParas->akwfne()) * deadPoolTurnoverRate)
			->addTransfer(_woodyFineDead, _acrotelm_o, decayParas->akwsb() * deadPoolTurnoverRate)	
			->addTransfer(_woodyCoarseDead, _acrotelm_o, decayParas->akwc() * deadPoolTurnoverRate)
			->addTransfer(_woodyRootsDead, _acrotelm_o, decayParas->akwr() * deadPoolTurnoverRate)
			->addTransfer(_sedgeFoliageDead, _acrotelm_o, decayParas->aksf() * deadPoolTurnoverRate)
			->addTransfer(_sedgeRootsDead, _acrotelm_o, decayParas->aksr() * deadPoolTurnoverRate)
			->addTransfer(_feathermossDead, _acrotelm_o, decayParas->akfm() * deadPoolTurnoverRate)
			->addTransfer(_acrotelm_o, _catotelm_a, decayParas->aka() * deadPoolTurnoverRate);
		_landUnitData->submitOperation(peatlandDeadPoolTurnover);		
	}

	/**
	ToAirTotal = 
		(D_W_Foliage *(1-Pt)(Pfe*akwfe) + Pfn*akwfne)) +
		(D_W_StemsBranches *(1-Pt)*akwsb +
		(D_W_Roots *(1-Pt)*akwr +
		(D_S_Foliage *(1-Pt)*aksf +
		(D_S_Roots *(1-Pt)*aksr +
		(D_Feather_Moss *(1-Pt)*akfm +
		(Acrotelm *(1-Pt)*aka +
		(Catotelm *'akc) 	

	ToCH4 = ToAirTotal * ((c * wtd) + d)
	ToCO2 = ToAirTotal - ToCH4
	*/
	void PeatlandDecayModule::doPeatlandDecay(double deadPoolTurnoverRate) {
		//special turnover rate for catotelm pool -> (Catotelm *'akc) and (Acrotelm *(1-Pt)*aka 
		//set zeroTurnoverRate to utilize the getToCO2Rate() and getToCH4Rate() function
		double zeroTurnoverRate = 0.0;

		auto peatlandDeadPoolDecay = _landUnitData->createProportionalOperation();
		peatlandDeadPoolDecay
			->addTransfer(_acrotelm_o, _co2, getToCO2Rate(decayParas->aka(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm_o, _ch4, getToCH4Rate(decayParas->aka(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm_a, _co2, getToCO2Rate(decayParas->akaa(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm_a, _ch4, getToCH4Rate(decayParas->akaa(), deadPoolTurnoverRate))

			->addTransfer(_catotelm_a, _co2, getToCO2Rate(decayParas->akc(), zeroTurnoverRate))
			->addTransfer(_catotelm_a, _ch4, getToCH4Rate(decayParas->akc(), zeroTurnoverRate))
			->addTransfer(_catotelm_o, _co2, getToCO2Rate(decayParas->akco(), zeroTurnoverRate))
			->addTransfer(_catotelm_o, _ch4, getToCH4Rate(decayParas->akco(), zeroTurnoverRate))

			->addTransfer(_woodyFoliageDead, _co2, getToCO2Rate((turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()), deadPoolTurnoverRate))
			->addTransfer(_woodyFoliageDead, _ch4, getToCH4Rate((turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()), deadPoolTurnoverRate))

			->addTransfer(_woodyFineDead, _co2, getToCO2Rate(decayParas->akwsb(), deadPoolTurnoverRate))
			->addTransfer(_woodyFineDead, _ch4, getToCH4Rate(decayParas->akwsb(), deadPoolTurnoverRate))
			->addTransfer(_woodyCoarseDead, _co2, getToCO2Rate(decayParas->akwc(), deadPoolTurnoverRate))
			->addTransfer(_woodyCoarseDead, _ch4, getToCH4Rate(decayParas->akwc(), deadPoolTurnoverRate))

			->addTransfer(_woodyRootsDead, _co2, getToCO2Rate(decayParas->akwr(), deadPoolTurnoverRate))
			->addTransfer(_woodyRootsDead, _ch4, getToCH4Rate(decayParas->akwr(), deadPoolTurnoverRate))

			->addTransfer(_sedgeFoliageDead, _co2, getToCO2Rate(decayParas->aksf(), deadPoolTurnoverRate))
			->addTransfer(_sedgeFoliageDead, _ch4, getToCH4Rate(decayParas->aksf(), deadPoolTurnoverRate))

			->addTransfer(_sedgeRootsDead, _co2, getToCO2Rate(decayParas->aksr(), deadPoolTurnoverRate))
			->addTransfer(_sedgeRootsDead, _ch4, getToCH4Rate(decayParas->aksr(), deadPoolTurnoverRate))

			->addTransfer(_feathermossDead, _co2, getToCO2Rate(decayParas->akfm(), deadPoolTurnoverRate))
			->addTransfer(_feathermossDead, _ch4, getToCH4Rate(decayParas->akfm(), deadPoolTurnoverRate));			
		_landUnitData->submitOperation(peatlandDeadPoolDecay);		
	}

	double PeatlandDecayModule::getToCH4Rate(double rate, double deadPoolTurnoverRate){
		double retVal = rate * (1 - deadPoolTurnoverRate) * (awtd * decayParas->c() + decayParas->d());		
		return retVal;
	}

	double PeatlandDecayModule::getToCO2Rate(double rate, double deadPoolTurnoverRate){
		double retVal = rate* (1 - deadPoolTurnoverRate) * ( 1 - (awtd * decayParas->c() + decayParas->d()));		
		return retVal;
	}

}}} // namespace moja::modules::cbm
