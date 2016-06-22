#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/logging.h"

#include "moja/modules/cbm/peatlanddecaymodule.h"
#include "moja/modules/cbm/printpools.h"

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandDecayModule::configure(const DynamicObject& config) { }

	void PeatlandDecayModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connect_signal(signals::LocalDomainInit, &PeatlandDecayModule::onLocalDomainInit, *this);
		notificationCenter.connect_signal(signals::TimingInit, &PeatlandDecayModule::onTimingInit, *this);
		notificationCenter.connect_signal(signals::TimingStep, &PeatlandDecayModule::onTimingStep, *this);
	}   

	void PeatlandDecayModule::onLocalDomainInit() {
		_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
		_woodyStemsBranchesDead = _landUnitData->getPool("WoodyStemsBranchesDead");
		_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");

		_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
		_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");

		_feathermossDead = _landUnitData->getPool("FeathermossDead");

		_acrotelm = _landUnitData->getPool("Acrotelm");
		_catotelm = _landUnitData->getPool("Catotelm");

		_co2 = _landUnitData->getPool("CO2");
		_ch4 = _landUnitData->getPool("CH4");

		_peatlandAge = _landUnitData->getVariable("age");			
    }

	void PeatlandDecayModule::onTimingInit() {
		// 1) get the data by variable "peatland_decay_parameters"
		const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();

		//create the PeaglandDecayParameters, set the value from the variable
		decayParas = std::make_shared<PeatlandDecayParameters>();
		decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

		// 2) get the data by variable "peatland_turnover_parameters"
		const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

		//create the PeaglandGrowthParameters, set the value from the variable
		turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
		turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());

		// 3) get the DC (drought code), and then compute the wtd parameter
		auto dc = _landUnitData->getVariable("drought_code")->value();

		//4) get the whater table function code
		auto wtdFunctionCode = _landUnitData->getVariable("wtd_function_code")->value();

		wtd = computeWTD(dc, wtdFunctionCode);
    }

	void PeatlandDecayModule::onTimingStep() {
		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
		updatePeatlandLivePoolValue();
		
		PrintPools::printPeatlandPools("Decay 0", *_landUnitData);

		double deadPoolTurnoverRate = decayParas->turnoverRate(); // 15% to acrotelm, 75% to air		

		doPeatlandDecay(deadPoolTurnoverRate);
		doDeadPoolTurnover(deadPoolTurnoverRate);	
    }

	void PeatlandDecayModule::doDeadPoolTurnover(double deadPoolTurnoverRate) {
		auto peatlandDeadPoolTurnover = _landUnitData->createProportionalOperation();
		peatlandDeadPoolTurnover

			->addTransfer(_woodyFoliageDead, _acrotelm, (turnoverParas->Pfe() * decayParas->akwfe() + 
														turnoverParas->Pfn() * decayParas->akwfne()) * deadPoolTurnoverRate)
			->addTransfer(_woodyStemsBranchesDead, _acrotelm, decayParas->akwsb() * deadPoolTurnoverRate)			
			->addTransfer(_woodyRootsDead, _acrotelm, decayParas->akwr() * deadPoolTurnoverRate)
			->addTransfer(_sedgeFoliageDead, _acrotelm, decayParas->aksf() * deadPoolTurnoverRate)
			->addTransfer(_sedgeRootsDead, _acrotelm, decayParas->aksr() * deadPoolTurnoverRate)
			->addTransfer(_feathermossDead, _acrotelm, decayParas->akfm() * deadPoolTurnoverRate)			
			->addTransfer(_acrotelm, _catotelm, decayParas->aka() * deadPoolTurnoverRate);
		_landUnitData->submitOperation(peatlandDeadPoolTurnover);
	}

	/*
	ToAirTotal = 
		(D_W_Foliage *(1-Pt)(Pfe*akwfe)+ Pfn*akwfne))+
		(D_W_StemsBranches *(1-Pt)*akwsb+
		(D_W_Roots *(1-Pt)*akwr+
		(D_S_Foliage *(1-Pt)*aksf+
		(D_S_Roots *(1-Pt)*aksr+
		(D_Feather_Moss *(1-Pt)*akfm+
		(Acrotelm *(1-Pt)*aka+
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

			->addTransfer(_acrotelm, _co2, getToCO2Rate(decayParas->aka(), deadPoolTurnoverRate))
			->addTransfer(_acrotelm, _ch4, getToCH4Rate(decayParas->aka(), deadPoolTurnoverRate))

			->addTransfer(_catotelm, _co2, getToCO2Rate(decayParas->akc(), 0.0))
			->addTransfer(_catotelm, _ch4, getToCH4Rate(decayParas->akc(), 0.0));
		_landUnitData->submitOperation(peatlandDeadPoolDecay);
	}

	double PeatlandDecayModule::getToCH4Rate(double rate, double deadPoolTurnoverRate){
		double retVal = rate * (1 - deadPoolTurnoverRate) * (wtd * decayParas->c() + decayParas->d());
		return retVal;
	}

	double PeatlandDecayModule::getToCO2Rate(double rate, double deadPoolTurnoverRate){
		double retVal = rate* (1 - deadPoolTurnoverRate) *( 1 - (wtd * decayParas->c() + decayParas->d()));
		return retVal;
	}

	double PeatlandDecayModule::computeWTD(double dc, int functionCode){
		double retVal = 0;

		switch (functionCode)
		{
		case 1:
			retVal = -20.8 - 0.054 * dc;
			break;
		case 2:
			retVal = -20.8 - 0.054 * dc + 19.2;
			break;
		case 3:
			retVal = -20.8 - 0.054 * dc + 12.3;
			break;
		case 4:
			retVal = -20.8 - 0.054 * dc + 33.2;
			break;
		}
		return retVal;

	}

	void PeatlandDecayModule::updatePeatlandLivePoolValue(){
		double v1 = _woodyFoliageDead->value();
		double v2 = _woodyStemsBranchesDead->value();
		double v3 = _woodyRootsDead->value();
		double v4 = _sedgeFoliageDead->value();
		double v5 = _sedgeRootsDead->value();
		double v6 = _feathermossDead->value();
		double v7 = _acrotelm->value();
		double v8  = _catotelm->value();
		double v9  = _co2->value();
		double v10  = _ch4->value();
	}
}}} // namespace moja::modules::cbm
