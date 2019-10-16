#include "moja/modules/cbm/esgymmodule.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>
#include "moja/flint/variable.h"
#include <moja/flint/flintexceptions.h>
#include <moja/flint/ilandunitdatawrapper.h>
#include <moja/flint/itiming.h>

#include "moja/logging.h"
#include <moja/signals.h>
#include <moja/notificationcenter.h>

namespace moja {
namespace modules {
namespace cbm {	

	void ESGYMModule::configure(const DynamicObject& config) { }

	void ESGYMModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connectSignal(signals::LocalDomainInit, &ESGYMModule::onLocalDomainInit, *this);
		notificationCenter.connectSignal(signals::TimingInit, &ESGYMModule::onTimingInit, *this);
		notificationCenter.connectSignal(signals::TimingStep, &ESGYMModule::onTimingStep, *this);
	}

	void ESGYMModule::doLocalDomainInit() {
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

		_age = _landUnitData->getVariable("age");
		_turnoverRates = _landUnitData->getVariable("turnover_rates");
		
		_regenDelay = _landUnitData->getVariable("regen_delay");

		_currentLandClass = _landUnitData->getVariable("current_land_class");

		auto rootParams = _landUnitData->getVariable("root_parameters")->value().extract<DynamicObject>();
		SWRootBio = std::make_shared<SoftwoodRootBiomassEquation>(
			rootParams["sw_a"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
		HWRootBio = std::make_shared<HardwoodRootBiomassEquation>(
			rootParams["hw_a"], rootParams["hw_b"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);

		const auto& growthSpeciesEffects = _landUnitData->getVariable("growth_esgym_species_specific_effects")->value()
			.extract<const std::vector<DynamicObject>>();
		for (const auto& row : growthSpeciesEffects) {
			int speciesID = row["species_id"];
			growth_esgym_species_specific_effects[speciesID] = row;
		}
		
		const auto& mortalitySpeciesEffects = _landUnitData->getVariable("mortality_esgym_species_specific_effects")->value()
			.extract<const std::vector<DynamicObject>>();
		for (const auto& row : mortalitySpeciesEffects) {
			int speciesID = row["species_id"];
			mortality_esgym_species_specific_effects[speciesID] = row;
		}

		growth_esgym_fixed_effects = _landUnitData->getVariable("growth_esgym_fixed_effects")->value().extract<DynamicObject>();
		mortality_esgym_fixed_effects = _landUnitData->getVariable("mortality_esgym_fixed_effects")->value().extract<DynamicObject>();
		growth_esgym_environmental_effects = _landUnitData->getVariable("growth_esgym_environmental_effects")->value().extract<DynamicObject>();
		mortality_esgym_environmental_effects = _landUnitData->getVariable("mortality_esgym_environmental_effects")->value().extract<DynamicObject>();
		mean_esgym_environmental_effects = _landUnitData->getVariable("mean_esgym_environmental_effects")->value().extract<DynamicObject>();
		stddev_esgym_environmental_effects = _landUnitData->getVariable("stddev_esgym_environmental_effects")->value().extract<DynamicObject>();
		
		foliageAllocationParameters = _landUnitData->getVariable("FoliageAllocationParameters")->value().extract<DynamicObject>();
		branchAllocationParameters = _landUnitData->getVariable("BranchAllocationParameters")->value().extract<DynamicObject>();
		
		standBiomassModifierParameters = _landUnitData->getVariable("StandBiomassModifierParameters")->value().extract<DynamicObject>();

		environmentalDescriptiveStatistics = _landUnitData->getVariable("EnvironmentalDescriptiveStatistics")->value().extract<DynamicObject>();

		topStumpParameters = _landUnitData->getVariable("top_stump_parameters")->value().extract<DynamicObject>();

		const auto& co2Value = _landUnitData->getVariable("ca")->value();
		if (co2Value.isVector()) {
			const auto co2Table = co2Value.extract<const std::vector<DynamicObject>>();
			for (const auto& row : co2Table) {
				int year = row["t_year"];
				double co2 = row["co2_concentration"];
				co2Concentrations[year] = co2;
			}
		}
		else {
			int year = co2Value["t_year"];
			double co2 = co2Value["co2_concentration"];
			co2Concentrations[year] = co2;
		}
			
	
		_isForest = _landUnitData->getVariable("is_forest");
		_cbm_species_id = _landUnitData->getVariable("CBM_Species_ID");
	}

	void ESGYMModule::doTimingInit() {
		const auto& turnoverRates = _turnoverRates->value().extract<DynamicObject>();
		_softwoodFoliageFallRate = turnoverRates["softwood_foliage_fall_rate"];
		_hardwoodFoliageFallRate = turnoverRates["hardwood_foliage_fall_rate"];
		_stemAnnualTurnOverRate = turnoverRates["stem_annual_turnover_rate"];
		_softwoodBranchTurnOverRate = turnoverRates["softwood_branch_turnover_rate"];
		_hardwoodBranchTurnOverRate = turnoverRates["hardwood_branch_turnover_rate"];
		_otherToBranchSnagSplit = turnoverRates["other_to_branch_snag_split"];
		_stemSnagTurnoverRate = turnoverRates["stem_snag_turnover_rate"];
		_branchSnagTurnoverRate = turnoverRates["branch_snag_turnover_rate"];
		_coarseRootSplit = turnoverRates["coarse_root_split"];
		_coarseRootTurnProp = turnoverRates["coarse_root_turn_prop"];
		_fineRootAGSplit = turnoverRates["fine_root_ag_split"];
		_fineRootTurnProp = turnoverRates["fine_root_turn_prop"];
	}

	float ESGYMModule::ExtractRasterValue(const std::string name) {
		auto value = _landUnitData->getVariable(name)->value();
		return value.isEmpty() ? 0
			: value.type() == typeid(TimeSeries) ? value.extract<TimeSeries>().value()
			: value.convert<double>();
	}

	void ESGYMModule::updateBiomassPools() {
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

	double ESGYMModule::ComputeComponentGrowth(double predictor, double b0, double b1, double b2) 
	{
		return b0 + b1 * predictor + b2 * predictor * predictor;
	}

	double ESGYMModule::StandBiomassModifier(double standBio, double standBio_mu, double standBio_sig, double LamBs) {
		return (standBio - standBio_mu) / standBio_sig * LamBs;
	}

	void ESGYMModule::doTimingStep() {

		int regenDelay = _regenDelay->value();
		if (regenDelay > 0) {
			_regenDelay->set_value(--regenDelay);
			return;
		}
		int species_id = _cbm_species_id->value();
		
		if (!shouldRun() || species_id < 0) {
			return;
		}
		// Get current biomass pool values.
		updateBiomassPools();

		auto growthSpeciesEffectsMatch = growth_esgym_species_specific_effects.find(species_id);
		if (growthSpeciesEffectsMatch == growth_esgym_species_specific_effects.end()) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("growth species specific effect not found")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("esgymmodule"));
		}
		auto growthSpeciesEffects = growthSpeciesEffectsMatch->second;

		auto mortalitySpeciesEffectsMatch = mortality_esgym_species_specific_effects.find(species_id);
		if (mortalitySpeciesEffectsMatch == mortality_esgym_species_specific_effects.end()) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("mortality species specific effect not found")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("esgymmodule"));
		}
		auto mortalitySpeciesEffects = mortalitySpeciesEffectsMatch->second;

		double softwoodProportion = _landUnitData->getVariable("SoftwoodProportion")->value();

		double dwf_a = ExtractRasterValue("dwf_a");
		double rswd_a = ExtractRasterValue("rswd_a");
		double tmean_a = ExtractRasterValue("tmean_a");
		double vpd_a = ExtractRasterValue("vpd_a");
		double eeq_a = ExtractRasterValue("eeq_a");
		double ws_a = ExtractRasterValue("ws_a");
		double ndep = ExtractRasterValue("ndep");
		
		//spatial variables
		double dwf_n = ExtractRasterValue("dwf");
		double eeq_n = ExtractRasterValue("eeq");

		//absolute carbon dioxide concentration
		double ca;
		if (co2Concentrations.size() > 1) 
		{
			//for simulation there is a map of co2 values by year
			int year = _landUnitData->timing()->curStartDate().year();
			auto CO2_Concentration = co2Concentrations.find(year);
			if (CO2_Concentration == co2Concentrations.end()) {
				BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
					<< moja::flint::Details("CA year not found")
					<< moja::flint::LibraryName("moja.modules.cbm")
					<< moja::flint::ModuleName("esgymmodule"));
			}
			ca = CO2_Concentration->second;
		}
		else if (co2Concentrations.size() == 1) 
		{
			//for spinup we have a single co2 value
			ca = co2Concentrations.begin()->second;
		}
		else 
		{
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("CA table empty")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("esgymmodule"));
		}


