/**
* @file
* The CBMDisturbanceListener module determines which pixels are disturbed in what
* years, but CBMDisturbanceEventModule does the actual work of applying the pool
* transfers defined in the disturbance matrix as well as resetting the pixel’s age to 0 if the
* event causes the total live biomass to drop to < 0.001 (effectively 0, using a small
* threshold).
********/

#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/peatlands.h"
#include "moja/modules/cbm/peatlandgrowthcurve.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			* Subscribe to the signals localDomainInit, Disturbance event, and TimingStep
			*
			* @param notificationcenter NotificationCenter&
			* @return void
			* ************************/
			void CBMDisturbanceEventModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMDisturbanceEventModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &CBMDisturbanceEventModule::onTimingStep, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMDisturbanceEventModule::onDisturbanceEvent, *this);
			}

			/**
			*
			* Initialise pools CBMDisturbanceEventModule._softwoodMerch, CBMDisturbanceEventModule._softwoodFoliage,CBMDisturbanceEventModule._softwoodOther, \n
			* CBMDisturbanceEventModule._softwoodCoarseRoots, CBMDisturbanceEventModule._softwoodFineRoots,
			* CBMDisturbanceEventModule._hardwoodMerch, CBMDisturbanceEventModule._hardwoodFoliage, CBMDisturbanceEventModule._hardwoodOther, CBMDisturbanceEventModule._hardwoodCoarseRoots, CBMDisturbanceEventModule._hardwoodFineRoots.
			*  and variable "age" from _landUnitData \n
			* If _landUnitData has variable "enable_peatland" and is not null, \n
			* initialise pools CBMDisturbanceEventModule._woodyFoliageLive, CBMDisturbanceEventModule._woodyStemsBranchesLive, CBMDisturbanceEventModule._woodyRootsLive, \n
			* CBMDisturbanceEventModule._softwoodStem, CBMDisturbanceEventModule._hardwoodStem and variables
			* CBMDisturbanceEventModule._shrubAge, CBMDisturbanceEventModule._smalltreeAge from _landUnitData
			*
			* @return void
			* ************************/
			void CBMDisturbanceEventModule::doLocalDomainInit() {
				_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
				_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
				_softwoodOther = _landUnitData->getPool("SoftwoodOther");
				_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
				_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

				_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
				_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
				_hardwoodOther = _landUnitData->getPool("HardwoodOther");
				_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
				_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

				_age = _landUnitData->getVariable("age");

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
					_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
					_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");

					_softwoodStem = _landUnitData->getPool("SoftwoodStem");
					_hardwoodStem = _landUnitData->getPool("HardwoodStem");

					_shrubAge = _landUnitData->getVariable("peatland_shrub_age");
					_mossAge = _landUnitData->getVariable("peatland_moss_age");
					_smalltreeAge = _landUnitData->getVariable("peatland_smalltree_age");
				}
			}

			/**
			* Get the disturbances and disturbance type codes from parameter n, \n
			* Invoke createProportionalOperation() on _landUnitData, \n
			* for each disturbance, add a transfer between the source and destination pools \n
			* Invoke submitOperation() and applyOperations() on _landUnitData \n
			* If the total biomass is < 0.001, set CBMDisturbanceEventModule._age to 0, \n
			* if the variable "enable_peatland" is present in _landUnitData and is not null, if the total woody biomass is < 0.001,
			* CBMDisturbanceEventModule._shrubAge is set to 0 \n
			* If the value of peatlandId in variable "peatland_class" of _landUnitData is either
			* Peatlands::TREED_PEATLAND_BOG, Peatlands::TREED_PEATLAND_POORFEN, Peatlands::TREED_PEATLAND_RICHFEN or Peatlands::TREED_PEATLAND_SWAMP, \n
			* indicating a treed peatland, and totalSmallTreeBiomass is < 0.001 set CBMDisturbanceEventModule._smalltreeAge to 0
			*
			* @param n DynamicVar
			* @return void
			* ************************/
			void CBMDisturbanceEventModule::doDisturbanceEvent(DynamicVar n) {
				auto& data = n.extract<const DynamicObject>();

				// Get the disturbance type for either historical or last disturbance event.
				std::string disturbanceType = data["disturbance"];
				int disturbanceCode = data["disturbance_type_code"];

				DynamicVar metadata = DynamicObject({
					{ "disturbance", disturbanceType },
					{ "disturbance_type_code", disturbanceCode }
					});

				auto disturbanceEvent = _landUnitData->createProportionalOperation(metadata);
				auto transferVec = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();
				for (const auto& transfer : *transferVec) {
					auto srcPool = transfer.sourcePool();
					auto dstPool = transfer.destPool();
					if (srcPool != dstPool) {
						disturbanceEvent->addTransfer(srcPool, dstPool, transfer.proportion());
					}
				}

				_landUnitData->submitOperation(disturbanceEvent);
				_landUnitData->applyOperations();

				double totalBiomass =
					_hardwoodCoarseRoots->value() + _hardwoodFineRoots->value() +
					_hardwoodFoliage->value() + _hardwoodMerch->value() + _hardwoodOther->value() +
					_softwoodCoarseRoots->value() + _softwoodFineRoots->value() +
					_softwoodFoliage->value() + _softwoodMerch->value() + _softwoodOther->value();

				if (totalBiomass < 0.001) {
					_age->set_value(0);
				}

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {
					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					// call followings only when pixel is of a valid peatland				
					if (peatlandId > 0) {
						auto totalWoodyBiomass =
							_woodyFoliageLive->value() +
							_woodyStemsBranchesLive->value() +
							_woodyRootsLive->value();

						if (totalWoodyBiomass < 0.001) {
							//reset woody layer shrub age to ZERO when almostnothing biomass left
							_shrubAge->set_value(0);
						}
						else {
							//for some disturbance event, which removes portion of the woody layer
							//check the remaining current woody stem branch carbon
							auto woodyStemCarbon = _woodyStemsBranchesLive->value();
							auto resetAge = PeatlandGrowthcurve::findTheMinumResetAge(peatlandId, woodyStemCarbon);

							//MOJA_LOG_INFO << (boost::format("PeatlandID: %d \tWoodyStemBranchCarbon: %-08d \tMatchedGrowthCurveAge:%d") % peatlandId % woodyStem % resetAge).str();

							//reset woody layer shrub age to some close matched age
							_shrubAge->set_value(resetAge);
						}

						if (peatlandId == (int)Peatlands::TREED_PEATLAND_BOG ||
							peatlandId == (int)Peatlands::TREED_PEATLAND_POORFEN ||
							peatlandId == (int)Peatlands::TREED_PEATLAND_RICHFEN ||
							peatlandId == (int)Peatlands::TREED_PEATLAND_SWAMP) {

							// reset small tree age only when current stand is treed peatland						
							auto totalSmallTreeBiomass =
								_hardwoodCoarseRoots->value() + _hardwoodFineRoots->value() +
								_hardwoodFoliage->value() + _hardwoodStem->value() + _hardwoodOther->value() +
								_softwoodCoarseRoots->value() + _softwoodFineRoots->value() +
								_softwoodFoliage->value() + _softwoodStem->value() + _softwoodOther->value();

							if (totalSmallTreeBiomass < 0.001) {
								_smalltreeAge->set_value(0);
							}
						}
					}
				}
			}
		}
	}
} // namespace moja::modules::cbm
