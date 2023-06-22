#include "moja/modules/cbm/peatlandgrowthmodule.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/logging.h>

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 *
			 * @param config DynamicObject&
			 * @return void
			 * *****************/
			void PeatlandGrowthModule::configure(const DynamicObject& config) { }

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
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
				_mossAge = _landUnitData->getVariable("peatland_moss_age");
				_regenDelay = _landUnitData->getVariable("regen_delay");
				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");

				_midSeaonFoliageTurnover = _landUnitData->getVariable("woody_foliage_turnover");
				_midSeaonStemBranchTurnover = _landUnitData->getVariable("woody_stembranch_turnover");
			}

			/**
			*
			* If the value of variable "peatland_class" in _landUnitData is > 0, set PeatlandGrowthModule._runPeatland as true. \n
			* Assign PeatlandGrowthModule.growthParas, value of variable "peatland_growth_parameters", \n
			* PeatlandGrowthModule.turnoverParas  "peatland_turnover_parameters", \n
			* PeatlandGrowthModule.growthCurve value of variable "peatland_growth_curve" from _landUnitData
			*
			* @return void
			* *******************************/
			void PeatlandGrowthModule::doTimingInit() {
				_runPeatland = false;

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					if (_peatlandId > 0) {
						_runPeatland = true;

						//reset the mid-season growth
						_midSeaonFoliageTurnover->reset_value();
						_midSeaonStemBranchTurnover->reset_value();

						updateParameters();
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

				// check peatland at current step
				// peatland of this Pixel may be changed due to disturbance and transition
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				int peatlandIdAtCurrentStep = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (peatlandIdAtCurrentStep != _peatlandId) {
					_peatlandId = peatlandIdAtCurrentStep;
					updateParameters();
				}

				int regenDelay = _regenDelay->value();
				if (regenDelay > 0) {
					return;
				}

				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) { return; }

				//get the live pool as they are at the end of last step
				updateLivePool();

				//get the current age
				int shrubAge = _shrubAge->value();
				int mossAge = _mossAge->value();

				doMidseasonGrowth(shrubAge);
				doNormalGrowth(shrubAge, mossAge);

				_shrubAge->set_value(shrubAge + 1);
				_mossAge->set_value(mossAge + 1);
			}

			/**
			*/
			void PeatlandGrowthModule::updateParameters() {
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

			/**
			 * Update to get the latest pool value to be used as stocks at the end of previous step
			*/
			void PeatlandGrowthModule::updateLivePool() {
				woodyFoliageLive = _woodyFoliageLive->value();
				woodyStemsBranchesLive = _woodyStemsBranchesLive->value();
				woodyRootsLive = _woodyRootsLive->value();
				sedgeFoliageLive = _sedgeFoliageLive->value();
				sedgeRootsLive = _sedgeRootsLive->value();
				featherMossLive = _featherMossLive->value();
				sphagnumMossLive = _sphagnumMossLive->value();
			}

			/**
			* Special growth to record the carboon intakes to correct the net growth
			*/
			void PeatlandGrowthModule::doMidseasonGrowth(int shrubAge) {
				auto plGrowth = _landUnitData->createStockOperation();
				double woodyFoliageLiveIncrement = growthCurve->getNetGrowthAtAge(shrubAge) * growthParas->FAr();
				double woodyStemsBranchesLiveIncrement = growthCurve->getNetGrowthAtAge(shrubAge) * (1 - growthParas->FAr());

				double midSeasongFoliage = woodyFoliageLive + 0.5 * woodyFoliageLiveIncrement;
				double midSeasonStemBranch = woodyStemsBranchesLive + 0.5 * woodyStemsBranchesLiveIncrement;

				double midSeasonFoliageTurnover = midSeasongFoliage * (turnoverParas->Pfe() * turnoverParas->Pel() + turnoverParas->Pfn() * turnoverParas->Pnl());
				double midSeasonStemBranchTurnover = midSeasonStemBranch * growthParas->Magls();

				/*MOJA_LOG_INFO << shrubAge << ", " << woodyFoliageLiveIncrement << ", " << midSeasongFoliage << ", " << midSeasonFoliageTurnover << ", "
					<< woodyFoliageLive << ", " << woodyStemsBranchesLiveIncrement << ", " << midSeasonStemBranch << ", " << midSeasonStemBranchTurnover << ", " << woodyStemsBranchesLive;*/

					//record the mid season growth for turnover 
				_midSeaonFoliageTurnover->set_value(midSeasonFoliageTurnover);
				_midSeaonStemBranchTurnover->set_value(midSeasonStemBranchTurnover);

				plGrowth->addTransfer(_atmosphere, _woodyFoliageLive, midSeasonFoliageTurnover)
					->addTransfer(_atmosphere, _woodyStemsBranchesLive, midSeasonStemBranchTurnover);

				_landUnitData->submitOperation(plGrowth);
				_landUnitData->applyOperations();
			}

			/**
			* Normal woody layer growth
			*/
			void PeatlandGrowthModule::doNormalGrowth(int shrubAge, int mossAge) {
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
				double sphagnumMossLive = mossAge < growthParas->Rsp() ? 0 : growthParas->GCsp() * growthParas->NPPsp();
				double featherMossLive = mossAge < growthParas->Rfm() ? 0 : growthParas->GCfm() * growthParas->NPPfm();

				auto plGrowth = _landUnitData->createStockOperation();

				plGrowth->addTransfer(_atmosphere, _woodyFoliageLive, woodyFoliageLiveIncrement)
					->addTransfer(_atmosphere, _woodyStemsBranchesLive, woodyStemsBranchesLiveIncrement)
					->addTransfer(_atmosphere, _woodyRootsLive, woodyRootsLive)
					->addTransfer(_atmosphere, _sedgeFoliageLive, sedgeFoliageLive)
					->addTransfer(_atmosphere, _sedgeRootsLive, sedgeRootsLive)
					->addTransfer(_atmosphere, _sphagnumMossLive, sphagnumMossLive)
					->addTransfer(_atmosphere, _featherMossLive, featherMossLive);

				_landUnitData->submitOperation(plGrowth);
				_landUnitData->applyOperations();
			}
		}
	}
} // namespace moja::modules::cbm