		double G = GrowthAndMortality(_age->value(), growth_esgym_fixed_effects["b1"],
			growth_esgym_fixed_effects["b2"], growth_esgym_fixed_effects["b3"],
			growth_esgym_fixed_effects["b4"], growth_esgym_fixed_effects["b5"],
			growthSpeciesEffects["b1"], growthSpeciesEffects["b2"],
			eeq_n, environmentalDescriptiveStatistics["eeq_mu"], environmentalDescriptiveStatistics["eeq_sig"],
			dwf_n, environmentalDescriptiveStatistics["dwf_mu"], environmentalDescriptiveStatistics["dwf_sig"]);

		double M = GrowthAndMortality(_age->value(), mortality_esgym_fixed_effects["b1"],
			mortality_esgym_fixed_effects["b2"], mortality_esgym_fixed_effects["b3"],
			mortality_esgym_fixed_effects["b4"], mortality_esgym_fixed_effects["b5"],
			mortalitySpeciesEffects["b1"], mortalitySpeciesEffects["b2"],
			eeq_n, environmentalDescriptiveStatistics["eeq_mu"], environmentalDescriptiveStatistics["eeq_sig"],
			dwf_n, environmentalDescriptiveStatistics["dwf_mu"], environmentalDescriptiveStatistics["dwf_sig"]);

