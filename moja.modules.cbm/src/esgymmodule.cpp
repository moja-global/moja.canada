#include "moja/flint/variable.h"


#include "moja/modules/cbm/esgymmodule.h"
#include "moja/logging.h"
#include "moja/modules/cbm/printpools.h"

namespace moja {
namespace modules {
namespace cbm {	

	void ESGYMModule::configure(const DynamicObject& config) { }

	void ESGYMModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connectSignal(signals::LocalDomainInit, &ESGYMModule::onLocalDomainInit, *this);
		notificationCenter.connectSignal(signals::TimingInit, &ESGYMModule::onTimingInit, *this);
		notificationCenter.connectSignal(signals::TimingStep, &ESGYMModule::onTimingStep, *this);
	}

	void ESGYMModule::onLocalDomainInit() {
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

		auto rootParams = _landUnitData->getVariable("root_parameters")->value().extract<DynamicObject>();
		SWRootBio = std::make_shared<SoftwoodRootBiomassEquation>(
			rootParams["sw_a"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
		HWRootBio = std::make_shared<HardwoodRootBiomassEquation>(
			rootParams["hw_a"], rootParams["hw_b"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
	}

	void ESGYMModule::onTimingInit() {
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
	void ESGYMModule::onTimingStep() {
		// Get current biomass pool values.
		updateBiomassPools();
		auto growth_esgym_fixed_effects = _landUnitData->getVariable("growth_esgym_fixed_effects")->value().extract<DynamicObject>();
		auto growth_esgym_species_specific_effects = _landUnitData->getVariable("growth_esgym_species_specific_effects")->value().extract<DynamicObject>();
		auto mortality_esgym_fixed_effects = _landUnitData->getVariable("mortality_esgym_fixed_effects")->value().extract<DynamicObject>();
		auto mortality_esgym_species_specific_effects = _landUnitData->getVariable("mortality_esgym_species_specific_effects")->value().extract<DynamicObject>();
		auto growth_esgym_environmental_effects = _landUnitData->getVariable("growth_esgym_environmental_effects")->value().extract<DynamicObject>();
		auto mortality_esgym_environmental_effects = _landUnitData->getVariable("mortality_esgym_environmental_effects")->value().extract<DynamicObject>();
		auto mean_esgym_environmental_effects = _landUnitData->getVariable("mean_esgym_environmental_effects")->value().extract<DynamicObject>();
		auto stddev_esgym_environmental_effects = _landUnitData->getVariable("stddev_esgym_environmental_effects")->value().extract<DynamicObject>();
		
		//spatial variables
		double dwf_a = _landUnitData->getVariable("dwf_a")->value();
		double rswd_a = _landUnitData->getVariable("rswd_a")->value();
		double tmean_a = _landUnitData->getVariable("tmean_a")->value();
		double vpd_a = _landUnitData->getVariable("vpd_a")->value();
		double eeq_a = _landUnitData->getVariable("eeq_a")->value();
		double ws_a = _landUnitData->getVariable("ws_a")->value();
		double ndep = _landUnitData->getVariable("ndep")->value();
		double ca = _landUnitData->getVariable("ca")->value();
		
		double G = GrowthAndMortality(_age->value(), growth_esgym_fixed_effects["b1"],
			growth_esgym_fixed_effects["b2"], growth_esgym_fixed_effects["b3"],
			growth_esgym_fixed_effects["b4"], growth_esgym_fixed_effects["b5"],
			growth_esgym_species_specific_effects["b1"], growth_esgym_species_specific_effects["b2"],
			growth_esgym_environmental_effects["eeq_n"], growth_esgym_environmental_effects["dwf_n"]);

		double M = GrowthAndMortality(_age->value(), mortality_esgym_fixed_effects["b1"],
			mortality_esgym_fixed_effects["b2"], mortality_esgym_fixed_effects["b3"],
			mortality_esgym_fixed_effects["b4"], mortality_esgym_fixed_effects["b5"],
			mortality_esgym_species_specific_effects["b1"], mortality_esgym_species_specific_effects["b2"],
			mortality_esgym_environmental_effects["eeq_n"], mortality_esgym_environmental_effects["dwf_n"]);

		double E_Growth = EnvironmentalModifier(
			growth_esgym_environmental_effects["dwf_a"], mean_esgym_environmental_effects["dwf_a"], stddev_esgym_environmental_effects["dwf_a"], dwf_a,
			growth_esgym_environmental_effects["rswd_a"], mean_esgym_environmental_effects["rswd_a"], stddev_esgym_environmental_effects["rswd_a"], rswd_a,
			growth_esgym_environmental_effects["tmean_a"], mean_esgym_environmental_effects["tmean_a"], stddev_esgym_environmental_effects["tmean_a"], tmean_a,
			growth_esgym_environmental_effects["vpd_a"], mean_esgym_environmental_effects["vpd_a"], stddev_esgym_environmental_effects["vpd_a"], vpd_a,
			growth_esgym_environmental_effects["eeq_a"], mean_esgym_environmental_effects["eeq_a"], stddev_esgym_environmental_effects["eeq_a"], eeq_a,
			growth_esgym_environmental_effects["ws_a"], mean_esgym_environmental_effects["ws_a"], stddev_esgym_environmental_effects["ws_a"], dwf_a,
			growth_esgym_environmental_effects["ndep"], mean_esgym_environmental_effects["ndep"], stddev_esgym_environmental_effects["ndep"], ndep,
			growth_esgym_environmental_effects["ca"], mean_esgym_environmental_effects["ca"], stddev_esgym_environmental_effects["ca"], ca);

		double E_Mortality = EnvironmentalModifier(
			mortality_esgym_environmental_effects["dwf_a"], mean_esgym_environmental_effects["dwf_a"], stddev_esgym_environmental_effects["dwf_a"], dwf_a,
			mortality_esgym_environmental_effects["rswd_a"], mean_esgym_environmental_effects["rswd_a"], stddev_esgym_environmental_effects["rswd_a"], rswd_a,
			mortality_esgym_environmental_effects["tmean_a"], mean_esgym_environmental_effects["tmean_a"], stddev_esgym_environmental_effects["tmean_a"], tmean_a,
			mortality_esgym_environmental_effects["vpd_a"], mean_esgym_environmental_effects["vpd_a"], stddev_esgym_environmental_effects["vpd_a"], vpd_a,
			mortality_esgym_environmental_effects["eeq_a"], mean_esgym_environmental_effects["eeq_a"], stddev_esgym_environmental_effects["eeq_a"], eeq_a,
			mortality_esgym_environmental_effects["ws_a"], mean_esgym_environmental_effects["ws_a"], stddev_esgym_environmental_effects["ws_a"], dwf_a,
			mortality_esgym_environmental_effects["ndep"], mean_esgym_environmental_effects["ndep"], stddev_esgym_environmental_effects["ndep"], ndep,
			mortality_esgym_environmental_effects["ca"], mean_esgym_environmental_effects["ca"], stddev_esgym_environmental_effects["ca"], ca);

		//clamp at 0 (the increment is never negative)
		double G_modified = std::max(0.0, G + E_Growth);
		double M_modified = std::max(0.0, M + E_Mortality);

		///*** IMPORTANT *** this is just a placeholder because there is no method to calculate component proportions
		double multiplier = 1.0 / 6.0;

		double swm = standSoftwoodMerch + G_modified * multiplier;
		double swf = standSoftwoodFoliage + G_modified * multiplier;
		double swo = standSoftwoodOther + G_modified * multiplier;

		double hwm = standHardwoodMerch + G_modified * multiplier;
		double hwf = standHardwoodFoliage + G_modified * multiplier;
		double hwo = standHardwoodOther + G_modified * multiplier;

		double swRoot = SWRootBio->calculateRootBiomass(
			swm + swf + swo);
		auto swRootBio = SWRootBio->calculateRootProportions(swRoot);
		double swCoarseIncrement = (swRootBio.coarse*swRoot) - standSWCoarseRootsCarbon;
		double swFineIncrement = (swRootBio.fine*swRoot) - standSWFineRootsCarbon;

		double hwRoot = HWRootBio->calculateRootBiomass(
			hwm + hwf + hwo);
		auto hwRootBio = HWRootBio->calculateRootProportions(hwRoot);
		double hwCoarseIncrement = (hwRootBio.coarse*hwRoot) - standHWCoarseRootsCarbon;
		double hwFineIncrement = (hwRootBio.fine*hwRoot) - standHWFineRootsCarbon;

		auto growth = _landUnitData->createStockOperation();
		growth
			->addTransfer(_atmosphere, _softwoodMerch, swm - standSoftwoodMerch)
			->addTransfer(_atmosphere, _softwoodFoliage, swf - standSoftwoodFoliage)
			->addTransfer(_atmosphere, _softwoodOther, swo - standSoftwoodOther)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, swCoarseIncrement)
			->addTransfer(_atmosphere, _softwoodFineRoots, swFineIncrement)
			->addTransfer(_atmosphere, _hardwoodMerch, hwm - standHardwoodMerch)
			->addTransfer(_atmosphere, _hardwoodFoliage, hwf - standHardwoodFoliage)
			->addTransfer(_atmosphere, _hardwoodOther, hwo - standHardwoodOther)
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
		PrintPools p;
		p.printForestPools("",_landUnitData.operator*());

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
			->addTransfer(_softwoodOther, _aboveGroundFastSoil, standSoftwoodOther * (1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, standSWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, standSWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, standSWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, standSWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp)

			->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, standHardwoodFoliage *_hardwoodFoliageFallRate)
			->addTransfer(_hardwoodOther, _aboveGroundFastSoil, standHardwoodOther * (1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
			->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, standHWCoarseRootsCarbon * _coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, standHWCoarseRootsCarbon * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, standHWFineRootsCarbon * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, standHWFineRootsCarbon * (1 - _fineRootAGSplit) * _fineRootTurnProp);
		_landUnitData->submitOperation(bioTurnover);

		auto addbackTurnover = _landUnitData->createStockOperation();
		addbackTurnover
			->addTransfer(_atmosphere, _softwoodOther, standSoftwoodOther * (1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
			->addTransfer(_atmosphere, _softwoodFoliage, standSoftwoodFoliage * _softwoodFoliageFallRate)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, standSWCoarseRootsCarbon * _coarseRootTurnProp)
			->addTransfer(_atmosphere, _softwoodFineRoots, standSWFineRootsCarbon * _fineRootTurnProp)

			->addTransfer(_atmosphere, _hardwoodOther, standHardwoodOther * (1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
			->addTransfer(_atmosphere, _hardwoodFoliage, standHardwoodFoliage * _hardwoodFoliageFallRate)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, standHWCoarseRootsCarbon * _coarseRootTurnProp)
			->addTransfer(_atmosphere, _hardwoodFineRoots, standHWFineRootsCarbon * _fineRootTurnProp);
		_landUnitData->submitOperation(addbackTurnover);

		double biomass = 
			standSoftwoodMerch + standSoftwoodOther + standSoftwoodFoliage +
			standSWCoarseRootsCarbon + standSWFineRootsCarbon +
			standHardwoodMerch + standHardwoodOther + standHardwoodFoliage +
			standHWCoarseRootsCarbon + standHWFineRootsCarbon;

		double delAGVF = 0.0;
		double delBGVF = 0.0;
		double delAGF = 0.0;
		double delBGF = 0.0;
		double delSWBS = 0.0;
		double delHWBS = 0.0;
		double delSWSS = 0.0;
		double delHWSS = 0.0;
		if (M > 0 && biomass > 0) {
			delAGVF = M * (standSoftwoodFoliage + standHardwoodFoliage +
				(standSWFineRootsCarbon + standHWFineRootsCarbon) * _fineRootAGSplit) / biomass;
			delBGVF = M * ((standSWFineRootsCarbon + standHWFineRootsCarbon) * (1 - _fineRootAGSplit)) / biomass;
			delAGF = M * ((standSoftwoodFoliage + standHardwoodFoliage)*(1 - _otherToBranchSnagSplit) +
				(standHWCoarseRootsCarbon + standSWCoarseRootsCarbon)*_coarseRootSplit) / biomass;
			delBGF = M * ((standHWCoarseRootsCarbon + standSWCoarseRootsCarbon)*(1 - _coarseRootSplit)) / biomass;
			delSWBS = M * (standSoftwoodOther * _otherToBranchSnagSplit) / biomass;
			delHWBS = M * (standHardwoodOther * _otherToBranchSnagSplit) / biomass;
			delSWSS = M * standSoftwoodMerch / biomass;
			delHWSS = M * standHardwoodMerch / biomass;
		}
		auto esgym_mortality = _landUnitData->createStockOperation();
		esgym_mortality->addTransfer(_atmosphere, _aboveGroundVeryFastSoil, delAGVF);
		esgym_mortality->addTransfer(_atmosphere, _belowGroundVeryFastSoil, delBGVF);
		esgym_mortality->addTransfer(_atmosphere, _aboveGroundFastSoil, delAGF);
		esgym_mortality->addTransfer(_atmosphere, _belowGroundFastSoil, delBGF);
		esgym_mortality->addTransfer(_atmosphere, _softwoodBranchSnag, delSWBS);
		esgym_mortality->addTransfer(_atmosphere, _hardwoodBranchSnag, delHWBS);
		esgym_mortality->addTransfer(_atmosphere, _softwoodStemSnag, delSWSS);
		esgym_mortality->addTransfer(_atmosphere, _hardwoodStemSnag, delHWSS);
		_landUnitData->submitOperation(esgym_mortality);

	}

}}}
