/**
 * @file
 * Parameters for moss related computing
 * **********************/
#include "moja/modules/cbm/mossgrowthmodule.h"
#include "moja/modules/cbm/helper.h"

#include <moja/flint/ioperation.h>
#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>



namespace moja {
	namespace modules {
		namespace cbm {
			
			/**
			 * Configuration function
			 * 
			 * @param config const DynamicObject&
			 * @return void
			 * *********************/
			void MossGrowthModule::configure(const DynamicObject& config) { }

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 * 
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 * **************************/
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

			/**
			 * If variable "enable_moss" exists in _landUnitData and it has a value, 
			 * invoke Helper.runMoss() with arguments as value of variables "growth_curve_id", "moss_leading_species" and "leading_species" in _landUnitData \n
			 * Assign MossGrowthModule.runMoss to true if variable "peatland_class" in _landUnitData is empty, variable "growth_curve_id" in _landUnitData
			 * is not empty, and Helper.runMoss() returns true
			 * 
			 * @return void
			 * **************************/
			void MossGrowthModule::doTimingInit() {
				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value()) {

					auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
					bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

					auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
					auto speciesName = _landUnitData->getVariable("leading_species")->value();

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					runMoss = peatlandId < 0
						&& isGrowthCurveDefined
						&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
				}
			};

			/**
			 * If the value of MossGrowthModule._regenDelay is greater than 0, return \n
			 * If MossGrowthModule.runMoss is true, get the total stand volume at MossGrowthModule._age as : 
			 * invoke StandGrowthCurve.getStandTotalVolumeAtAge() with argument MossGrowthModule._age, on the result of StandGrowthCurveFactory.getStandGrowthCurve() 
			 * on MossGrowthModule._gcFactory with argument as the value of variable "growth_curve_id" in _landUnitData \n
			 * Invoke MossGrowthModule.doMossGrowth() with arguments age, total stand volume at MossGrowthModule._age \n
			 * When moss module is spinning up, i.e MossGrowthModule.spinupMossOnly is true, increment the value of spinupMossOnly._age by 1 and update it
			 * 
			 * @return void
			 * ***************************/
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

			/**
			 * Invoke createStockOperation() on _landUnitData \n
			 * 
			 * Assign variable canopyOpenness result of MossGrowthModule.F1() with arguments MossGrowthModule.a, MossGrowthModule.b, parameter standMerchVolume, \n
			 * groundCoverFeatherMoss result of MossGrowthModule.F2() with arguments MossGrowthModule.c, MossGrowthModule.d, parameter mossAge and variable canopyOpenness, \n
			 * groundCoverSphagnumMoss result of MossGrowthModule.F3() with arguments MossGrowthModule.e, MossGrowthModule.f, parameter mossAge and variable canopyOpenness, \n
			 * nppFeatherMoss result of MossGrowthModule.F4() with arguments MossGrowthModule.g, MossGrowthModule.h, and variable canopyOpenness, \n
			 * nppSphagnumMoss result of MossGrowthModule.F5() with arguments MossGrowthModule.i, MossGrowthModule.j, MossGrowthModule.l and variable canopyOpenness, \n
			 * 
			 * Add transfers between source MossGrowthModule._atmosphere to sink MossGrowthModule._featherMossLive with transfer value nppFeatherMoss * groundCoverFeatherMoss / 100.0, \n
			 * source MossGrowthModule._atmosphere to sink MossGrowthModule._sphagnumMossLive with transfer value nppSphagnumMoss * groundCoverSphagnumMoss / 100.0 
			 * 
			 * Invoke submitOperation() on _landUnitData to submit the transfers
			 * 
			 * @param mossAge int
			 * @param standMerchVolume double
			 * @return void
			 * ******************************/
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
			/**
			 * Return Canopy openNess
			 * 
			 * Canopy openNess, O(t) as a function of merchant volume given a value 60.0 if parameter volume = 0, 
			 * else O(t) = 10 ^ (((a) * (log(volume)) + b)
			 * 
			 * @param a double
			 * @param b double
			 * @param volume double
			 * @return double
			 * *************************/
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
			/**
			 * Return Feather moss ground cover
			 * 
			 * Feather moss ground cover, given a value 0 if parameter age < 0, 
			 * a value 100 if parameter openNess > 70.0, else GCFm(t) = c * openNess + d
			 * 
			 * @param c double
			 * @param d double
			 * @param age int
			 * @param openNess double
			 * @return double
			 * ***************************/
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
			/**
			 * Return Sphagnum ground cover
			 * 
			 * Feather moss ground cover, given a value 0 if parameter age < 0, 
			 * a value 100 if parameter openNess > 70.0, else GCSp(t) = e * openNess  + f
			 * 
			 * @param e double
			 * @param f double
			 * @param age int
			 * @param openNess double
			 * @return double
			 * ***************************/
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
			/**
			 * Return Feather moss NPP
			 * 
			 * Feather moss NPP, given a value 0 if parameter age < 0, 
			 * a value 100 if parameter openNess > 70.0, else GCSp(t) = e * openNess  + f
			 * 
			 * @param e double
			 * @param f double
			 * @param openNess double
			 * @return double
			 * ***************************/
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
			/**
			 * Return Sphagnum NPP
			 * 
			 * Sphagnum NPP, given as NPPSp = i * (openNess ^ 2) + j * openNess + l
			 * 
			 * @param i double
			 * @param j double
			 * @param l double
			 * @param openNess double
			 * @return double
			 * ***************************/
			double MossGrowthModule::F5(double i, double j, double l, double openNess) {
				double value = i * pow(openNess, 2.0) + j * openNess + l;
				return value;
	}	
}}}
