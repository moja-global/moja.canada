/**
* @file
* @brief The brief description goes here.
*
* The detailed description if any, goes here
* ******/
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/peatlands.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			
			/**
			* @brief subscribe to signal
			*
			* This function subscribes the signal localDomainInit, Disturbance event, and TimingStep
	        * using the function onLocalDomainInit,onDisturbanceEvent ,and onTimingStep respectively.
	        * The values are passed and assigned here
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
			* @brief Initiate local domain
			*
			* This function gets the pool value of softwood merch,softwood foliage,softwood other, softwood coarse roots,softwood fine roots,
			* hardwood merch, hardwood foliage, hardwood other, hardwood coarse roots, hardwood fine roots and also gets the variable value of age 
			* from the land unit data variable.
			* if the enable peatland variable exists in the land unit data, it will get the pool value of woody foliage live,
			* woody stems branches live, woody roots live, softwood stem, hardwood stem, peatland shrub age and peatland small tree age from 
			* land unit data variable.
			* 
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
					_smalltreeAge = _landUnitData->getVariable("peatland_smalltree_age");
				}
			}

			/**
			* @brief doDisturbanceEvent
			*
			* Detailed description here
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

					double totalWoodyBiomass =
						_woodyFoliageLive->value() +
						_woodyStemsBranchesLive->value() +
						_woodyRootsLive->value();

					if (totalWoodyBiomass < 0.001) {
						//alway reset woody layer shrub age 
						_shrubAge->set_value(0);
					}

					if (peatlandId == (int)Peatlands::TREED_PEATLAND_BOG ||
						peatlandId == (int)Peatlands::TREED_PEATLAND_POORFEN ||
						peatlandId == (int)Peatlands::TREED_PEATLAND_RICHFEN ||
						peatlandId == (int)Peatlands::TREED_PEATLAND_SWAMP) {
						// reset small tree age only when current stand is treed peatland						
						double totalSmallTreeBiomass =
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
} // namespace moja::modules::cbm