		double E_Growth = EnvironmentalModifier(
			growth_esgym_environmental_effects["dwf_a"], mean_esgym_environmental_effects["dwf_a"], stddev_esgym_environmental_effects["dwf_a"], dwf_a,
			growth_esgym_environmental_effects["rswd_a"], mean_esgym_environmental_effects["rswd_a"], stddev_esgym_environmental_effects["rswd_a"], rswd_a,
			growth_esgym_environmental_effects["tmean_a"], mean_esgym_environmental_effects["tmean_a"], stddev_esgym_environmental_effects["tmean_a"], tmean_a,
			growth_esgym_environmental_effects["vpd_a"], mean_esgym_environmental_effects["vpd_a"], stddev_esgym_environmental_effects["vpd_a"], vpd_a,
			growth_esgym_environmental_effects["eeq_a"], mean_esgym_environmental_effects["eeq_a"], stddev_esgym_environmental_effects["eeq_a"], eeq_a,
			growth_esgym_environmental_effects["ws_a"], mean_esgym_environmental_effects["ws_a"], stddev_esgym_environmental_effects["ws_a"], ws_a,
			growth_esgym_environmental_effects["ndep"], mean_esgym_environmental_effects["ndep"], stddev_esgym_environmental_effects["ndep"], ndep,
			growth_esgym_environmental_effects["ca"], mean_esgym_environmental_effects["ca"], stddev_esgym_environmental_effects["ca"], ca);

