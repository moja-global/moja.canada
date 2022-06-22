/**
 * @file
 * Parameters for moss related computing
 * 
 * *********************/
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include "moja/modules/cbm/mossturnovermodule.h"
#include "moja/modules/cbm/helper.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Constructor
			 * ********************/
			MossTurnoverModule::MossTurnoverModule() {}

			/**
			 * Configuration function
			 * 
			 * @param config const DynamicObject&
			 * @return void
			 * *********************/
			void MossTurnoverModule::configure(const DynamicObject& config) { }

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 * 
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 * **************************/
			void MossTurnoverModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossTurnoverModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossTurnoverModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &MossTurnoverModule::onTimingStep, *this);
			}

			/**
			 * Initialise MossTurnoverModule._featherMossFast, MossTurnoverModule._sphagnumMossFast, MossTurnoverModule._featherMossSlow, MossTurnoverModule._sphagnumMossSlow, 
			 * value of "FeatherMossFast", SphagnumMossFast", "FeatherMossSlow", "SphagnumMossSlow" in _landUnitData \n
			 * Initialise MossTurnoverModule._mossParameters, MossTurnoverModule._regenDelay as variable "moss_parameters", "regen_delay" in _landUnitData, \n
			 * MossTurnoverModule.fmlTurnoverRate, MossTurnoverModule.smlTurnoverRate values of "fmlTurnoverRate", "smlTurnoverRate"in MossTurnoverModule._mossParameters
			 * 
			 * @return void
			 * *************************/
			void MossTurnoverModule::doLocalDomainInit() {
				_featherMossLive = _landUnitData->getPool("FeatherMossLive");
				_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
				_featherMossFast = _landUnitData->getPool("FeatherMossFast");
				_sphagnumMossFast = _landUnitData->getPool("SphagnumMossFast");

				_mossParameters = _landUnitData->getVariable("moss_parameters");
				const auto& mossGrowthParameters = _mossParameters->value().extract<DynamicObject>();

				fmlTurnoverRate = mossGrowthParameters["fmlTurnoverRate"];
				smlTurnoverRate = mossGrowthParameters["smlTurnoverRate"];

				_regenDelay = _landUnitData->getVariable("regen_delay");
			};

			/**
			 * If variable "enable_moss" exists in _landUnitData and it has a value, 
			 * invoke Helper.runMoss() with arguments as value of variables "growth_curve_id", "moss_leading_species" and "leading_species" in _landUnitData \n
			 * Assign MossTurnoverModule.runMoss to true if variable "peatland_class" in _landUnitData is empty, variable "growth_curve_id" in _landUnitData
			 * is not empty, and Helper.runMoss() returns true
			 * 
			 * @return void
			 * **************************/
			void MossTurnoverModule::doTimingInit() {
				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {

					auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
					bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

					auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
					auto speciesName = _landUnitData->getVariable("leading_species")->value();

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					// no moss module run on peatland 
					runMoss = peatlandId < 0
						&& isGrowthCurveDefined
						&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
				}
			};

			/**
			 * If value of MossTurnoverModule._regenDelay > 0, return \n
			 * If MossTurnoverModule.runMoss is true, invoke MossTurnoverModule.doLiveMossTurnover()
			 * 
			 * @return void
			 * ************************/
			void MossTurnoverModule::doTimingStep() {
				int regenDelay = _regenDelay->value();
				if (regenDelay > 0) {
					return;
				}

				if (runMoss) {
					doLiveMossTurnover();
				}
			};

			//Moss turnover (moss live pool to moss fast pool)
			//FeatherMossLive -> FeatherMossFast
			//SphagnumMossLive -> SphagnumMossFast
			/**
			 * Perform moss turnover between the moss live and fast pools
			 * 
			 * Invoke createStockOperation() on _landUnitData \n
			 * Add a FeatherMossLive to FeatherMossFast transfer between source MossTurnoverModule._featherMossLive and sink MossTurnoverModule._featherMossFast with transfer 
			 * value of MossTurnoverModule._featherMossLive * MossTurnoverModule.fmlTurnoverRate, a SphagnumMossLive to SphagnumMossFast transfer
			 * between source MossTurnoverModule._sphagnumMossLive and sink MossTurnoverModule._sphagnumMossFast with transfer 
			 * value of MossTurnoverModule._sphagnumMossLive * MossTurnoverModule.smlTurnoverRate \n
			 * Invoke submitOperation() on _landUnitData to submit the transfers 
			 * 
			 * @return void
			 * ***************************/
			void MossTurnoverModule::doLiveMossTurnover() {
				auto MossTurnoverModule = _landUnitData->createStockOperation();

				double featherMossTurnoverModuleAmount = _featherMossLive->value() * fmlTurnoverRate;
				double sphagnumMossTurnoverModuleAmount = _sphagnumMossLive->value() * smlTurnoverRate;

				MossTurnoverModule->addTransfer(_featherMossLive, _featherMossFast, featherMossTurnoverModuleAmount);
				MossTurnoverModule->addTransfer(_sphagnumMossLive, _sphagnumMossFast, sphagnumMossTurnoverModuleAmount);

				_landUnitData->submitOperation(MossTurnoverModule);
	}
}}}
