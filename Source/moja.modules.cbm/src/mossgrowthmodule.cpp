#include "moja/modules/cbm/mossgrowthmodule.h"
#include "moja/modules/cbm/helper.h"

#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>



namespace moja {
	namespace modules {
		namespace cbm {

			void MossGrowthModule::configure(const DynamicObject& config) { }

			void MossGrowthModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossGrowthModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossGrowthModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &MossGrowthModule::onTimingStep, *this);
			}

			void MossGrowthModule::doLocalDomainInit() {
				_atmosphere = _landUnitData->getPool("Atmosphere");
				_featherMossLive = _landUnitData->getPool("FeatherMossLive");
				_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
				_mossParameters = _landUnitData->getVariable("moss_parameters");

				const auto& MossGrowthModuleParameters = _mossParameters->value().extract<DynamicObject>();

				a = MossGrowthModuleParameters["a"];
				b = MossGrowthModuleParameters["b"];
				c = MossGrowthModuleParameters["c"];
				d = MossGrowthModuleParameters["d"];
				e = MossGrowthModuleParameters["e"];
				f = MossGrowthModuleParameters["f"];
				g = MossGrowthModuleParameters["g"];
				h = MossGrowthModuleParameters["h"];
				i = MossGrowthModuleParameters["i"];
				j = MossGrowthModuleParameters["j"];
				l = MossGrowthModuleParameters["l"];

				_regenDelay = _landUnitData->getVariable("regen_delay");
				_age = _landUnitData->getVariable("age");
			};

			void MossGrowthModule::doTimingInit() {
				auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
				bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

				auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
				auto speciesName = _landUnitData->getVariable("leading_species")->value();

				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				runMoss = peatlandId < 0
					&& isGrowthCurveDefined
					&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
			};

			void MossGrowthModule::doTimingStep() {
				int regenDelay = _regenDelay->value();
				if (regenDelay > 0) {
					return;
				}

				if (runMoss) {
					currentStandGCId = _landUnitData->getVariable("growth_curve_id")->value();
					if (currentStandGCId < 0) return;

					int age = _age->value();
					double gcMerchantVolume = _gcFactory->getStandGrowthCurve(currentStandGCId)->getStandTotalVolumeAtAge(age);

					doMossGrowth(age, gcMerchantVolume);

					bool spinupMossOnly = _landUnitData->getVariable("spinup_moss_only")->value();
					if (spinupMossOnly) {
						//when moss module is spinning up, update the stand age
						_age->set_value(++age);
					}
				}
			};


			void MossGrowthModule::doMossGrowth(int mossAge, double standMerchVolume) {
				auto mossGrowth = _landUnitData->createStockOperation();

				double canopyOpenness = F1(a, b, standMerchVolume);
				double groundCoverFeatherMoss = F2(c, d, mossAge, canopyOpenness);
				double groundCoverSphagnumMoss = F3(e, f, mossAge, canopyOpenness);

				double nppFeatherMoss = F4(g, h, canopyOpenness);
				double nppSphagnumMoss = F5(i, j, l, canopyOpenness);

				//get the growth increment
				double featherMossLiveCIncrement = nppFeatherMoss * groundCoverFeatherMoss / 100.0;
				double sphagnumLiveMossCIncrement = nppSphagnumMoss * groundCoverSphagnumMoss / 100.0;

				mossGrowth->addTransfer(_atmosphere, _featherMossLive, featherMossLiveCIncrement);
				mossGrowth->addTransfer(_atmosphere, _sphagnumMossLive, sphagnumLiveMossCIncrement);

				_landUnitData->submitOperation(mossGrowth);
			}


			// Canopy openness, 10 ^ (((a)*(Log(V(t))) + b)
			double MossGrowthModule::F1(double a, double b, double volume) {
				double value = 0.0;

				if (volume == 0) {
					value = 60.0;
				}
				else {
					value = pow(10.0, (a * log10(volume) + b));
				}

				return value;
			}


			//Feather moss ground cover, GCFm(t) = c*O(t) + d
			double MossGrowthModule::F2(double c, double d, int age, double openNess) {
				double gcfm = 0;

				if (age < 10) {
					gcfm = 0;
				}
				else if (openNess > 70.0) {
					gcfm = 100;
				}
				else {
					gcfm = c * openNess + d;
				}

				return gcfm;
			}

			//Sphagnum ground cover, GCSp(t) = e*O(t) + f
			double MossGrowthModule::F3(double e, double f, int age, double openNess) {
				double gcsp = 0;

				if (age < 10) {
					gcsp = 0;
				}
				else if (openNess > 70.0) {
					gcsp = 100;
				}
				else {
					gcsp = e * openNess + f;
				}

				return gcsp;
			}

			//Feather moss NPP, NPPFm = (g*O(t))^h
			double MossGrowthModule::F4(double g, double h, double openNess) {
				double NPPFm = 0;

				if (openNess < 5.0) {
					NPPFm = 0.6;
				}
				else {
					NPPFm = g * pow(openNess, h);
				}

				return NPPFm;
			}

			//Sphagnum NPP, NPPSp = i*(O(t)^2) + j*O(t) + l
			double MossGrowthModule::F5(double i, double j, double l, double openNess) {
				double value = i * pow(openNess, 2.0) + j * openNess + l;
				return value;
	}	
}}}
