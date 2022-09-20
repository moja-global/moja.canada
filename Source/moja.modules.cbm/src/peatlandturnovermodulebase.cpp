#include "moja/modules/cbm/peatlandturnovermodulebase.h"
#include "moja/modules/cbm/printpools.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include "moja/modules/cbm/timeseries.h"
#include <moja/logging.h>

std::mutex myMutex;

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 *
			 * @param config const DynamicObject&
			 * @return void
			 * *******************************/
			void PeatlandTurnoverModuleBase::configure(const DynamicObject& config) { }

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 *
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 * **************************/
			void PeatlandTurnoverModuleBase::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandTurnoverModuleBase::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &PeatlandTurnoverModuleBase::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &PeatlandTurnoverModuleBase::onTimingStep, *this);
			}

			/**
			 * Set PeatlandTurnoverModuleBase.woodyFoliageLive, PeatlandTurnoverModuleBase.woodyStemsBranchesLive, PeatlandTurnoverModuleBase.woodyRootsLive, PeatlandTurnoverModuleBase.sedgeFoliageLive,
			 * PeatlandTurnoverModuleBase.sedgeRootsLive, PeatlandTurnoverModuleBase.featherMossLive,
			 * PeatlandTurnoverModuleBase.sphagnumMossLive to the values of PeatlandTurnoverModuleBase._woodyFoliageLive, PeatlandTurnoverModuleBase._woodyStemsBranchesLive,
			 * PeatlandTurnoverModuleBase._woodyRootsLive, PeatlandTurnoverModuleBase._sedgeFoliageLive, PeatlandTurnoverModuleBase._sedgeRootsLive,
			 * PeatlandTurnoverModuleBase._featherMossLive and PeatlandTurnoverModuleBase._sphagnumMossLive
			 *
			 * @return void
			 * *************************/
			void PeatlandTurnoverModuleBase::updatePeatlandLivePoolValue() {
				woodyFoliageLive = _woodyFoliageLive->value();
				woodyStemsBranchesLive = _woodyStemsBranchesLive->value();
				woodyRootsLive = _woodyRootsLive->value();
				sedgeFoliageLive = _sedgeFoliageLive->value();
				sedgeRootsLive = _sedgeRootsLive->value();
				featherMossLive = _featherMossLive->value();
				sphagnumMossLive = _sphagnumMossLive->value();
			}

			/**
			 * Live to Dead pool turnover transfers
			 *
			 * Invoke createStockOperation() on _landUnitData \n
			 * since this is a turnover module that transfers carbon from a living carbon pool to a dead carbon pool,
			 * add transfers between the PeatlandTurnoverModuleBase._atmosphere and PeatlandTurnoverModuleBase._woodyFoliageDead, PeatlandTurnoverModuleBase._woodyFineDead pools,
			 * PeatlandTurnoverModuleBase._woodyRootsLive to PeatlandTurnoverModuleBase._woodyRootsDead pool, PeatlandTurnoverModuleBase._sedgeFoliageLive to PeatlandTurnoverModuleBase._sedgeFoliageDead pool,
			 * PeatlandTurnoverModuleBase._sedgeRootsLive to PeatlandTurnoverModuleBase._sedgeRootsDead pool, PeatlandTurnoverModuleBase._featherMossLive to  PeatlandTurnoverModuleBase._feathermossDead pool and
			 * PeatlandTurnoverModuleBase._sphagnumMossLive to  PeatlandTurnoverModuleBase._acrotelm_o pool \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers, applyOperations() to apply the transfers
			 *
			 * @return void
			 * *******************************/
			void PeatlandTurnoverModuleBase::doLivePoolTurnover() {
				//live to dead pool turnover transfers
				//for live woody layer, woodyRootsLive does transfer and can be deducted from source.
				auto peatlandTurnover = _landUnitData->createStockOperation();

				//Special implementation - no moss turnover in the first few years (by Rsp and Rfm, current 5).
				int shrubAge = _landUnitData->getVariable("peatland_shrub_age")->value();
				double sphagnumMossLiveTurnover = (shrubAge - 1.0) <= growthParas->Rsp() ? 0.0 : growthParas->GCsp() * growthParas->NPPsp();
				double featherMossLiveTurnover = (shrubAge - 1.0) <= growthParas->Rfm() ? 0.0 : growthParas->GCfm() * growthParas->NPPfm();

				//Septeber 20, 2022, rollacked code to transfer from live pool to dead pool to keep carbon balance
				//the first two, source is atmosphere, it is particularly modeled, no problem.				
				peatlandTurnover
					->addTransfer(_woodyFoliageLive, _woodyFoliageDead, woodyFoliageLive * (turnoverParas->Pfe() * turnoverParas->Pel() + turnoverParas->Pfn() * turnoverParas->Pnl()))
					->addTransfer(_woodyStemsBranchesLive, _woodyFineDead, woodyStemsBranchesLive * growthParas->Magls())
					->addTransfer(_woodyRootsLive, _woodyRootsDead, woodyRootsLive * turnoverParas->Mbgls())
					->addTransfer(_sedgeFoliageLive, _sedgeFoliageDead, sedgeFoliageLive * turnoverParas->Mags())
					->addTransfer(_sedgeRootsLive, _sedgeRootsDead, sedgeRootsLive * turnoverParas->Mbgs())
					->addTransfer(_featherMossLive, _feathermossDead, featherMossLiveTurnover)
					->addTransfer(_sphagnumMossLive, _acrotelm_o, sphagnumMossLiveTurnover);

				_landUnitData->submitOperation(peatlandTurnover);
				_landUnitData->applyOperations();
			}

			/**
			 * Return the carbon transfer amount
			 *
			 * Computer pow(fabs(previousAwtd), b) and pow(fabs(currentAwtd), b) where fabs refers to the absolute value floating point value
			 * and pow refers to the power function. \n
			 * Return value of 10.0 * fabs(a * (pow(fabs(previousAwtd), b) - pow(fabs(currentAwtd), b)))
			 *
			 * @param previousAwtd double
			 * @param currentAwtd double
			 * @param a double
			 * @param b double
			 * @return double
			 * **************************/
			double PeatlandTurnoverModuleBase::computeCarbonTransfers(double previousAwtd, double currentAwtd, double a, double b) {
				double transferAmount = 0.0;

				//at this moment, the water table depth value should be <=0, make it >=0
				/*previousAwtd = previousAwtd < 0.0 ? (-1 * previousAwtd) : 0.0;
				currentAwtd = currentAwtd < 0.0 ? (-1 * currentAwtd) : 0.0;*/

				double val1 = pow(fabs(previousAwtd), b);
				double val2 = pow(fabs(currentAwtd), b);

				transferAmount = 10.0 * fabs(a * (val1 - val2));

				return transferAmount;
			}

			/**
			 * Compute the water table depth
			 *
			 * Return -0.045 * parameter dc + value of peatlandID (converted to a string) in PeatlandTurnoverModuleBase.baseWTDParameters
			 *
			 * @param dc double
			 * @param peatlandID int
			 * @return double
			 * ****************************/
			double PeatlandTurnoverModuleBase::computeWaterTableDepth(double dc, int peatlandID) {
				double retVal = 0.0;

				std::string peatlandIDStr = std::to_string(peatlandID);
				double wtdBaseValue = baseWTDParameters[peatlandIDStr];
				retVal = -0.045 * dc + wtdBaseValue;

				return retVal;
			}
		}
	}
} // namespace moja::modules::cbm
