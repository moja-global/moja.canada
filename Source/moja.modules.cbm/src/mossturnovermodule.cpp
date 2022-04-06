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

			MossTurnoverModule::MossTurnoverModule() {}

			void MossTurnoverModule::configure(const DynamicObject& config) { }

			void MossTurnoverModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossTurnoverModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossTurnoverModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &MossTurnoverModule::onTimingStep, *this);
			}

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


			void MossTurnoverModule::doTimingInit() {
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
			};

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
			void MossTurnoverModule::doLiveMossTurnover() {
				auto MossTurnoverModule = _landUnitData->createStockOperation();

				double featherMossTurnoverModuleAmount = _featherMossLive->value() * fmlTurnoverRate;
				double sphagnumMossTurnoverModuleAmount = _sphagnumMossLive->value() * smlTurnoverRate;

				MossTurnoverModule->addTransfer(_featherMossLive, _featherMossFast, featherMossTurnoverModuleAmount);
				MossTurnoverModule->addTransfer(_sphagnumMossLive, _sphagnumMossFast, sphagnumMossTurnoverModuleAmount);

				_landUnitData->submitOperation(MossTurnoverModule);
	}
}}}