		double E_Mortality = EnvironmentalModifier(
			mortality_esgym_environmental_effects["dwf_a"], mean_esgym_environmental_effects["dwf_a"], stddev_esgym_environmental_effects["dwf_a"], dwf_a,
			mortality_esgym_environmental_effects["rswd_a"], mean_esgym_environmental_effects["rswd_a"], stddev_esgym_environmental_effects["rswd_a"], rswd_a,
			mortality_esgym_environmental_effects["tmean_a"], mean_esgym_environmental_effects["tmean_a"], stddev_esgym_environmental_effects["tmean_a"], tmean_a,
			mortality_esgym_environmental_effects["vpd_a"], mean_esgym_environmental_effects["vpd_a"], stddev_esgym_environmental_effects["vpd_a"], vpd_a,
			mortality_esgym_environmental_effects["eeq_a"], mean_esgym_environmental_effects["eeq_a"], stddev_esgym_environmental_effects["eeq_a"], eeq_a,
			mortality_esgym_environmental_effects["ws_a"], mean_esgym_environmental_effects["ws_a"], stddev_esgym_environmental_effects["ws_a"], ws_a,
			mortality_esgym_environmental_effects["ndep"], mean_esgym_environmental_effects["ndep"], stddev_esgym_environmental_effects["ndep"], ndep,
			mortality_esgym_environmental_effects["ca"], mean_esgym_environmental_effects["ca"], stddev_esgym_environmental_effects["ca"], ca);

		double GrowthStandBiomassModifier = 0;
		double MortalityStandBiomassModifier = 0;
		if (standBiomassModifierParameters["enabled"]) {
			double totalStandAGBio = standSoftwoodMerch + standSoftwoodFoliage + standSoftwoodOther
				+ standHardwoodMerch + standHardwoodFoliage + standHardwoodOther;

			GrowthStandBiomassModifier = StandBiomassModifier(totalStandAGBio,
				standBiomassModifierParameters["Bs_mu"], standBiomassModifierParameters["Bs_sig"], standBiomassModifierParameters["Growth_LamBs"]);

			MortalityStandBiomassModifier = StandBiomassModifier(totalStandAGBio,
				standBiomassModifierParameters["Bs_mu"], standBiomassModifierParameters["Bs_sig"], standBiomassModifierParameters["Mortality_LamBs"]);
		}

		//clamp at 0
		double G_modified = std::max(0.0, G + E_Growth + GrowthStandBiomassModifier);
		double M_modified = std::max(0.0, M + E_Mortality + MortalityStandBiomassModifier);
		//the net growth can be negative
		double stemWoodBarkNetGrowth = G_modified - M_modified;
		
		//MOJA_LOG_INFO << (int)_age->value() << "," << G << "," << E_Growth << "," << G_modified << "," << M << "," << E_Mortality << "," << M_modified << "," << stemWoodBarkNetGrowth << "," << dwf_a << "," << rswd_a << "," << tmean_a << "," << vpd_a << "," << eeq_a << "," << ws_a << "," << ndep << "," << dwf_n << "," << eeq_n << "," << ca;
		
		// these are spinup related
		int delay = _landUnitData->getVariable("delay")->value();
		bool runDelay = _landUnitData->getVariable("run_delay")->value();
		if (runDelay && delay > 0) {
			G_modified = 0;//no growth if we are in spinup delay
			M_modified = 0;//not sure about this one though...
		}

		double foliageInc = stemWoodBarkNetGrowth * ComputeComponentGrowth(
			_age->value(), foliageAllocationParameters["b0"], 
			foliageAllocationParameters["b1"], 
			foliageAllocationParameters["b2"]);

		double branchInc = stemWoodBarkNetGrowth * ComputeComponentGrowth(
			_age->value(), branchAllocationParameters["b0"],
			branchAllocationParameters["b1"],
			branchAllocationParameters["b2"]);

		double hwTopsAndStumpsInc =
			topStumpParameters["hardwood_top_prop"] / 100.0 * stemWoodBarkNetGrowth * (1 - softwoodProportion ) +
			topStumpParameters["hardwood_stump_prop"] / 100.0 * stemWoodBarkNetGrowth * (1 - softwoodProportion);

