/**
 * @file
 * This module simulates growth and turnover using empirical yield curves which give 
 * the volume of live biomass in m^3/ha at regular age intervals. The yield curve 
 * volumes are converted to carbon using Boudewyn et al’s volume to biomass
 * equations using species-specific coefficients. By default, a smoothing algorithm is applied to the final carbon curve.
 * Growth takes places in two half-year periods with annual turnover (the foliage and 
 * snags that grow and fall off in the same year) in between.
 * Overmature decline, when a stand has reached its maximum volume and then 
 * begins to decrease as it gets older, is also modeled. A stand is considered to be in 
 * decline when the total softwood and hardwood merch, foliage, and other increments
 * are less than a threshold of -0.0001
 * *****************/
#include "moja/modules/cbm/yieldtablegrowthmodule.h"
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

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 * 
			 * Assign YieldTableGrowthModule._smootherEnabled, YieldTableGrowthModule._debuggingEnabled, YieldTableGrowthModule._debuggingOutputPath values of "smoother_enabled", 
			 * "debugging_enabled", "debugging_output_path" in parameter config. \n
			 * Invoke VolumeToBiomassConverter.setSmoothing() with the value of YieldTableGrowthModule._smootherEnabled. \n
			 * 
			 * @param config DynamicObject&
			 * @return void
			 */
			void YieldTableGrowthModule::configure(const DynamicObject& config) {
				if (config.contains("smoother_enabled")) {
					_smootherEnabled = config["smoother_enabled"];
					_volumeToBioGrowth->setSmoothing(_smootherEnabled);
				}

				if (config.contains("debugging_enabled")) {
					_debuggingEnabled = config["debugging_enabled"];
				}

				if (config.contains("debugging_output_path")) {
					_debuggingOutputPath = config["debugging_output_path"].convert<std::string>();
				}
			}

			/**
			 * Subscribe to the signals LocalDomainInit, TimingInit and TimingStep
			 * 
			 * @param notificationCenter NotificationCenter&
			 * @return void
			 */
			void YieldTableGrowthModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &YieldTableGrowthModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &YieldTableGrowthModule::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingStep, &YieldTableGrowthModule::onTimingStep, *this);
			}

			/**
			 * If the value of YieldTableGrowthModule._gcId is not empty, set YieldTableGrowthModule._standGrowthCurveID as value of _gcId, else to -1 \n
			 * If the value of YieldTableGrowthModule._standGrowthCurveID is -1, set YieldTableGrowthModule._isDecaying to false \n
			 * Try to get the stand growth curve and related yield table data from memory, if the result of 
			 * VolumeToBiomassConverter.isBiomassCarbonCurveAvailable() on YieldTableGrowthModule._volumeToBioGrowth with parameter YieldTableGrowthModule._standGrowthCurveID 
			 * and YieldTableGrowthModule._standSPUID true, indicating that the carbon curve is found, 
			 * call the stand growth curve factory to create the stand growth curve, invoke StandGrowthCurveFactory.createStandGrowthCurve() on
			 * YieldTableGrowthModule._gcFactory with parameter YieldTableGrowthModule._standGrowthCurveID, YieldTableGrowthModule._standSPUID and _landUnitData \n
			 * Process and convert yield volume to carbon curves, invoke VolumeToBiomassCarbonGrowth.generateBiomassCarbonCurve() on YieldTableGrowthModule._volumeToBioGrowth 
			 * with argument as the generated stand growth curve \n
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::getYieldCurve() {
				// Get the stand growth curve ID associated to the pixel/svo.
				const auto& standGrowthCurveID = _gcId->value();

				_standGrowthCurveID = standGrowthCurveID.isEmpty() ? -1 : standGrowthCurveID.convert<Int64>();
				if (_standGrowthCurveID == -1) {
					_isDecaying->set_value(false);
				}

				// Try to get the stand growth curve and related yield table data from memory.
				bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(
					_standGrowthCurveID, _standSPUID);

				if (!carbonCurveFound) {
					// Call the stand growth curve factory to create the stand growth curve.
					auto standGrowthCurve = _gcFactory->createStandGrowthCurve(
						_standGrowthCurveID, _standSPUID, *_landUnitData);

					// Process and convert yield volume to carbon curves.
					_volumeToBioGrowth->generateBiomassCarbonCurve(*standGrowthCurve);

					if (_debuggingEnabled) {
						auto curve = _volumeToBioGrowth->getBiomassCarbonCurve(_standGrowthCurveID, _standSPUID);
						std::string outPath = (boost::format("%1%%2%_%3%.csv")
							% _debuggingOutputPath % _standGrowthCurveID % _standSPUID).str();
						curve->writeDebuggingInfo(outPath);
					}
				}
			}

			/**
			 * If _landUnitData has variables "current_growth_multipliers", "output_removal ", assign it to YieldTableGrowthModule._growthMultipliers, 
			 * YieldTableGrowthModule._output_removal \n
			 * Initialise pools YieldTableGrowthModule._softwoodStemSnag, YieldTableGrowthModule._softwoodBranchSnag, YieldTableGrowthModule._softwoodMerch, 
			 * YieldTableGrowthModule._softwoodFoliage, YieldTableGrowthModule._softwoodOther, YieldTableGrowthModule._softwoodCoarseRoots, YieldTableGrowthModule._softwoodFineRoots, 
			 * YieldTableGrowthModule._hardwoodStemSnag, YieldTableGrowthModule._hardwoodBranchSnag, 
			 * YieldTableGrowthModule._hardwoodMerch, YieldTableGrowthModule._hardwoodFoliage, YieldTableGrowthModule._hardwoodOther, 
			 * YieldTableGrowthModule._hardwoodCoarseRoots, YieldTableGrowthModule._hardwoodFineRoots, YieldTableGrowthModule._aboveGroundVeryFastSoil
			 * YieldTableGrowthModule._aboveGroundFastSoil,  YieldTableGrowthModule._belowGroundVeryFastSoil, YieldTableGrowthModule._belowGroundFastSoil, YieldTableGrowthModule._mediumSoil, YieldTableGrowthModule._atmosphere
			 * values of "SoftwoodStemSnag", "SoftwoodBranchSnag", "SoftwoodMerch", "SoftwoodFoilage", "SoftwoodOther",
			 * "SoftwoodCoarseRoots", "SoftwoodFineRoots", "HardwoodStemSnag", "HardwoodBranchSnag", "HardwoodMerch", "HardwoodFoilage", "HardwoodOther",
			 * "HardwoodCoarseRoots", "HardwoodFineRoots", "AboveGroundVeryFastSoil", "AboveGroundFastSoil", "BelowGroundVeryFastSoil", "BelowGroundFastSoil"
			 * "MediumSoil", "Atmosphere" in _landUnitData. \n
			 * If the value of variable "enable_peatland" exists in _landUnitData and it is true, initialise pools 
			 * _woodyFineDead , _woodyCoarseDead, _woodyFoliageDead, _woodyRootsDead values of "WoodyFineDead", "WoodyCoarseDead", "WoodyFoliageDead", "WoodyRootsDead"
			 * in _landUnitData. \n
			 * Set values of variables "age", "growth_curve_id", "spatial_unit_id", "turnover_rates", "regen_delay", "spinup_moss_only", "is_forest", "is_decaying" 
			 * in _landUnitData to YieldTableGrowthModule._age, YieldTableGrowthModule._gcId, YieldTableGrowthModule._spuId, 
			 * YieldTableGrowthModule._turnoverRates, YieldTableGrowthModule._regenDelay, YieldTableGrowthModule._spinupMossOnly,
			 * YieldTableGrowthModule._isForest, YieldTableGrowthModule._isDecaying. \n
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::doLocalDomainInit() {
				_growthMultipliersEnabled = _landUnitData->hasVariable("current_growth_multipliers");
				if (_growthMultipliersEnabled) {
					_growthMultipliers = _landUnitData->getVariable("current_growth_multipliers");
				}

				_softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
				_softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
				_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
				_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
				_softwoodOther = _landUnitData->getPool("SoftwoodOther");
				_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
				_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

				_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
				_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
				_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
				_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
				_hardwoodOther = _landUnitData->getPool("HardwoodOther");
				_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
				_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

				_aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
				_aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
				_belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
				_belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");

				_mediumSoil = _landUnitData->getPool("MediumSoil");
				_atmosphere = _landUnitData->getPool("Atmosphere");

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value().extract<bool>()) {
					_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
					_woodyCoarseDead = _landUnitData->getPool("WoodyCoarseDead");
					_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
					_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");
				}

				_age = _landUnitData->getVariable("age");
				_gcId = _landUnitData->getVariable("growth_curve_id");
				_spuId = _landUnitData->getVariable("spatial_unit_id");
				_turnoverRates = _landUnitData->getVariable("turnover_rates");
				_regenDelay = _landUnitData->getVariable("regen_delay");
				_spinupMossOnly = _landUnitData->getVariable("spinup_moss_only");
				_isForest = _landUnitData->getVariable("is_forest");
				_isDecaying = _landUnitData->getVariable("is_decaying");


				if (_landUnitData->hasVariable("output_removal")) {
					_output_removal = _landUnitData->getVariable("output_removal");
				}
				else {
					_output_removal = nullptr;
				}
			}

			/**
			 * Determine if the YieldTableGrowthModule should run
			 * 
			 * Return true if YieldTableGrowthModule._isForest is true and YieldTableGrowthModule._standGrowthCurveID is not -1, else return false.
			 * 
			 * @return bool
			 */
			bool YieldTableGrowthModule::shouldRun() const {
				bool isForest = _isForest->value();
				bool hasGrowthCurve = _standGrowthCurveID != -1;

				return isForest && hasGrowthCurve;
			}

			/**
			 * Assign YieldTableGrowthModule._standSPUID value of YieldTableGrowthModule._spuId. Invoke YieldTableGrowthModule.initPeatland()
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::doTimingInit() {
				_standSPUID = _spuId->value();

				initPeatland();
			}

			/**
			 * For each pixel set the YieldTableGrowthModule._skipPeatland, YieldTableGrowthModule._runForForestedPeatland to false. \n
			 * If _landUnitData has the variable "enable_peatland" and the value of the variable is true, if the value of variable 
			 * "peatland_class" in _landUnitData is either Peatlands::FOREST_PEATLAND_BOG, Peatlands::FOREST_PEATLAND_POORFEN, 
			 * Peatlands::FOREST_PEATLAND_RICHFEN or Peatlands::FOREST_PEATLAND_SWAMP, set _runForForestedPeatland to true. \n
			 * Skip growth and turnover when running peatland on non-forest peatland stand
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::initPeatland() {
				//for each pixel, always initially reset followings to false
				_skipForPeatland = false;
				_runForForestedPeatland = false;

				if (_landUnitData->hasVariable("enable_peatland") &&
					_landUnitData->getVariable("enable_peatland")->value()) {

					auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
					auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

					_runForForestedPeatland = (
						peatlandId == (int)Peatlands::FOREST_PEATLAND_BOG ||
						peatlandId == (int)Peatlands::FOREST_PEATLAND_POORFEN ||
						peatlandId == (int)Peatlands::FOREST_PEATLAND_RICHFEN ||
						peatlandId == (int)Peatlands::FOREST_PEATLAND_SWAMP);

					//skip growth and turnover when running peatlant on non-forest peatland stand
					_skipForPeatland = peatlandId > 0 && !_runForForestedPeatland;
				}
			}

			/**
			 * If YieldTableGrowthModule._regenDelay > 0, set its value to the decrement of the current value of YieldTableGrowthModule._regenDelay and return \n
			 * If YieldTableGrowthModule._skipForPeatland is true, return \n
			 * If YieldTableGrowthModule.spinupMossOnly us true, the moss module is spinning up, and there is nothing to grow, turnover and decay, return \n
			 * Invoke YieldTableGrowthModule.getYieldCurve(), invoke YieldTableGrowthModule.updateBiomassPools() to get the current biomass pool values.
			 * Set YieldTableGrowthModule.softwoodStemSnag, YieldTableGrowthModule.softwoodBranchSnag, YieldTableGrowthModule.hardwoodStemSnag, 
			 * YieldTableGrowthModule.hardwoodStemSnag to the current values of YieldTableGrowthModule._softwoodStemSnag, 
			 * YieldTableGrowthModule._softwoodBranchSnag, YieldTableGrowthModule._hardwoodStemSnag, YieldTableGrowthModule._hardwoodBranchSnag
			 * 
			 * If _landUnitData has the variable "delay" and the value of variable "run_delay" is true, 
			 * the last rotation is done, and the delay is defined, do turnover and following decay, 
			 * invoke YieldTableGrowthModule.getTurnoverRates(), YieldTableGrowthModule.updateBiomassPools(), 
			 * YieldTableGrowthModule.doMidSeasonGrowth(), YieldTableGrowthModule.switchTurnover() \n
			 * As there should be no growth in the delay period, return
			 * 
			 * If _landUnitData does not have the variable "delay", and YieldTableGrowthModule.shouldRun() is false, return \n
			 * Else, invoke YieldTableGrowthModule.getIncrements() to get and store the biomass carbon growth increments,
			 * YieldTableGrowthModule.getTurnoverRates() to get and store the ecoboundary/genus-specific turnover rates,
			 * YieldTableGrowthModule.switchHalfGrowth() to  transfer half of the biomass growth increment to the biomass pool,
			 * YieldTableGrowthModule.updateBiomassPools() to update to record the current biomass pool value plus the half increment of biomass,
			 * YieldTableGrowthModule.doMidSeasonGrowth() to the foliage and snags that grow and are turned over,
			 * YieldTableGrowthModule.switchTurnover() to switch to do biomass and snag turnover for peatland or regular forest land 
			 * YieldTableGrowthModule.switchHalfGrowth() to transfer the remaining half increment to the biomass pool \n
			 * 
			 * Set the value of YieldTableGrowthModule._age to the increment of the current value of YieldTableGrowthModule._age by 1
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::doTimingStep() {
				int regenDelay = _regenDelay->value();
				if (regenDelay > 0) {
					_regenDelay->set_value(--regenDelay);
					return;
				}

				if (_skipForPeatland) {
					return;
				}

				// When moss module is spinning up, nothing to grow, turnover and decay.
				bool spinupMossOnly = _spinupMossOnly->value();
				if (spinupMossOnly) {
					return;
				}

				getYieldCurve();

				// Get current biomass pool values.
				updateBiomassPools();
				softwoodStemSnag = _softwoodStemSnag->value();
				softwoodBranchSnag = _softwoodBranchSnag->value();
				hardwoodStemSnag = _hardwoodStemSnag->value();
				hardwoodBranchSnag = _hardwoodBranchSnag->value();

				if (_landUnitData->hasVariable("delay")) { // Following delay and isLastRotation are for spinup only.
					int delay = _landUnitData->getVariable("delay")->value();
					bool runDelay = _landUnitData->getVariable("run_delay")->value();

					// When the last rotation is done, and the delay is defined, do turnover and following decay.
					if (runDelay && delay > 0) {
						getTurnoverRates();
						updateBiomassPools();
						doMidSeasonGrowth();
						switchTurnover();

						// No growth in delay period.
						return;
					}
				}

				if (!shouldRun()) {
					return;
				}

				getIncrements();		// 1) get and store the biomass carbon growth increments
				getTurnoverRates();		// 2) get and store the ecoboundary/genus-specific turnover rates
				switchHalfGrowth();		// 3) transfer half of the biomass growth increment to the biomass pool
				updateBiomassPools();	// 4) update to record the current biomass pool value plus the half increment of biomass
				doMidSeasonGrowth();	// 5) the foliage and snags that grow and are turned over
				switchTurnover();		// 6) switch to do biomass and snag turnover for peatland or regular forest land 
				switchHalfGrowth();		// 7) transfer the remaining half increment to the biomass pool

				int standAge = _age->value();
				_age->set_value(standAge + 1);
			}

			/**
			 * If the result of VolumeToBiomassCarbonGrowth.getBiomassCarbonIncrements() on YieldTableGrowthModule._volumeToBioGrowth, indicating there are increments
			 * for the particular _standGrowthCurveID and _standSPUID, assign YieldTableGrowthModule.swm, 
			 * YieldTableGrowthModule.swo, YieldTableGrowthModule.swf, YieldTableGrowthModule.swcr, YieldTableGrowthModule.swfr 
			 * the Softwood increments, YieldTableGrowthModule.hwm, YieldTableGrowthModule.hwo, YieldTableGrowthModule.hwf, 
			 * YieldTableGrowthModule.hwcr, YieldTableGrowthModule.hwfr Hardwood increments \n
			 * If YieldTableGrowthModule._growthMultipliersEnabled is not enabled return \n
			 * else, check if "Softwood" is found in _growthMultipliers and multiply YieldTableGrowthModule.swm, YieldTableGrowthModule.swo, YieldTableGrowthModule.swf, 
			 * YieldTableGrowthModule.swcr, YieldTableGrowthModule.swfr by the softwood multiplication factor, 
			 * check if "Hardwood" is found in YieldTableGrowthModule._growthMultipliers and multiply YieldTableGrowthModule.hwm, 
			 * YieldTableGrowthModule.hwo, YieldTableGrowthModule.hwf, YieldTableGrowthModule.hwcr, YieldTableGrowthModule.hwfr by the hardwood 
			 * multiplication factor 
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::getIncrements() {
				auto increments = _volumeToBioGrowth->getBiomassCarbonIncrements(
					_landUnitData.get(), _standGrowthCurveID, _standSPUID);

				if (increments.size() == 0) {
					MOJA_LOG_INFO << "Warning - growth curve with no increments found: " << _standGrowthCurveID;
				}

				swm = increments["SoftwoodMerch"];
				swo = increments["SoftwoodOther"];
				swf = increments["SoftwoodFoliage"];
				swcr = increments["SoftwoodCoarseRoots"];
				swfr = increments["SoftwoodFineRoots"];

				hwm = increments["HardwoodMerch"];
				hwo = increments["HardwoodOther"];
				hwf = increments["HardwoodFoliage"];
				hwcr = increments["HardwoodCoarseRoots"];
				hwfr = increments["HardwoodFineRoots"];

				if (!_growthMultipliersEnabled) {
					return;
				}

				auto growthMultipliers = _growthMultipliers->value();
				if (growthMultipliers.size() == 0) {
					return;
				}

				auto multipliers = growthMultipliers.extract<
					std::unordered_map<std::string, double>>();

				auto newSwMult = multipliers.find("Softwood");
				if (newSwMult != multipliers.end()) {
					double swMultiplier = newSwMult->second;
					swm *= swMultiplier;
					swo *= swMultiplier;
					swf *= swMultiplier;
					swcr *= swMultiplier;
					swfr *= swMultiplier;

					if (_debuggingEnabled) {
						MOJA_LOG_DEBUG << (boost::format("Applied softwood multiplier of %1% in year %2%")
							% swMultiplier % _landUnitData->timing()->curStartDate().year()).str();
					}
				}

				auto newHwMult = multipliers.find("Hardwood");
				if (newHwMult != multipliers.end()) {
					double hwMultiplier = newHwMult->second;
					hwm *= hwMultiplier;
					hwo *= hwMultiplier;
					hwf *= hwMultiplier;
					hwcr *= hwMultiplier;
					hwfr *= hwMultiplier;

					if (_debuggingEnabled) {
						MOJA_LOG_DEBUG << (boost::format("Applied hardwood multiplier of %1% in year %2%")
							% hwMultiplier % _landUnitData->timing()->curStartDate().year()).str();
					}
				}
			}

			/** 
			 * Set the value of the current turnover rate
			 * 
			 * Set YieldTableGrowthModule._currentTurnoverRates to the value of the tuple key (YieldTableGrowthModule._standGrowthCurveID, YieldTableGrowthModule._standSPUID)
			 * in YieldTableGrowthModule._cachedTurnoverRates if it exists \n
			 * Else set YieldTableGrowthModule._currentTurnoverRates to a shared pointer of TurnoverRates, with the value of 
			 * YieldTableGrowthModule._turnoverRates, a nullptr. Set the value of the tuple key (YieldTableGrowthModule._standGrowthCurveID, YieldTableGrowthModule._standSPUID) to 
			 * YieldTableGrowthModule._currentTurnoverRates
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::getTurnoverRates() {
				auto key = std::make_tuple(_standGrowthCurveID, _standSPUID);
				auto turnoverRates = _cachedTurnoverRates.find(key);
				if (turnoverRates != _cachedTurnoverRates.end()) { //it is there
					_currentTurnoverRates = turnoverRates->second;
				}
				else {
					_currentTurnoverRates = std::make_shared<TurnoverRates>(_turnoverRates->value().extract<DynamicObject>());
					_cachedTurnoverRates[key] = _currentTurnoverRates;
				}
			}

			/**
			 * 
			 * 
			 * 
			 * 
			 * 
			 */
			void YieldTableGrowthModule::doHalfGrowth() const {
				static double tolerance = -0.0001;
				auto growth = _landUnitData->createStockOperation();

				bool swOvermature = swm + swo + swf + swcr + swfr < tolerance;
				if (swOvermature && swm < 0) {
					growth->addTransfer(_softwoodMerch, _softwoodStemSnag, -swm / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodMerch, swm / 2);
				}

				if (swOvermature && swo < 0) {
					growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * _currentTurnoverRates->swBranchSnagSplit() / 2);
					growth->addTransfer(_softwoodOther, _aboveGroundFastSoil, -swo * (1 - _currentTurnoverRates->swBranchSnagSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodOther, swo / 2);
				}

				if (swOvermature && swf < 0) {
					growth->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, -swf / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodFoliage, swf / 2);
				}

				if (swOvermature && swcr < 0) {
					growth->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, -swcr * _currentTurnoverRates->swCoarseRootSplit() / 2);
					growth->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, -swcr * (1 - _currentTurnoverRates->swCoarseRootSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr / 2);
				}

				if (swOvermature && swfr < 0) {
					growth->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, -swfr * _currentTurnoverRates->swFineRootSplit() / 2);
					growth->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, -swfr * (1 - _currentTurnoverRates->swFineRootSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodFineRoots, swfr / 2);
				}

				bool hwOvermature = hwm + hwo + hwf + hwcr + hwfr < tolerance;
				if (hwOvermature && hwm < 0) {
					growth->addTransfer(_hardwoodMerch, _hardwoodStemSnag, -hwm / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodMerch, hwm / 2);
				}

				if (hwOvermature && hwo < 0) {
					growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * _currentTurnoverRates->hwBranchSnagSplit() / 2);
					growth->addTransfer(_hardwoodOther, _aboveGroundFastSoil, -hwo * (1 - _currentTurnoverRates->hwBranchSnagSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodOther, hwo / 2);
				}

				if (hwOvermature && hwf < 0) {
					growth->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, -hwf / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf / 2);
				}

				if (hwOvermature && hwcr < 0) {
					growth->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, -hwcr * _currentTurnoverRates->hwCoarseRootSplit() / 2);
					growth->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, -hwcr * (1 - _currentTurnoverRates->hwCoarseRootSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr / 2);
				}

				if (hwOvermature && hwfr < 0) {
					growth->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, -hwfr * _currentTurnoverRates->hwFineRootSplit() / 2);
					growth->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, -hwfr * (1 - _currentTurnoverRates->hwFineRootSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr / 2);
				}

				_landUnitData->submitOperation(growth);
				_landUnitData->applyOperations();
			}

			/**
			 * 
			 * 
			 * 
			 * 
			 */
			void YieldTableGrowthModule::doPeatlandHalfGrowth() const {
				static double tolerance = -0.0001;
				auto growth = _landUnitData->createStockOperation();

				double swOvermature = swm + swo + swf + swcr + swfr < tolerance;
				if (swOvermature && swm < 0) {
					growth->addTransfer(_softwoodMerch, _softwoodStemSnag, -swm / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodMerch, swm / 2);
				}

				if (swOvermature && swo < 0) {
					growth->addTransfer(_softwoodOther, _softwoodBranchSnag, -swo * _currentTurnoverRates->swBranchSnagSplit() / 2);
					growth->addTransfer(_softwoodOther, _woodyFineDead, -swo * (1 - _currentTurnoverRates->swBranchSnagSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodOther, swo / 2);
				}

				if (swOvermature && swf < 0) {
					growth->addTransfer(_softwoodFoliage, _woodyFoliageDead, -swf / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodFoliage, swf / 2);
				}

				if (swOvermature && swcr < 0) {
					growth->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, -swcr / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr / 2);
				}

				if (swOvermature && swfr < 0) {
					growth->addTransfer(_softwoodFineRoots, _woodyRootsDead, -swfr / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _softwoodFineRoots, swfr / 2);
				}

				double hwOvermature = hwm + hwo + hwf + hwcr + hwfr < tolerance;
				if (hwOvermature && hwm < 0) {
					growth->addTransfer(_hardwoodMerch, _hardwoodStemSnag, -hwm / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodMerch, hwm / 2);
				}

				if (hwOvermature && hwo < 0) {
					growth->addTransfer(_hardwoodOther, _hardwoodBranchSnag, -hwo * _currentTurnoverRates->hwBranchSnagSplit() / 2);
					growth->addTransfer(_hardwoodOther, _woodyFineDead, -hwo * (1 - _currentTurnoverRates->hwBranchSnagSplit()) / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodOther, hwo / 2);
				}

				if (hwOvermature && hwf < 0) {
					growth->addTransfer(_hardwoodFoliage, _woodyFoliageDead, -hwf / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodFoliage, hwf / 2);
				}

				if (hwOvermature && hwcr < 0) {
					growth->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, -hwcr / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr / 2);
				}

				if (hwOvermature && hwfr < 0) {
					growth->addTransfer(_hardwoodFineRoots, _woodyRootsDead, -hwfr / 2);
				}
				else {
					growth->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr / 2);
				}

				_landUnitData->submitOperation(growth);
				_landUnitData->applyOperations();
			}

			/**
			 * Update the biomass and snag pool variables with the latest values
			 * 
			 * Set values of YieldTableGrowthModule._softwoodMerch, YieldTableGrowthModule._softwoodOther, 
			 * YieldTableGrowthModule._softwoodFoilage, YieldTableGrowthModule._softwoodCoarseRoots, YieldTableGrowthModule._softwoodFineRoots, 
			 * YieldTableGrowthModule._hardwoodMerch, YieldTableGrowthModule._hardwoodOther, YieldTableGrowthModule._hardwoodFoilage, 
			 * YieldTableGrowthModule._hardwoodCoarseRoots, YieldTableGrowthModule._hardwoodFineRoots to 
			 * YieldTableGrowthModule.standSoftwoodMerch, YieldTableGrowthModule.standSoftwoodOther, YieldTableGrowthModule.standSoftwoodFoliage, 
			 * YieldTableGrowthModule.standSoftwoodCoarseRoots, YieldTableGrowthModule.standSoftwoodFineRoots,
			 * YieldTableGrowthModule.standHardwoodMerch, YieldTableGrowthModule.standHardwoodOther, YieldTableGrowthModule.standHardwoodFoliage, 
			 * YieldTableGrowthModule.standHardwoodCoarseRoots, YieldTableGrowthModule.standHardwoodFineRoots
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::updateBiomassPools() {
				standSoftwoodMerch = _softwoodMerch->value();
				standSoftwoodOther = _softwoodOther->value();
				standSoftwoodFoliage = _softwoodFoliage->value();
				standSWCoarseRootsCarbon = _softwoodCoarseRoots->value();
				standSWFineRootsCarbon = _softwoodFineRoots->value();
				standHardwoodMerch = _hardwoodMerch->value();
				standHardwoodOther = _hardwoodOther->value();
				standHardwoodFoliage = _hardwoodFoliage->value();
				standHWCoarseRootsCarbon = _hardwoodCoarseRoots->value();
				standHWFineRootsCarbon = _hardwoodFineRoots->value();
			}

			/**
			 * If YieldTableGrowthModule._runForForestedPeatland is true, and YieldTableGrowthModule._output_removal is not null, invoke 
			 * YieldTableGrowthModule.printRemovals() to log all the removals from the stand. Invoke YieldTableGrowthModule.doPeatlandTurnover() to calculate the turnover
			 * If YieldTableGrowthModule._runForForestedPeatland is true, invoke YieldTableGrowthModule.doTurnover() 
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::switchTurnover() const {
				if (_runForForestedPeatland) {
					if (_output_removal != nullptr && _output_removal->value()) {
						int standAge = _age->value();
						double standFoliageRemoval = standSoftwoodFoliage * _currentTurnoverRates->swFoliageTurnover() + standHardwoodFoliage * _currentTurnoverRates->hwFoliageTurnover();
						double standStemSnagRemoval = softwoodStemSnag * _currentTurnoverRates->swStemSnagTurnover() + hardwoodStemSnag * _currentTurnoverRates->hwStemSnagTurnover();
						double standBranchSnagRemoval = softwoodBranchSnag * _currentTurnoverRates->swBranchSnagTurnover() + hardwoodBranchSnag * _currentTurnoverRates->hwBranchSnagTurnover();
						double standOtherRemovalToWFD = standSoftwoodOther * (1 - _currentTurnoverRates->swBranchSnagSplit()) * _currentTurnoverRates->swBranchSnagTurnover() + standHardwoodOther * (1 - _currentTurnoverRates->hwBranchSnagSplit()) * _currentTurnoverRates->hwBranchSnagTurnover();
						double standCoarseRootsRemoval = standSWCoarseRootsCarbon * _currentTurnoverRates->swCoarseRootTurnover() + standHWCoarseRootsCarbon * _currentTurnoverRates->hwCoarseRootTurnover();
						double standFineRootsRemoval = standSWFineRootsCarbon * _currentTurnoverRates->swFineRootTurnover() + +standHWFineRootsCarbon * _currentTurnoverRates->hwFineRootTurnover();
						double standOtherRemovalToBranchSnag = standSoftwoodOther * _currentTurnoverRates->swBranchSnagSplit() * _currentTurnoverRates->swBranchSnagTurnover() + standHardwoodOther * _currentTurnoverRates->hwBranchSnagSplit() * _currentTurnoverRates->hwBranchTurnover();
						printRemovals(standAge, standFoliageRemoval, standStemSnagRemoval, standBranchSnagRemoval, standOtherRemovalToWFD, standCoarseRootsRemoval, standFineRootsRemoval, standOtherRemovalToBranchSnag);
					}
					doPeatlandTurnover();
				}
				else {
					doTurnover();
				}
			}

			/**
			 * If YieldTableGrowthModule._runForForestedPeatland is true, invoke the YieldTableGrowthModule.doPeatlandHalfGrowth(), else invoke 
			 * YieldTableGrowthModule.doHalfGrowth()
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::switchHalfGrowth() const {
				if (_runForForestedPeatland) {
					doPeatlandHalfGrowth();
				}
				else {
					doHalfGrowth();
				}
			}

			/**
			 * Perform snag and biomass turnovers as stock operations
			 * 
			 * Invoke createStockOperation() on _landUnitData . Add transfers between softwood and hardwood branch
			 * and snag pools to medium and above ground fast soil pools. Invoke submitStockOperation() on _landUnitData to submit the transfers \n
			 * Invoke createStockOperation() on _landUnitData . Add transfers between softwood and hardwood merchantable, foilage, 
			 * other, coarse and fine roots to softwood and hardwood stem and branch snag pools, above and below ground
			 * fast and slow pools. Invoke submitStockOperation() on _landUnitData to submit the transfers \n
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::doTurnover() const {
				// Snag turnover.
				auto domTurnover = _landUnitData->createStockOperation();
				domTurnover
					->addTransfer(_softwoodStemSnag, _mediumSoil, softwoodStemSnag * _currentTurnoverRates->swStemSnagTurnover())
					->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, softwoodBranchSnag * _currentTurnoverRates->swBranchSnagTurnover())
					->addTransfer(_hardwoodStemSnag, _mediumSoil, hardwoodStemSnag * _currentTurnoverRates->hwStemSnagTurnover())
					->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, hardwoodBranchSnag * _currentTurnoverRates->hwBranchSnagTurnover());
				_landUnitData->submitOperation(domTurnover);

				// Biomass turnover as stock operation.
				auto bioTurnover = _landUnitData->createStockOperation();
				bioTurnover
					->addTransfer(_softwoodMerch, _softwoodStemSnag, standSoftwoodMerch * _currentTurnoverRates->swStemTurnover())
					->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, standSoftwoodFoliage * _currentTurnoverRates->swFoliageTurnover())
					->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _currentTurnoverRates->swBranchSnagSplit() * _currentTurnoverRates->swBranchTurnover())
					->addTransfer(_softwoodOther, _aboveGroundFastSoil, standSoftwoodOther * (1 - _currentTurnoverRates->swBranchSnagSplit()) * _currentTurnoverRates->swBranchTurnover())
					->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _currentTurnoverRates->swCoarseRootSplit() * _currentTurnoverRates->swCoarseRootTurnover())
					->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _currentTurnoverRates->swCoarseRootSplit()) * _currentTurnoverRates->swCoarseRootTurnover())
					->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon * _currentTurnoverRates->swFineRootSplit() * _currentTurnoverRates->swFineRootTurnover())
					->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _currentTurnoverRates->swFineRootSplit()) * _currentTurnoverRates->swFineRootTurnover())

					->addTransfer(_hardwoodMerch, _hardwoodStemSnag, standHardwoodMerch * _currentTurnoverRates->hwStemTurnover())
					->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage * _currentTurnoverRates->hwFoliageTurnover())
					->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _currentTurnoverRates->hwBranchSnagSplit() * _currentTurnoverRates->hwBranchTurnover())
					->addTransfer(_hardwoodOther, _aboveGroundFastSoil, standHardwoodOther * (1 - _currentTurnoverRates->hwBranchSnagSplit()) * _currentTurnoverRates->hwBranchTurnover())
					->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon * _currentTurnoverRates->hwCoarseRootSplit() * _currentTurnoverRates->hwCoarseRootTurnover())
					->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon * (1 - _currentTurnoverRates->hwCoarseRootSplit()) * _currentTurnoverRates->hwCoarseRootTurnover())
					->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon * _currentTurnoverRates->hwFineRootSplit() * _currentTurnoverRates->hwFineRootTurnover())
					->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _currentTurnoverRates->hwFineRootSplit()) * _currentTurnoverRates->hwFineRootTurnover());
				_landUnitData->submitOperation(bioTurnover);
			}

			/**
			 * Perform dead organic matter and biomass turnovers as stock operations
			 * 
			 * If the value of variable "peatland_class" in _landUnitData is not empty, 
			 * invoke createStockOperation() on _landUnitData . Add transfers between softwood and hardwood
			 * snag and branch pools to woody coarse and fine dead pools. Invoke submitStockOperation() on _landUnitData to submit the transfers \n 
			 * Invoke createStockOperation() on _landUnitData . Add transfers between softwood and hardwood merchantable, foilage,
			 * other, coarse and fine roots to woody coarse, fine foilage and dead pools, softwood and harwood snag pools. 
			 * Invoke submitStockOperation() on _landUnitData to submit the transfers and applyOperations() on _landUnitData 
			 * to apply the transfers \n
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::doPeatlandTurnover() const {
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				if (peatlandId > 0) {
					auto domTurnover = _landUnitData->createStockOperation();
					domTurnover
						->addTransfer(_softwoodStemSnag, _woodyCoarseDead, softwoodStemSnag * _currentTurnoverRates->swStemSnagTurnover())
						->addTransfer(_softwoodBranchSnag, _woodyFineDead, softwoodBranchSnag * _currentTurnoverRates->swBranchSnagTurnover())
						->addTransfer(_hardwoodStemSnag, _woodyCoarseDead, hardwoodStemSnag * _currentTurnoverRates->hwStemSnagTurnover())
						->addTransfer(_hardwoodBranchSnag, _woodyFineDead, hardwoodBranchSnag * _currentTurnoverRates->hwBranchSnagTurnover());
					_landUnitData->submitOperation(domTurnover);

					auto bioTurnover = _landUnitData->createStockOperation();
					bioTurnover
						->addTransfer(_softwoodMerch, _softwoodStemSnag, standSoftwoodMerch * _currentTurnoverRates->swStemTurnover())
						->addTransfer(_softwoodFoliage, _woodyFoliageDead, standSoftwoodFoliage * _currentTurnoverRates->swFoliageTurnover())
						->addTransfer(_softwoodOther, _softwoodBranchSnag, standSoftwoodOther * _currentTurnoverRates->swBranchSnagSplit() * _currentTurnoverRates->swBranchTurnover())
						->addTransfer(_softwoodOther, _woodyFineDead, standSoftwoodOther * (1 - _currentTurnoverRates->swBranchSnagSplit()) * _currentTurnoverRates->swBranchTurnover())
						->addTransfer(_softwoodCoarseRoots, _woodyRootsDead, standSWCoarseRootsCarbon * _currentTurnoverRates->swCoarseRootTurnover())
						->addTransfer(_softwoodFineRoots, _woodyRootsDead, standSWFineRootsCarbon * _currentTurnoverRates->swFineRootTurnover())
						->addTransfer(_hardwoodMerch, _hardwoodStemSnag, standHardwoodMerch * _currentTurnoverRates->hwStemTurnover())
						->addTransfer(_hardwoodFoliage, _woodyFoliageDead, standHardwoodFoliage * _currentTurnoverRates->hwFoliageTurnover())
						->addTransfer(_hardwoodOther, _hardwoodBranchSnag, standHardwoodOther * _currentTurnoverRates->hwBranchSnagSplit() * _currentTurnoverRates->hwBranchTurnover())
						->addTransfer(_hardwoodOther, _woodyFineDead, standHardwoodOther * (1 - _currentTurnoverRates->hwBranchSnagSplit()) * _currentTurnoverRates->hwBranchTurnover())
						->addTransfer(_hardwoodCoarseRoots, _woodyRootsDead, standHWCoarseRootsCarbon * _currentTurnoverRates->hwCoarseRootTurnover())
						->addTransfer(_hardwoodFineRoots, _woodyRootsDead, standHWFineRootsCarbon * _currentTurnoverRates->hwFineRootTurnover());
					_landUnitData->submitOperation(bioTurnover);
					_landUnitData->applyOperations();
				}
			}

			/**
			 * Add transfers from the atmospheric pools to softwood and hardwood pools
			 * 
			 * Record carbon transfers that occur from the atmosphere to softwood and hardwoord pools during mid-season
			 * Invoke createStockOperation() on _landUnitData \n
			 * Add transfers from the atmosphere pool to softwood and hardwood merchantable, foilage, coarse root and fine root pools.
			 * Submit the operation to the _landUnitData by invoking submitOperation() 
			 * 
			 * @return void
			 */
			void YieldTableGrowthModule::doMidSeasonGrowth() const {
				auto seasonalGrowth = _landUnitData->createStockOperation();
				seasonalGrowth
					->addTransfer(_atmosphere, _softwoodMerch, standSoftwoodMerch * _currentTurnoverRates->swStemTurnover())
					->addTransfer(_atmosphere, _softwoodOther, standSoftwoodOther * _currentTurnoverRates->swBranchTurnover())
					->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _currentTurnoverRates->swFoliageTurnover())
					->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _currentTurnoverRates->swCoarseRootTurnover())
					->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _currentTurnoverRates->swFineRootTurnover())
					->addTransfer(_atmosphere, _hardwoodMerch, standHardwoodMerch * _currentTurnoverRates->hwStemTurnover())
					->addTransfer(_atmosphere, _hardwoodOther, standHardwoodOther * _currentTurnoverRates->hwBranchTurnover())
					->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _currentTurnoverRates->hwFoliageTurnover())
					->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _currentTurnoverRates->hwCoarseRootTurnover())
					->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _currentTurnoverRates->hwFineRootTurnover());
				_landUnitData->submitOperation(seasonalGrowth);
			}

			/**
			 * Return a shared pointer to an object StandGrowthCurve with parameters standGrowthCurveID and spuID
			 * 
			 * @param standGrowthCurveID Int64
			 * @param spuID Int64
			 * @return shared_ptr<StandGrowthCurve>
			 */
			std::shared_ptr<StandGrowthCurve> YieldTableGrowthModule::createStandGrowthCurve(
				Int64 standGrowthCurveID, Int64 spuID) const {

				auto standGrowthCurve = std::make_shared<StandGrowthCurve>(standGrowthCurveID, spuID);
				return standGrowthCurve;
			}

			/**
			 * Log the parameters standAge, standFoilageRemoval, standStemSnagRemoval, standBranchSnagRemoval,
			 * standOtherRemovalToWFD, standCoarseRootRemoval, standFineRootRemoval and standOtherRemovalToBranchSnag
			 * 
			 * @param standAge int
			 * @param standFoilageRemoval double
			 * @param standStemSnagRemoval double
			 * @param standBranchSnagRemoval double
			 * @param standOtherRemovalToWFD double
			 * @param standCoarseRootRemoval double
			 * @param standFineRootRemoval double
			 * @param standOtherRemovalToBranchSnag double
			 * @return void
			 */
			void YieldTableGrowthModule::printRemovals(int standAge,
				double standFoliageRemoval,
				double standStemSnagRemoval,
				double standBranchSnagRemoval,
				double standOtherRemovalToWFD,
				double standCoarseRootsRemoval,
				double standFineRootsRemoval,
				double standOtherRemovalToBranchSnag) const {
				MOJA_LOG_INFO << standAge << ", " << standFoliageRemoval << ", " << standStemSnagRemoval << ", " << standBranchSnagRemoval << ", "
					<< standOtherRemovalToWFD << ", " << standCoarseRootsRemoval << ", " << standFineRootsRemoval << ", " << standOtherRemovalToBranchSnag;
			}
		}
	}
}