#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/logging.h"

#include "moja/modules/cbm/peatlanddecaymodule.h"
#include "moja/modules/cbm/printpools.h"

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandDecayModule::configure(const DynamicObject& config) { }

	void PeatlandDecayModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandDecayModule::onLocalDomainInit, *this);
		notificationCenter.subscribe(signals::TimingInit, &PeatlandDecayModule::onTimingInit, *this);
		notificationCenter.subscribe(signals::TimingStep, &PeatlandDecayModule::onTimingStep, *this);
	}   

	void PeatlandDecayModule::onLocalDomainInit() {
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
		_c_accumulation = _landUnitData->getPool("C_Accumulation");
		_co2 = _landUnitData->getPool("CO2");
		_ch4 = _landUnitData->getPool("CH4");

		_peatlandAge = _landUnitData->getVariable("age");			
    }

	void PeatlandDecayModule::onTimingInit() {
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }
		// 1) get the data by variable "peatland_decay_parameters"
		const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();

		//create the PeaglandDecayParameters, set the value from the variable
		decayParas = std::make_shared<PeatlandDecayParameters>();
		decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

		// 2) get the data by variable "peatland_turnover_parameters"
		const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

		//create the PeaglandGrowthParameters, set the value from the variable
		turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
		if (!peatlandTurnoverParams.isEmpty()) {
		turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());
		}
		// 3) get the DC (drought code), and then compute the wtd parameter
		lwtd = _landUnitData->getVariable("peatland_longterm_wtd")->value();

		//4) get the whater table function code
		tic = _landUnitData->getVariable("peatland_total_initial_carbon")->value();

    }

	void PeatlandDecayModule::onTimingStep() {
		bool runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (!runPeatland){ return; }
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
		if (spinupMossOnly) { return; }
		
		//time to print the pool values to check
		PrintPools::printPeatlandPools("Year ", *_landUnitData);

		double deadPoolTurnoverRate = decayParas->turnoverRate(); // 15% to acrotelm, 75% to air		

		doDeadPoolTurnover(deadPoolTurnoverRate);
		doPeatlandDecay(deadPoolTurnoverRate);
		doAccumulation(deadPoolTurnoverRate);
    }

	void PeatlandDecayModule::doAccumulation(double deadPoolTurnoverRate) {
		double acrotelm_o = _acrotelm_o->value();
		double catotelm_a = _catotelm_a->value();
		double acrotelm_a = _acrotelm_a->value();
		double catotelm_o = _catotelm_o->value();
		double accumulatedValue = 0;
		if ((acrotelm_o + catotelm_a + acrotelm_a + catotelm_o) > tic) {
			accumulatedValue = (catotelm_o + catotelm_a) - ((acrotelm_o + acrotelm_a + catotelm_o + catotelm_a) - tic);
			if (accumulatedValue > 0){
				auto peatlandAccumulation = _landUnitData->createStockOperation();
				peatlandAccumulation
					->addTransfer(_catotelm_a, _c_accumulation, accumulatedValue);
				_landUnitData->submitOperation(peatlandAccumulation);
			}			
		}		
	}
	void PeatlandDecayModule::doDeadPoolTurnover(double deadPoolTurnoverRate) {
		auto peatlandDeadPoolTurnover = _landUnitData->createProportionalOperation();
		peatlandDeadPoolTurnover

			->addTransfer(_woodyFoliageDead, _acrotelm_o, (turnoverParas->Pfe() * decayParas->akwfe() + 
														turnoverParas->Pfn() * decayParas->akwfne()) * deadPoolTurnoverRate)
			->addTransfer(_woodyStemsBranchesDead, _acrotelm_o, decayParas->akwsb() * deadPoolTurnoverRate)			
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


	ToCH4 =  ToAirTotal * ((c * wtd) + d)
	ToCO2 = ToAirTotal = ToCH4
	*/
	void PeatlandDecayModule::doPeatlandDecay(double deadPoolTurnoverRate) {
		auto peatlandDeadPoolDecay = _landUnitData->createProportionalOperation();
		peatlandDeadPoolDecay
			->addTransfer(_woodyFoliageDead, _co2, getToCO2Rate((turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()), deadPoolTurnoverRate))
			->addTransfer(_woodyFoliageDead, _ch4, getToCH4Rate((turnoverParas->Pfn() * decayParas->akwfne() + turnoverParas->Pfe() * decayParas->akwfe()), deadPoolTurnoverRate))

			->addTransfer(_woodyStemsBranchesDead, _co2, getToCO2Rate(decayParas->akwsb(), deadPoolTurnoverRate))
			->addTransfer(_woodyStemsBranchesDead, _ch4, getToCH4Rate(decayParas->akwsb(), deadPoolTurnoverRate))

			->addTransfer(_woodyRootsDead, _co2, getToCO2Rate(decayParas->akwr(), deadPoolTurnoverRate))
			->addTransfer(_woodyRootsDead, _ch4, getToCH4Rate(decayParas->akwr(), deadPoolTurnoverRate))

			->addTransfer(_sedgeFoliageDead, _co2, getToCO2Rate(decayParas->aksf(), deadPoolTurnoverRate))
			->addTransfer(_sedgeFoliageDead, _ch4, getToCH4Rate(decayParas->aksf(), deadPoolTurnoverRate))

			->addTransfer(_sedgeRootsDead, _co2, getToCO2Rate(decayParas->aksr(), deadPoolTurnoverRate))
			->addTransfer(_sedgeRootsDead, _ch4, getToCH4Rate(decayParas->aksr(), deadPoolTurnoverRate))

			->addTransfer(_feathermossDead, _co2, getToCO2Rate(decayParas->akfm(), deadPoolTurnoverRate))
			->addTransfer(_feathermossDead, _ch4, getToCH4Rate(decayParas->akfm(), deadPoolTurnoverRate))

			->addTransfer(_acrotelm_o, _co2, getToCO2Rate(decayParas->aka(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm_o, _ch4, getToCH4Rate(decayParas->aka(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm_a, _co2, getToCO2Rate(decayParas->akaa(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm_a, _ch4, getToCH4Rate(decayParas->akaa(), deadPoolTurnoverRate))

			->addTransfer(_catotelm_a, _co2, getToCO2Rate(decayParas->akc(), deadPoolTurnoverRate))
			->addTransfer(_catotelm_a, _ch4, getToCH4Rate(decayParas->akc(), deadPoolTurnoverRate))
			->addTransfer(_catotelm_o, _co2, getToCO2Rate(decayParas->akco(), deadPoolTurnoverRate))
			->addTransfer(_catotelm_o, _ch4, getToCH4Rate(decayParas->akco(), deadPoolTurnoverRate))
			->addTransfer(_c_accumulation, _co2, getToCO2Rate(decayParas->akc(), deadPoolTurnoverRate))
			->addTransfer(_c_accumulation, _ch4, getToCH4Rate(decayParas->akc(), deadPoolTurnoverRate));
		_landUnitData->submitOperation(peatlandDeadPoolDecay);
	}

	double PeatlandDecayModule::getToCH4Rate(double rate, double deadPoolTurnoverRate){
		double retVal = rate * (1 - deadPoolTurnoverRate) * (lwtd * decayParas->c() + decayParas->d());		
		return retVal;
	}

	double PeatlandDecayModule::getToCO2Rate(double rate, double deadPoolTurnoverRate){
		double retVal = rate* (1 - deadPoolTurnoverRate) *( 1 - (lwtd * decayParas->c() + decayParas->d()));		
		return retVal;
	}


	

}}} // namespace moja::modules::cbm