		double swTopsAndStumpsInc =
			topStumpParameters["softwood_top_prop"] / 100.0 * stemWoodBarkNetGrowth * softwoodProportion +
			topStumpParameters["softwood_stump_prop"] / 100.0 * stemWoodBarkNetGrowth * softwoodProportion;

		double netMerchGrowthSW = stemWoodBarkNetGrowth * softwoodProportion - swTopsAndStumpsInc;
		double netOtherGrowthSW = swTopsAndStumpsInc + branchInc * softwoodProportion/* + saplingSW + subMerchAndBarkSW */ ;
		double netFoliageGrowthSW = foliageInc * softwoodProportion;

		double netMerchGrowthHW = stemWoodBarkNetGrowth * (1-softwoodProportion) - hwTopsAndStumpsInc;
		double netOtherGrowthHW = hwTopsAndStumpsInc + branchInc* (1 - softwoodProportion)/* + saplingHW + subMerchAndBarkHW */;
		double netFoliageGrowthHW = foliageInc * (1-softwoodProportion);

		// find the softwood increments (or decrement) based on the esgym growth
		// prediction, the value is set so that a decrement may not set biomass
		// to be less than 0
		double swm_inc = std::max(-standSoftwoodMerch, netMerchGrowthSW);
		double swf_inc = std::max(-standSoftwoodFoliage, netFoliageGrowthSW);
		double swo_inc = std::max(-standSoftwoodOther, netOtherGrowthSW);

		//compute the total softwood ag biomass at the end of this growth period
		double swAgBioC = standSoftwoodMerch + swm_inc +
			standSoftwoodFoliage + swf_inc +
			standSoftwoodOther + swo_inc;

		double hwm_inc = std::max(-standHardwoodMerch, netMerchGrowthHW);
		double hwf_inc = std::max(-standHardwoodFoliage, netFoliageGrowthHW);
		double hwo_inc = std::max(-standHardwoodOther, netOtherGrowthHW);

		//compute the total hardwood ag biomass at the end of this growth period
		double hwAgBioC = standHardwoodMerch + hwm_inc +
			standHardwoodFoliage + hwf_inc +
			standHardwoodOther + hwo_inc;

		double swRootBio = SWRootBio->calculateRootBiomass(swAgBioC);
		double hwRootBio = HWRootBio->calculateRootBiomass(hwAgBioC);
		auto RootProp = SWRootBio->calculateRootProportions(swRootBio + hwRootBio);
		double swCoarseIncrement = SWRootBio->biomassToCarbon(RootProp.coarse*swRootBio) - standSWCoarseRootsCarbon;
		double swFineIncrement = SWRootBio->biomassToCarbon(RootProp.fine*swRootBio) - standSWFineRootsCarbon;

		double hwCoarseIncrement = HWRootBio->biomassToCarbon(RootProp.coarse*hwRootBio) - standHWCoarseRootsCarbon;
		double hwFineIncrement = HWRootBio->biomassToCarbon(RootProp.fine*hwRootBio) - standHWFineRootsCarbon;

