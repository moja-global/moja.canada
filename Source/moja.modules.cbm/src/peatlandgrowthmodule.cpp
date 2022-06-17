#include "moja/modules/cbm/peatlandgrowthmodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
	namespace modules {
		namespace cbm {

			 /**
			 * @brief Configuration function
			 * 
			 * @param config DynamicObject&
			 * @return void
			 * *****************/
			void PeatlandGrowthModule::configure(const DynamicObject& config) { }

			 /**
			 * @brief Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 * 
			 * @param notificationCenter NotificationCenter& 
			 * @return void
			 * *****************/
			void PeatlandGrowthModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandGrowthModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandGrowthModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &PeatlandGrowthModule::onTimingStep, *this);
			}

			 /**
			 * 
			 * Initialise pools PeatlandGrowthModule._atmosphere, PeatlandGrowthModule._woodyFoliageLive, \n
			 * PeatlandGrowthModule._woodyStemsBranchesLive,  PeatlandGrowthModule._woodyRootsLive, PeatlandGrowthModule._sedgeFoliageLive, \n
			 * PeatlandGrowthModule._sedgeRootsLive, PeatlandGrowthModule._sphagnumMossLive, PeatlandGrowthModule._featherMossLive, PeatlandGrowthModule._shrubAge, \n 
			 * PeatlandGrowthModule._regenDelay, PeatlandGrowthModule._spinupMossOnly from _landUnitData.
			 * 
			 * @return void
			 * ******************/
			void PeatlandGrowthModule::doLocalDomainInit() {
				_atmosphere = _landUnitData->getPool("Atmosphere");

				_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
				_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
				_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");
				_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
				_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");
				_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
				_featherMossLive = _landUnitData->getPool("FeatherMossLive");

				_shrubAge = _landUnitData->getVariable("peatland_shrub_age");
				_regenDelay = _landUnitData->getVariable("regen_delay");
				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
			}

			 /**
			 * 
			 * If the value of variable "peatland_class" in _landUnitData is > 0, set PeatlandGrowthModule._runPeatland as true. \n
			 * Assign PeatlandGrowthModule.growthParas, value of variable "peatland_growth_parameters", \n
			 * PeatlandGrowthModule.turnoverParas value of variable "peatland_turnover_parameters", \n
			 * PeatlandGrowthModule.growthCurve value of variable "peatland_growth_curve" from _landUnitData
			 * 
			 * @return void
			 * *******************************/
			void PeatlandGrowthModule::doTimingInit() {
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (_peatlandId > 0) {
					_runPeatland = true;

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
					if (!peatlandGrowthCurveData.isEmpty()) {
						growthCurve->setValue(peatlandGrowthCurveData.extract<const std::vector<DynamicObject>>());
					}
				}
			}

			 /**
			 * 
			 * If PeatlandGrowthModule._runPeatland is true, PeatlandGrowthModule._regenDelay > 0 and PeatlandGrowthModule._spinupMossOnly is false, \n
			 * simulate woody layer growth, sedge layer growth and moss layer growth. \n
			 * Initiate the start of the operation by _landUnitData->createStockOperation() and add transfers between various pools. Finally, submit the operation \n
			 * Increment PeatlandGrowthModule._shrubAge by 1 
			 * 
			 * @return void
			 * ***********************************/
			void PeatlandGrowthModule::doTimingStep() {
				if (!_runPeatland) { return; }

				int regenDelay = _regenDelay->value();
				if (regenDelay > 0) {
					return;
				}

				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) { return; }

				//get the current age
				int shrubAge = _shrubAge->value();
				double woodyStemsBranchesLiveCurrent = _woodyStemsBranchesLive->value();

				//simulate woody layer growth
				double woodyFoliageLiveIncrement = growthCurve->getNetGrowthAtAge(shrubAge) * growthParas->FAr();
				double woodyStemsBranchesLiveIncrement = growthCurve->getNetGrowthAtAge(shrubAge) * (1 - growthParas->FAr());
				double woodyRootsLive = 0;
				if (woodyStemsBranchesLiveIncrement != 0 || woodyStemsBranchesLiveCurrent != 0) {
					woodyRootsLive = (woodyStemsBranchesLiveIncrement + woodyStemsBranchesLiveCurrent) * growthParas->a() + growthParas->b();
				}

				//simulate sedge layer growth
				double sedgeFoliageLive = shrubAge == 0 ? 0 : growthParas->aNPPs();
				double sedgeRootsLive = shrubAge == 0 ? 0 : growthParas->aNPPs() * (1 / growthParas->AgBgS());

				//simulate moss layer growth
				double sphagnumMossLive = shrubAge < growthParas->Rsp() ? 0 : growthParas->GCsp() * growthParas->NPPsp();
				double featherMossLive = shrubAge < growthParas->Rfm() ? 0 : growthParas->GCfm() * growthParas->NPPfm();

				auto plGrowth = _landUnitData->createStockOperation();

				plGrowth->addTransfer(_atmosphere, _woodyFoliageLive, woodyFoliageLiveIncrement)
					->addTransfer(_atmosphere, _woodyStemsBranchesLive, woodyStemsBranchesLiveIncrement)
					->addTransfer(_atmosphere, _woodyRootsLive, woodyRootsLive)
					->addTransfer(_atmosphere, _sedgeFoliageLive, sedgeFoliageLive)
					->addTransfer(_atmosphere, _sedgeRootsLive, sedgeRootsLive)
					->addTransfer(_atmosphere, _sphagnumMossLive, sphagnumMossLive)
					->addTransfer(_atmosphere, _featherMossLive, featherMossLive);

				_landUnitData->submitOperation(plGrowth);
		_shrubAge->set_value(shrubAge + 1);
    }

}}} // namespace moja::modules::cbm
