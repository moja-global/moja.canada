#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/observer.h"
#include "moja/logging.h"

#include "moja/modules/cbm/peatlandgrowthmodule.h"
#include "moja/modules/cbm/printpools.h"

namespace moja {
namespace modules {
namespace cbm {

	void PeatlandGrowthModule::configure(const DynamicObject& config) { }

	void PeatlandGrowthModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connect_signal(signals::LocalDomainInit, &PeatlandGrowthModule::onLocalDomainInit, *this);
		notificationCenter.connect_signal(signals::TimingInit, &PeatlandGrowthModule::onTimingInit, *this);
		notificationCenter.connect_signal(signals::TimingStep, &PeatlandGrowthModule::onTimingStep, *this);
	}   

	void PeatlandGrowthModule::onLocalDomainInit() {
		_atmosphere = _landUnitData->getPool("Atmosphere");

		_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
		_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
		_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");
		_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
		_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");
		_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
		_featherMossLive = _landUnitData->getPool("FeatherMossLive");		

		_peatlandAge = _landUnitData->getVariable("age");
    }

	void PeatlandGrowthModule::onTimingInit() {
		// get the data by variable "peatland_growth_parameters"
		const auto& peatlandGrowthParams = _landUnitData->getVariable("peatland_growth_parameters")->value();

		//create the PeatlandGrowthParameters, set the value from the variable
		growthParas = std::make_shared<PeatlandGrowthParameters>();
		growthParas->setValue(peatlandGrowthParams.extract<DynamicObject>());

		// get the data by variable "peatland_turnover_parameters"
		const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

		//create the PeatlandTurnoverParameters, set the value from the variable
		turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
		turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());


		//get the data by variable "peatland_growth_curve"
		const auto& peatlandGrowthCurveData = _landUnitData->getVariable("peatland_growth_curve")->value();

		// create the peatland growth curve, set the component value
		growthCurve = std::make_shared<PeatlandGrowthcurve>();
		growthCurve->setValue(peatlandGrowthCurveData.extract<DynamicObject>());

		//when data is from table, using following
		//growthCurve->setValue(peatlandGrowthCurveData.extract<const std::vector<DynamicObject>>()); 		
    }

	void PeatlandGrowthModule::onTimingStep() {		

		bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();	
		//get the current age
		int age = _peatlandAge->value();
		double woodyStemsBranchesLiveCurrent = _woodyStemsBranchesLive->value();

		//simulate woody layer growth
		double woodyFoliageLiveIncrement = growthCurve->getNetGrowthAtAge(age) * growthParas->FAr();
		double woodyStemsBranchesLiveIncrement = growthCurve->getNetGrowthAtAge(age) * (1 - growthParas->FAr());
		double woodyRootsLive = (woodyStemsBranchesLiveIncrement + woodyStemsBranchesLiveCurrent) * growthParas->a() + growthParas->b();

		//simulate sedge layer growth
		double sedgeFoliageLive = growthParas->aNPPs();
		double sedgeRootsLive = growthParas->aNPPs() * (1 / growthParas->AgBgS());

		//simulate moss layer growth
		double sphagnumMossLive = age < growthParas->Rsp() ? 0 : growthParas->GCsp() * growthParas->NPPsp();
		double featherMossLive = age < growthParas->Rfm() ? 0 : growthParas->GCfm() * growthParas->NPPfm();	

		auto plGrowth = _landUnitData->createStockOperation();

		plGrowth->addTransfer(_atmosphere, _woodyFoliageLive, woodyFoliageLiveIncrement)
			->addTransfer(_atmosphere, _woodyStemsBranchesLive, woodyStemsBranchesLiveIncrement)
			->addTransfer(_atmosphere, _woodyRootsLive, woodyRootsLive)
			->addTransfer(_atmosphere, _sedgeFoliageLive, sedgeFoliageLive)
			->addTransfer(_atmosphere, _sedgeRootsLive, sedgeRootsLive)
			->addTransfer(_atmosphere, _sphagnumMossLive, sphagnumMossLive)
			->addTransfer(_atmosphere, _featherMossLive, featherMossLive);

		_landUnitData->submitOperation(plGrowth); 		
    }

}}} // namespace moja::modules::cbm