		auto growth = _landUnitData->createStockOperation();
		growth
			->addTransfer(_atmosphere, _softwoodMerch, swm_inc)
			->addTransfer(_atmosphere, _softwoodFoliage, swf_inc)
			->addTransfer(_atmosphere, _softwoodOther, swo_inc)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, swCoarseIncrement)
			->addTransfer(_atmosphere, _softwoodFineRoots, swFineIncrement)
			->addTransfer(_atmosphere, _hardwoodMerch, hwm_inc)
			->addTransfer(_atmosphere, _hardwoodFoliage, hwf_inc)
			->addTransfer(_atmosphere, _hardwoodOther, hwo_inc)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwCoarseIncrement)
			->addTransfer(_atmosphere, _hardwoodFineRoots, hwFineIncrement);

		_landUnitData->submitOperation(growth);
		_landUnitData->applyOperations();
		updateBiomassPools();

		softwoodStemSnag = _softwoodStemSnag->value();
		softwoodBranchSnag = _softwoodBranchSnag->value();
		hardwoodStemSnag = _hardwoodStemSnag->value();
		hardwoodBranchSnag = _hardwoodBranchSnag->value();

		doTurnover(M_modified);

		int standAge = _age->value();
		_age->set_value(standAge + 1);

	}

	void ESGYMModule::doTurnover(double M) const {
		// Snag turnover.
		auto domTurnover = _landUnitData->createStockOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag, _mediumSoil, softwoodStemSnag * _stemSnagTurnoverRate)
			->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, softwoodBranchSnag * _branchSnagTurnoverRate)
			->addTransfer(_hardwoodStemSnag, _mediumSoil, hardwoodStemSnag * _stemSnagTurnoverRate)
			->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, hardwoodBranchSnag * _branchSnagTurnoverRate);
		_landUnitData->submitOperation(domTurnover);

		// Biomass turnover as stock operation.
		auto bioTurnover = _landUnitData->createStockOperation();
		bioTurnover
			->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, standSoftwoodFoliage * _softwoodFoliageFallRate)
			->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp)

			->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage *_hardwoodFoliageFallRate)
			->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp);
		_landUnitData->submitOperation(bioTurnover);

		auto addbackTurnover = _landUnitData->createStockOperation();
		addbackTurnover
			->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _softwoodFoliageFallRate)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _coarseRootTurnProp)
			->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _fineRootTurnProp)

			->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _hardwoodFoliageFallRate)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _coarseRootTurnProp)
			->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _fineRootTurnProp);
		_landUnitData->submitOperation(addbackTurnover);

		double totalMerch = standSoftwoodMerch + standHardwoodMerch;

		double delSWBS = 0.0;
		double delHWBS = 0.0;
		double delSWSS = 0.0;
		double delHWSS = 0.0;
		if (M > 0 && totalMerch > 0) {
			if (standSoftwoodMerch > 0)
			{
				delSWBS = M * standSoftwoodOther / standSoftwoodMerch;
			}
			if (standHardwoodMerch > 0)
			{
				delHWBS = M * standHardwoodOther / standHardwoodMerch;
			}
			delSWSS = M * standSoftwoodMerch / totalMerch;
			delHWSS = M * standHardwoodMerch / totalMerch;
		}

		auto esgym_mortality_to_bio = _landUnitData->createStockOperation();
		esgym_mortality_to_bio->addTransfer(_atmosphere, _softwoodOther, delSWBS);
		esgym_mortality_to_bio->addTransfer(_atmosphere, _hardwoodOther, delHWBS);
		esgym_mortality_to_bio->addTransfer(_atmosphere, _softwoodMerch, delSWSS);
		esgym_mortality_to_bio->addTransfer(_atmosphere, _hardwoodMerch, delHWSS);
		_landUnitData->submitOperation(esgym_mortality_to_bio);

		auto esgym_mortality = _landUnitData->createStockOperation();
		esgym_mortality->addTransfer(_softwoodOther, _softwoodBranchSnag, delSWBS * _otherToBranchSnagSplit);
		esgym_mortality->addTransfer(_softwoodOther, _aboveGroundFastSoil, delSWBS * (1 - _otherToBranchSnagSplit));
		esgym_mortality->addTransfer(_hardwoodOther, _hardwoodBranchSnag, delHWBS * _otherToBranchSnagSplit);
		esgym_mortality->addTransfer(_hardwoodOther, _aboveGroundFastSoil, delHWBS * (1 - _otherToBranchSnagSplit));
		esgym_mortality->addTransfer(_softwoodMerch, _softwoodStemSnag, delSWSS);
		esgym_mortality->addTransfer(_hardwoodMerch, _hardwoodStemSnag, delHWSS);
		_landUnitData->submitOperation(esgym_mortality);
	}

	bool ESGYMModule::shouldRun() const {
		bool isForest = _isForest->value();
		return isForest;
	}

}}}
