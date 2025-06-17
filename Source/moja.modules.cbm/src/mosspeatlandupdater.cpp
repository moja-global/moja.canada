/**
 * @file
 * This module to check whether to simulate CaMP or Moss-C
 *
 * When the pixel is hit first time.
 * 1) If there is a valid peatland ID mappled, it will initially simulate CaMP.
 * 2) If CaMP is not qualified to simulate, it will check if Moss_C is conditioned to simulate.
 * 3) If Moss-C is qualified, Moss-C is to simulate with no spinup
 *
 * Then, at each time step
 * Peat_CaMP and Peat_Moss are calcuated, based on the result of those peat pool
 * simulation is swtiched in-between CaMP and Moss-C.
 *
 * When switch from CaMP to Moss-C, runtime peatland is set to -1
 * When switch from Moss-C to CaMP, runtime peatland is set to forest peatland (3)
 * *****************/
#include "moja/modules/cbm/mosspeatlandupdater.h"
#include "moja/modules/cbm/turnoverrates.h"

#include <moja/flint/variable.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/itiming.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>
 //#include <iostream>
#include <boost/filesystem.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 *
			 * Assign following variables configured in module paralmeters:
			 * MossPeatlandUpdater._isDebug,
			 * MossPeatlandUpdater._debuggingEnabled,
			 * MossPeatlandUpdater._debuggingOutputPath.\n
			 *
			 * @param config DynamicObject&
			 * @return void
			 **/
			void MossPeatlandUpdater::configure(const DynamicObject& config) {
				// module configuration setting
				// indicate this instance is used in spinup or foward run
				_inSpinup = config["in_spinup"];
			}

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 *
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 **/
			void MossPeatlandUpdater::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossPeatlandUpdater::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossPeatlandUpdater::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &MossPeatlandUpdater::onTimingStep, *this);
				notificationCenter.subscribe(signals::PreTimingSequence, &MossPeatlandUpdater::onPreTimingSequence, *this);
				notificationCenter.subscribe(signals::PrePostDisturbanceEvent, &MossPeatlandUpdater::onPrePostDisturbanceEvent, *this);
			}

			/**
			 * Initialization when the system is up running
			 * @return void
			 **/
			void MossPeatlandUpdater::doLocalDomainInit() {
				_atmosphere = _landUnitData->getPool("Atmosphere");

				_aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
				_aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
				_aboveGroundSlowSoil = _landUnitData->getPool("AboveGroundSlowSoil");
				_belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
				_belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");
				_belowGroundSlowSoil = _landUnitData->getPool("BelowGroundSlowSoil");

				_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
				_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
				_woodyCoarseDead = _landUnitData->getPool("WoodyCoarseDead");
				_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");
				_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
				_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");
				_featherMossDead = _landUnitData->getPool("FeatherMossDead");

				_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
				_catotelm_a = _landUnitData->getPool("Catotelm_A");
				_acrotelm_a = _landUnitData->getPool("Acrotelm_A");
				_catotelm_o = _landUnitData->getPool("Catotelm_O");

				_featherMossFast = _landUnitData->getPool("FeatherMossFast");
				_sphagnumMossFast = _landUnitData->getPool("SphagnumMossFast");
				_featherMossSlow = _landUnitData->getPool("FeatherMossSlow");
				_sphagnumMossSlow = _landUnitData->getPool("SphagnumMossSlow");

				_mossCaMPIndictor = _landUnitData->getVariable("moss_camp_indictor");

				_runtimePeatlandId = _landUnitData->getVariable("peatland_class");

				_runPeatland = _landUnitData->getVariable("run_peatland");
				_runMoss = _landUnitData->getVariable("run_moss");

				_age = _landUnitData->getVariable("age");
				_shrubAge = _landUnitData->getVariable("peatland_shrub_age");
				_mossAge = _landUnitData->getVariable("peatland_moss_age");
				_smallTreeAge = _landUnitData->getVariable("peatland_smalltree_age");

				_cset = _landUnitData->getVariable("classifier_set");
				_loadMossInitial = _landUnitData->getVariable("load_moss_initials");
			}

			/*
			* Reset _runPeatland and _runMoss.
			*
			* Read the original mapped peatland ID
			* Update the runtime peatland_class ID (classifer)
			*/
			void MossPeatlandUpdater::doPreTimingSequence() {
				// first time on a pixel, reset run_peatland and run_moss variables
				// for a pixel, read original peatland profile(ID) map 	
				if (_inSpinup) {
					_runPeatland->set_value(false);
					_runMoss->set_value(false);

					auto& const mappedPeatlandId = _landUnitData->getVariable("peatland")->value();
					auto peatlandId = mappedPeatlandId.isEmpty() ? -1 : mappedPeatlandId.convert<int>();

					// set the runtime variable "peatland_class" for this pixel
					_runtimePeatlandId->set_value(peatlandId);

					// in CaMP and Moss simulation
					// peatland_class is a must have classifier
					// update peatland_class classifier value
					// initial_classifier_set has -1 for "peatland_class"
					auto cset = _cset->value();
					cset["peatland_class"] = peatlandId;
					_cset->set_value(cset);

					// Reset the ages to ZERO before the spinup procedure
					_age->reset_value();
					_shrubAge->reset_value();
					_mossAge->reset_value();
					_smallTreeAge->reset_value();
				}
			}

			/**
			 * Try to get possible assoociated stand growth curve
			 * Check if peatland simulation(CaMP) is applicable on this pixel
			 * Check if moss simulation(Moss-C) is applicable on this pixel
			 *
			 * Update _runPeatland and _runMoss
			 *
			 * @return void
			 */
			void MossPeatlandUpdater::doTimingInit() {
				// for a landunit pixel, only evaluate it in spinup
				if (_inSpinup) {
					// for a pixel, always try to find the associated growth curve				
					// associated stand growth curve ID maybe none				
					const auto& gcId = _landUnitData->getVariable("growth_curve_id")->value();
					auto standForestGrowthCurveID = gcId.isEmpty() ? -1 : gcId.convert<Int64>();

					// check whether to simulate peatland or moss
					// peatland and moss are exclusively simulated at any timestep
					int peatlandId = _runtimePeatlandId->value().convert<int>();

					bool runPeatland = isPeatlandApplicable(peatlandId, standForestGrowthCurveID);
					bool runMoss = isMossApplicable(runPeatland, standForestGrowthCurveID);

					_runPeatland->set_value(runPeatland);
					_runMoss->set_value(runMoss);
				}
			}

			/**
			 * In foward run only
			 * Main function is to evaluate whether to simulate CaMP or Moss-C
			 * switch between moss-c and CaMP based on peat_CaMP and peat_Moss
			 *
			 * @return void
			 **/
			void MossPeatlandUpdater::doTimingStep() {
				// evaluate in forward run only
				if (!_inSpinup) {
					int ageNow = _age->value();
					auto peatlandEnabled = _landUnitData->getVariable("enable_peatland")->value();
					auto mossEnabled = _landUnitData->getVariable("enable_moss")->value();

					// calcuate the current CAMP and MOSS-C condition pools
					double peat_CaMP = getPeatCaMP();
					double peat_Moss = getPeatMoss();
					//MOJA_LOG_INFO << peat_CaMP << ", " << peat_Moss;

					double switchPoint = _mossCaMPIndictor->value().convert<double>();

					// switch betwee CaMP and MoSS-C based on above condition
					// when moss is enabled and it is now run_peatland
					if (_runPeatland->value() && mossEnabled) {
						if (peat_CaMP > 0 && peat_CaMP < switchPoint) {
							// transition to run MOSS-C	
							transferCaMP2Moss();
						}
					}

					// when peatland is enabled and it is now run_moss
					else if (_runMoss->value() && peatlandEnabled) {
						if (peat_Moss > 0 && peat_Moss >= switchPoint) {
							// transition to run CaMP	
							transferMoss2CaMP();
						}
					}
				}
			}

			/*
			* In spinup phase, when moss-c is simulated
			* Load the moss slow pool value if configured.
			*/
			void MossPeatlandUpdater::doPrePostDisturbanceEvent() {
				bool runMoss = _runMoss->value().convert<bool>();
				bool loadInitial = _loadMossInitial->value().convert<bool>();
				if (runMoss && loadInitial) {
					resetMossInitialValue();
					loadMossInitialValue();
				}
			}

			/**
			 * Determine whether peatland has to be simulated
			 *
			 * For a spatial pixel, original mapped peatland ID is initially evaluated.
			 * Peatland variable in variable_moss_peatland.json is the original peatland profile ID.
			 * passed argument peatlandId is always the original mapped peatland profile ID
			 *
			 * Variable peatland_class is a runtime peatland ID. It is a runtime variable as it may be changed.
			 *
			 * It is set to -1 to skip the peatland simulation.
			 * It can be set to other peatland ID value which is different from the original one if transition applied.
			 *
			 * If the origianl peatland is valid. In most cases, the peatland should be simulated.
			 *
			 * However, if inventory data is trusted over peatland map, it will check other conditions further.
			 *
			 * @param peatlandId int
			 * @param standForestGrowthCurveID Int64
			 * @return bool
			 */
			bool MossPeatlandUpdater::isPeatlandApplicable(int peatlandId, Int64 standForestGrowthCurveId) {
				bool toSimulatePeatland = false;

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value().convert<bool>()) {
					// runtime peatland ID may be changed to -1 during the evaluation here
					// runtime peatland ID may also be changed after the disturbance (transition to new peatland type)	
					int runtimePeatlandId = peatlandId;

					int maxPeatlandId = _landUnitData->getVariable("max_peatland_Id")->value().convert<int>();

					if (runtimePeatlandId < 0 || runtimePeatlandId > maxPeatlandId) {
						// not a valid peatlandId, skip peatland simulation
						return false;
					}

					// with real spatial data, for one spatial location, 
					// pixel is mapped as open bog in peatland map, 
					// however this pixel is mapped as foretry in inventory dataset
					// we need to pick one layer over another layer				
					// if we pick inventory_win which means that inventory data is more trustable
					// we trust inventory data over peatland map				
					auto inventoryOverPeatland = _landUnitData->getVariable("inventory_over_peatland")->value();
					bool inventory_win = inventoryOverPeatland.convert<bool>();

					if (inventory_win) {
						// check if the peatland layer is of open peatland (1, 4, 7)			
						auto isBogPeatland =
							(runtimePeatlandId == (int)Peatlands::OPEN_PEATLAND_BOG) ||
							(runtimePeatlandId == (int)Peatlands::OPEN_PEATLAND_POORFEN) ||
							(runtimePeatlandId == (int)Peatlands::OPEN_PEATLAND_RICHFEN);

						if (!isBogPeatland) {
							// the pixel have a valid growth curve associated, and is treed or forest peatland
							if (standForestGrowthCurveId > 0) {
								// it should be one of the forest/treed peatland, check the leading speciese
								std::string speciesName = _landUnitData->getVariable("leading_species")->value();
								boost::algorithm::to_lower(speciesName);

								// list of leading specices for forest and treed peatland
								auto forestPeatlandLeadingSpecies =
									_landUnitData->getVariable("forest_peatland_leading_species")->value();
								bool hasForestPeatlandLeadingSpecies = false;

								// only leading species matches the pre-defined forest peatland leading speciese
								for (std::string item : forestPeatlandLeadingSpecies) {
									boost::algorithm::to_lower(item);
									hasForestPeatlandLeadingSpecies = boost::contains(speciesName, item);
									if (hasForestPeatlandLeadingSpecies) {
										// one matched leading species is found, skip remaining find/check
										// keep original peatlandId no change, simulate the forest or treed peatland
										break;
									}
								}

								if (!hasForestPeatlandLeadingSpecies) {
									// the leading specise is not one of the forest peatland leading species
									// skip peatland simulation by reset the runtime peatlan ID to -1
									runtimePeatlandId = -1;
								}
							}
						}
						else if (standForestGrowthCurveId > 0) {
							// it is of open peatland, with a valid growth curve
							// bypass CaMP and run regular GCBM
							runtimePeatlandId = -1;
						}

						// in case of not simulating peatland
						// update the classifier_set with updated peatland_class
						if (runtimePeatlandId == -1) {
							auto cset = _cset->value();
							cset["peatland_class"] = runtimePeatlandId;
							_cset->set_value(cset);
						}
					}

					// update the runtime "peatland_class" variable
					_runtimePeatlandId->set_value(runtimePeatlandId);

					// CaMP is simulated only if runtimePeatlandId > 0
					toSimulatePeatland = runtimePeatlandId > 0;
				}
				return toSimulatePeatland;
			}

			/**
			 * Determine whether the moss needs to be simulated
			 *
			 * Currently, moss is simulated when peatland is not simulated
			 * Stand leading species is checked to match the pre-defined species to run the moss
			 *
			 * @param runPeatland bool
			 * @param standForestGrowthCurveID Int64
			 * @return bool
			 */
			bool MossPeatlandUpdater::isMossApplicable(bool runPeatland, Int64 standForestGrowthCurveID) {
				bool toSimulateMoss = false;

				if (_landUnitData->hasVariable("enable_moss") &&
					_landUnitData->getVariable("enable_moss")->value().convert<bool>()) {

					bool isGrowthCurveDefined = standForestGrowthCurveID != -1;

					// moss growth is based on leading species' growth.
					if (isGrowthCurveDefined) {
						auto mossLeadingSpecies =
							_landUnitData->getVariable("moss_leading_species")->value();
						std::string speciesName =
							_landUnitData->getVariable("leading_species")->value().convert<std::string>();

						// Can also get species from a spatial layer:
						// std::string speciesName2 = _landUnitData->getVariable("species")->value();
						bool mossLeadingSpeciesMatched = false;
						for (std::string species : mossLeadingSpecies) {
							boost::algorithm::to_lower(species);
							boost::algorithm::to_lower(speciesName);
							mossLeadingSpeciesMatched = boost::contains(speciesName, species);
							if (mossLeadingSpeciesMatched) {
								break;
							}
						}
						toSimulateMoss = !runPeatland && mossLeadingSpeciesMatched;
					}
				}
				return toSimulateMoss;
			}

			/*
			* Reset moss fast and slow pool
			*/
			void MossPeatlandUpdater::resetMossInitialValue() {
				// reset current moss slow pool
				auto mossSlowwReset = _landUnitData->createProportionalOperation();
				mossSlowwReset
					->addTransfer(_featherMossFast, _atmosphere, 1.0)
					->addTransfer(_sphagnumMossFast, _atmosphere, 1.0)
					->addTransfer(_featherMossSlow, _atmosphere, 1.0)
					->addTransfer(_sphagnumMossSlow, _atmosphere, 1.0);
				_landUnitData->submitOperation(mossSlowwReset);
				_landUnitData->applyOperations();
			}

			/*
			* Load the moss slow pool values
			*
			* Moss slow pool will be loaded with pre-defined initial value
			*/
			void MossPeatlandUpdater::loadMossInitialValue() {
				const auto& mossInitials = _landUnitData->getVariable("moss_initial_stocks")->value();
				const DynamicObject& data = mossInitials.extract<DynamicObject>();

				// load pre-defined moss slow pool value
				auto mossInitial = _landUnitData->createStockOperation();
				mossInitial->addTransfer(_atmosphere, _featherMossFast, data["FeatherMossFast"])
					->addTransfer(_atmosphere, _sphagnumMossFast, data["SphagnumMossFast"])
					->addTransfer(_atmosphere, _featherMossSlow, data["FeatherMossSlow"])
					->addTransfer(_atmosphere, _sphagnumMossSlow, data["SphagnumMossSlow"]);
				_landUnitData->submitOperation(mossInitial);
				_landUnitData->applyOperations();
			}

			/*
			* Calcuate peat carbon in CaMP module
			* @return double
			*/
			double MossPeatlandUpdater::getPeatCaMP() {
				double peatCaMP =
					_featherMossDead->value() +
					_acrotelm_o->value() +
					_catotelm_a->value() +
					_acrotelm_a->value() +
					_catotelm_o->value();

				return peatCaMP;
			}

			/*
			* Calculate slow moss carbon in Moss-C module
			* @return double
			*/
			double MossPeatlandUpdater::getPeatMoss() {
				double peatMoss =
					_featherMossFast->value() +
					_sphagnumMossFast->value() +
					_featherMossSlow->value() +
					_sphagnumMossSlow->value();

				return peatMoss;
			}

			/*
			* Transfer Moss-C slow pool carbon to CaMP peat pool
			* Set runtime peatland as forest peatland (ID=3)
			*/
			void MossPeatlandUpdater::transferMoss2CaMP() {
				auto mossInitial = _landUnitData->createProportionalOperation();
				mossInitial->addTransfer(_featherMossFast, _featherMossDead, 1)
					->addTransfer(_sphagnumMossFast, _acrotelm_o, 1)
					->addTransfer(_featherMossSlow, _catotelm_a, 1)
					->addTransfer(_sphagnumMossSlow, _catotelm_a, 1);
				_landUnitData->submitOperation(mossInitial);
				_landUnitData->applyOperations();

				// transit to forest peatland		
				// set runtime peatlan ID as forest peatland			
				_runtimePeatlandId->set_value((int)Peatlands::FOREST_PEATLAND_BOG);

				// update the classifier_set -> forest peatland
				auto cset = _cset->value();
				cset["peatland_class"] = (int)Peatlands::FOREST_PEATLAND_BOG;
				_cset->set_value(cset);

				_runMoss->set_value(false);
				_runPeatland->set_value(true);
			}

			/*
			* Transfer CaMP slow and peat pool to moss-c slow pool
			*/
			void MossPeatlandUpdater::transferCaMP2Moss() {
				auto camp2moss = _landUnitData->createProportionalOperation();
				camp2moss
					->addTransfer(_woodyFineDead, _belowGroundVeryFastSoil, 1.0)
					->addTransfer(_woodyCoarseDead, _belowGroundFastSoil, 1.0)
					->addTransfer(_woodyFoliageDead, _belowGroundVeryFastSoil, 1.0)
					->addTransfer(_woodyRootsDead, _belowGroundFastSoil, 1.0)
					->addTransfer(_sedgeFoliageDead, _aboveGroundVeryFastSoil, 1.0)
					->addTransfer(_sedgeRootsDead, _belowGroundVeryFastSoil, 1.0)
					->addTransfer(_featherMossDead, _featherMossFast, 1.0)
					->addTransfer(_acrotelm_o, _featherMossFast, 0.5)
					->addTransfer(_acrotelm_o, _sphagnumMossFast, 0.5)
					->addTransfer(_acrotelm_a, _featherMossFast, 0.5)
					->addTransfer(_acrotelm_a, _sphagnumMossFast, 0.5)
					->addTransfer(_catotelm_a, _sphagnumMossSlow, 1.0)
					->addTransfer(_catotelm_o, _featherMossSlow, 1.0);
				_landUnitData->submitOperation(camp2moss);
				_landUnitData->applyOperations();

				// no longer to simulate peatland, reset runtime peatland ID -1
				// it may be already changed after disturbance transition
				// to be checked with input data (transition rule)				
				_runtimePeatlandId->set_value((int)Peatlands::NON_PEATLAND);

				// update the classifier_set -> non_peatland
				auto cset = _cset->value();
				cset["peatland_class"] = (int)Peatlands::NON_PEATLAND;
				_cset->set_value(cset);

				_runPeatland->set_value(false);
				_runMoss->set_value(true);
			}
		}
	}
}