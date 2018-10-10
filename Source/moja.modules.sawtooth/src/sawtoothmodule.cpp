#include <moja/notificationcenter.h>
#include <moja/signals.h>
#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>
#include <moja/logging.h>
#include <boost/format.hpp>

#include "moja/modules/cbm/cbmdisturbancelistener.h"
#include "moja/modules/sawtooth/sawtoothmodule.h"

namespace moja {
namespace modules {
namespace cbm {

	void* Sawtooth_Handle = NULL;
	thread_local void* Sawtooth_Stand_Handle = NULL;

	void SawtoothModule::configure(const DynamicObject& config) {
		MOJA_LOG_INFO << "configure " << (long)this;

		tmean_ann_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		tmin_ann_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		tmean_gs_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		vpd_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etp_gs_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		eeq_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_gs_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ca_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ndep_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_gs_z_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_gs_n_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etp_gs_z_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etp_gs_n_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		disturbance_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		slope_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		twi_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		aspect_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);

		spatialVar.tmean_ann = *tmin_ann_mat.Get();
		spatialVar.tmin_ann = *tmin_ann_mat.Get();
		spatialVar.tmean_gs = *tmean_gs_mat.Get();
		spatialVar.vpd = *vpd_mat.Get();
		spatialVar.etp_gs = *etp_gs_mat.Get();
		spatialVar.eeq = *eeq_mat.Get();
		spatialVar.ws_gs = *ws_gs_mat.Get();
		spatialVar.ca = *ca_mat.Get();
		spatialVar.ndep = *ndep_mat.Get();
		spatialVar.ws_gs_z = *ws_gs_z_mat.Get();
		spatialVar.ws_gs_n = *ws_gs_n_mat.Get();
		spatialVar.etp_gs_z = *etp_gs_z_mat.Get();
		spatialVar.etp_gs_n = *etp_gs_n_mat.Get();
		spatialVar.disturbances = *disturbance_mat.Get();
		spatialVar.slope = *slope_mat.Get();
		spatialVar.twi = *twi_mat.Get();
		spatialVar.aspect = *aspect_mat.Get();

		MeanAge_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MeanHeight_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		StandDensity_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		TotalBiomassCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		TotalBiomassCarbonGrowth_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MeanBiomassCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		RecruitmentRate_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MortalityRate_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MortalityCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		DisturbanceType_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		DisturbanceMortalityRate_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		DisturbanceMortalityCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);

		standLevelResult.MeanAge = MeanAge_mat.Get();
		standLevelResult.MeanHeight = MeanHeight_mat.Get();
		standLevelResult.StandDensity = StandDensity_mat.Get();
		standLevelResult.TotalBiomassCarbon = TotalBiomassCarbon_mat.Get();
		standLevelResult.TotalBiomassCarbonGrowth = TotalBiomassCarbonGrowth_mat.Get();
		standLevelResult.MeanBiomassCarbon = MeanBiomassCarbon_mat.Get();
		standLevelResult.RecruitmentRate = RecruitmentRate_mat.Get();
		standLevelResult.MortalityRate = MortalityRate_mat.Get();
		standLevelResult.MortalityCarbon = MortalityCarbon_mat.Get();
		standLevelResult.DisturbanceType = DisturbanceType_mat.Get();
		standLevelResult.DisturbanceMortalityRate = DisturbanceMortalityRate_mat.Get();
		standLevelResult.DisturbanceMortalityCarbon = DisturbanceMortalityCarbon_mat.Get();

		StumpParmeterId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		RootParameterId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		TurnoverParameterId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		RegionId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);

		cbmVariables.StumpParameterId = *StumpParmeterId_mat.Get();
		cbmVariables.RootParameterId = *RootParameterId_mat.Get();
		cbmVariables.TurnoverParameterId = *TurnoverParameterId_mat.Get();
		cbmVariables.RegionId = *RegionId_mat.Get();

		annualProcess = std::make_shared<Sawtooth_CBMAnnualProcesses>();
		cbmResult.Processes = annualProcess.get();
		
		Sawtooth_Max_Density = config["max_density"];
		unsigned long long random_seed = config["random_seed"];

		speciesList = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, Sawtooth_Max_Density, 0);
		generator = std::default_random_engine(random_seed);

		if (Sawtooth_Handle == NULL) { // protect the handle from multiple calls to configure which may be called multiple times

			MOJA_LOG_INFO << "Sawtooth_Initialize " << (long)this;
			Sawtooth_Error sawtooth_error;
			std::string sawtoothDbPath = config["sawtooth_db_path"];

			Sawtooth_Handle = Sawtooth_Initialize(&sawtooth_error,
				sawtoothDbPath.c_str(), InitializeModelMeta(config), random_seed);

			if (sawtooth_error.Code != Sawtooth_NoError) {
				MOJA_LOG_FATAL << std::string(sawtooth_error.Message);
			}
		}
	}

	Sawtooth_ModelMeta SawtoothModule::InitializeModelMeta(const DynamicObject& config) {
		Sawtooth_ModelMeta meta;
		meta.CBMEnabled = true;
		std::string mortality_model = config["mortality_model"];
		if (mortality_model == "Sawtooth_MortalityNone") meta.mortalityModel = Sawtooth_MortalityNone;
		else if (mortality_model == "Sawtooth_MortalityConstant") meta.mortalityModel = Sawtooth_MortalityConstant;
		else if (mortality_model == "Sawtooth_MortalityD1") meta.mortalityModel = Sawtooth_MortalityD1;
		else if (mortality_model == "Sawtooth_MortalityD2") meta.mortalityModel = Sawtooth_MortalityD2;
		else if (mortality_model == "Sawtooth_MortalityES1") meta.mortalityModel = Sawtooth_MortalityES1;
		else if (mortality_model == "Sawtooth_MortalityES2") meta.mortalityModel = Sawtooth_MortalityES2;
		else if (mortality_model == "Sawtooth_MortalityMLR35") meta.mortalityModel = Sawtooth_MortalityMLR35;
		else MOJA_LOG_FATAL << "specified sawtooth mortality_model not valid";


		std::string growth_model = config["growth_model"];
		if (growth_model == "Sawtooth_GrowthD1") meta.growthModel = Sawtooth_GrowthD1;
		else if (growth_model == "Sawtooth_GrowthD2") meta.growthModel = Sawtooth_GrowthD2;
		else if (growth_model == "Sawtooth_GrowthES1") meta.growthModel = Sawtooth_GrowthES1;
		else if (growth_model == "Sawtooth_GrowthES2") meta.growthModel = Sawtooth_GrowthES2;
		else if (growth_model == "Sawtooth_GrowthES3") meta.growthModel = Sawtooth_GrowthES3;
		else MOJA_LOG_FATAL << "specified sawtooth growth_model not valid";

		std::string recruitment_model = config["recruitment_model"];
		if (recruitment_model == "Sawtooth_RecruitmentD1") meta.recruitmentModel = Sawtooth_RecruitmentD1;
		else if (recruitment_model == "Sawtooth_RecruitmentD2") meta.recruitmentModel = Sawtooth_RecruitmentD2;
		else MOJA_LOG_FATAL << "specified sawtooth recruitment_model not valid";

		return meta;
	}

	bool SawtoothModule::shouldRun() const {
		bool isForest = _isForest->value();
		return isForest;
	}

	void SawtoothModule::subscribe(NotificationCenter& notificationCenter) {
		MOJA_LOG_INFO << "subscribe " << (long)this;
		notificationCenter.connectSignal(signals::LocalDomainInit, &SawtoothModule::onLocalDomainInit, *this);
		notificationCenter.connectSignal(signals::DisturbanceEvent, &SawtoothModule::onDisturbanceEvent, *this);
		notificationCenter.connectSignal(signals::TimingInit, &SawtoothModule::onTimingInit, *this);
		notificationCenter.connectSignal(signals::TimingStep, &SawtoothModule::onTimingStep, *this);
		notificationCenter.connectSignal(signals::TimingShutdown, &SawtoothModule::onTimingShutdown, *this);
		notificationCenter.connectSignal(signals::SystemShutdown, &SawtoothModule::onSystemShutdown, *this);
	}

	void SawtoothModule::doLocalDomainInit() {

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


		_softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
		_softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");

		_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
		_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");

		_aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
		_aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
		_belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
		_belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");

		_mediumSoil = _landUnitData->getPool("MediumSoil");
		_atmosphere = _landUnitData->getPool("Atmosphere");

		bioPools = std::unordered_set<const moja::flint::IPool*>
		{
			_softwoodMerch,
			_softwoodFoliage,
			_softwoodOther,
			_softwoodCoarseRoots,
			_softwoodFineRoots,
			_hardwoodMerch,
			_hardwoodFoliage,
			_hardwoodOther,
			_hardwoodCoarseRoots,
			_hardwoodFineRoots,
		};


		_age = _landUnitData->getVariable("age");

		_turnoverRates = _landUnitData->getVariable("turnover_rates");

		_regenDelay = _landUnitData->getVariable("regen_delay");

		_isForest = _landUnitData->getVariable("is_forest");

		PlotId = _landUnitData->getVariable("Plot_id");
	}

	void SawtoothModule::AllocateSpecies(int* species, size_t max_density,
		const Site_data& site_data) {
		std::discrete_distribution<int> distribution
		{ 
			(double)site_data.Frac_Spc1,
			(double)site_data.Frac_Spc2,
			(double)site_data.Frac_Spc3,
			(double)site_data.Frac_Spc4
		};
		std::vector<int> species_ids = { 
			site_data.ID_Spc1,
			site_data.ID_Spc2,
			site_data.ID_Spc3,
			site_data.ID_Spc4 
		};
		for (auto i = 0; i < max_density; i++) {
			species[i] = species_ids[distribution(generator)];
		}
	}

	void SawtoothModule::GetSiteData(Site_data& dat) {
		auto row = _landUnitData->getVariable("Site_Data")->value();
		dat.ID_Spc1 = row["ID_Spc1"];
		dat.ID_Spc2 = row["ID_Spc2"];
		dat.ID_Spc3 = row["ID_Spc3"];
		dat.ID_Spc4 = row["ID_Spc4"];
		dat.Frac_Spc1 = row["Frac_Spc1"];
		dat.Frac_Spc2 = row["Frac_Spc2"];
		dat.Frac_Spc3 = row["Frac_Spc3"];
		dat.Frac_Spc4 = row["Frac_Spc4"];
		dat.Slope = row["Slope"];
		dat.Aspect = row["Aspect"];
		dat.TWI = row["TWI"];
	}

	void SawtoothModule::LoadEnvironmentData() {
		environmentData.clear();
		auto e = _landUnitData->getVariable("Environment_Data")->value()
			.extract<const std::vector<DynamicObject>>();
		environmentDataBaseYear = e[0]["Year"];
		for (const auto& row : e) {
			Environment_data env;
			env.tmean_ann = row["tmean_ann"];
			env.tmin_ann = row["tmin_ann"];
			env.tmean_gs = row["tmean_gs"];
			env.etp_gs = row["etp_gs"];
			env.ws_gs = row["ws_gs"];
			env.etp_gs_z = row["etp_gs_z"];
			env.ws_gs_z = row["ws_gs_z"];
			env.etp_gs_n = row["etp_gs_n"];
			env.ws_gs_n = row["ws_gs_n"];
			env.ca = row["ca"];
			env.ndep = row["ndep"];
			environmentData.push_back(env);
		}
	}

	Environment_data SawtoothModule::GetEnvironmentData(int year) {
		int index = year - environmentDataBaseYear;
		index = std::min((int)environmentData.size()-1, std::max(0, index));
		return environmentData[index];
	}

	void SawtoothModule::doTimingInit() {
		MOJA_LOG_INFO << "doTimingInit " << (long)this;
		LoadEnvironmentData();

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

		if (Sawtooth_Stand_Handle == NULL) { 
			MOJA_LOG_INFO << "Sawtooth_Stand_Alloc " << (long)this;
			Site_data site_data;
			GetSiteData(site_data);
			StumpParmeterId_mat.SetValue(0, 0, _landUnitData->getVariable("StumpParameterId")->value());
			RootParameterId_mat.SetValue(0, 0, _landUnitData->getVariable("RootParameterId")->value());
			TurnoverParameterId_mat.SetValue(0, 0, _landUnitData->getVariable("TurnoverParameterId")->value());
			RegionId_mat.SetValue(0, 0, _landUnitData->getVariable("spatial_unit_id")->value());
			AllocateSpecies(speciesList.Get()->values, Sawtooth_Max_Density, site_data);
			Sawtooth_Stand_Handle = Sawtooth_Stand_Alloc(&sawtooth_error, 1,
				Sawtooth_Max_Density, *speciesList.Get(), &cbmVariables);
			if (sawtooth_error.Code != Sawtooth_NoError) {
				MOJA_LOG_FATAL << std::string(sawtooth_error.Message);
			}


		}
	}

	void SawtoothModule::Step(long plot_id, int year, int disturbance_type_id) {
		int regenDelay = _regenDelay->value();
		if (regenDelay > 0) {
			_regenDelay->set_value(--regenDelay);
			return;
		}

		if (!shouldRun()){
			return;
		}

		Environment_data env = GetEnvironmentData(year);
		
		spatialVar.tmean_ann.SetValue(0, 0, env.tmean_ann);
		spatialVar.tmin_ann.SetValue(0, 0, env.tmin_ann);
		spatialVar.tmean_gs.SetValue(0, 0, env.tmean_gs);
		spatialVar.etp_gs.SetValue(0,0, env.etp_gs);
		spatialVar.ws_gs.SetValue(0, 0, env.ws_gs);
		spatialVar.etp_gs_z.SetValue(0, 0, env.etp_gs_z);
		spatialVar.ws_gs_z.SetValue(0, 0, env.ws_gs_z);
		spatialVar.etp_gs_n.SetValue(0, 0, env.etp_gs_n);
		spatialVar.ws_gs_n.SetValue(0, 0, env.ws_gs_n);
		spatialVar.ca.SetValue(0, 0, env.ca);
		spatialVar.ndep.SetValue(0, 0, env.ndep);

		Site_data site;
		GetSiteData(site);
		spatialVar.slope.SetValue(0, 0, site.Slope);
		spatialVar.twi.SetValue(0, 0, site.TWI);
		spatialVar.aspect.SetValue(0, 0, site.Aspect);

		spatialVar.disturbances.SetValue(0, 0, disturbance_type_id);

		Sawtooth_Step(&sawtooth_error, Sawtooth_Handle, Sawtooth_Stand_Handle,
			1, spatialVar, &standLevelResult, NULL, &cbmResult);

		if (sawtooth_error.Code != Sawtooth_NoError) {
			MOJA_LOG_FATAL << std::string(std::string(sawtooth_error.Message));
		}

		const auto npp = cbmResult.Processes[0].NPP;
		auto growth = _landUnitData->createStockOperation();
		growth
			->addTransfer(_atmosphere, _softwoodMerch, npp.SWM)
			->addTransfer(_atmosphere, _softwoodFoliage, npp.SWF)
			->addTransfer(_atmosphere, _softwoodOther, npp.SWO)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, npp.SWCR)
			->addTransfer(_atmosphere, _softwoodFineRoots, npp.SWFR)
			->addTransfer(_atmosphere, _hardwoodMerch, npp.HWM)
			->addTransfer(_atmosphere, _hardwoodFoliage, npp.HWF)
			->addTransfer(_atmosphere, _hardwoodOther, npp.HWO)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, npp.HWCR)
			->addTransfer(_atmosphere, _hardwoodFineRoots, npp.HWFR);

		_landUnitData->submitOperation(growth);


		auto domTurnover = _landUnitData->createProportionalOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag, _mediumSoil, _stemSnagTurnoverRate)
			->addTransfer(_softwoodBranchSnag, _aboveGroundFastSoil, _branchSnagTurnoverRate)
			->addTransfer(_hardwoodStemSnag, _mediumSoil, _stemSnagTurnoverRate)
			->addTransfer(_hardwoodBranchSnag, _aboveGroundFastSoil, _branchSnagTurnoverRate);

		// litterfall and mortality as stock operations
		for (const auto losses : {
			cbmResult.Processes[0].Litterfall,
			cbmResult.Processes[0].Mortality
		}) {
			auto lossesOp = _landUnitData->createStockOperation();
			lossesOp
				->addTransfer(_softwoodMerch, _softwoodStemSnag, losses.SWM)
				->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, losses.SWF)
				->addTransfer(_softwoodOther, _softwoodBranchSnag, losses.SWO * _otherToBranchSnagSplit)
				->addTransfer(_softwoodOther, _aboveGroundFastSoil, losses.SWO * (1 - _otherToBranchSnagSplit))
				->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, losses.SWCR * _coarseRootSplit)
				->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, losses.SWCR * (1 - _coarseRootSplit))
				->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, losses.SWFR * _fineRootAGSplit)
				->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, losses.SWFR * (1 - _fineRootAGSplit))

				->addTransfer(_hardwoodMerch, _hardwoodStemSnag, losses.HWM)
				->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, losses.HWF)
				->addTransfer(_hardwoodOther, _hardwoodBranchSnag, losses.HWO * _otherToBranchSnagSplit)
				->addTransfer(_hardwoodOther, _aboveGroundFastSoil, losses.HWO * (1 - _otherToBranchSnagSplit))
				->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, losses.HWO * _coarseRootSplit)
				->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, losses.HWCR * (1 - _coarseRootSplit))
				->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, losses.HWFR * _fineRootAGSplit)
				->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, losses.HWFR * (1 - _fineRootAGSplit));
			_landUnitData->submitOperation(lossesOp);
		}

		_age->set_value(standLevelResult.MeanAge->GetValue(0, 0));
		
	}

	void SawtoothModule::doTimingStep() {
		if (WasDisturbed) {
			WasDisturbed = false;
			return;
		}
		else {
			
			Step(PlotId->value(), 
				_landUnitData->timing()->curStartDate().year(), 
				-1);
		}
	}

	void SawtoothModule::adjustPartialMatrix(DynamicVar e, 
		const Sawtooth_CBMBiomassPools& disturbanceLosses) {
		std::unordered_map<const moja::flint::IPool*, double> currentBioPools
		{
			{_softwoodMerch, _softwoodMerch->value() },
			{_softwoodFoliage, _softwoodFoliage->value() },
			{_softwoodOther, _softwoodOther->value() },
			{_softwoodCoarseRoots, _softwoodCoarseRoots->value() },
			{_softwoodFineRoots, _softwoodFineRoots->value() },
			{_hardwoodMerch, _hardwoodMerch->value() },
			{_hardwoodFoliage, _hardwoodFoliage->value() },
			{_hardwoodOther, _hardwoodOther->value() },
			{_hardwoodCoarseRoots, _hardwoodCoarseRoots->value() },
			{_hardwoodFineRoots, _hardwoodFineRoots->value() }
		};
		std::unordered_map<const moja::flint::IPool*, double> sawtoothBioLosses
		{
			{ _softwoodMerch, disturbanceLosses.SWM },
			{ _softwoodFoliage, disturbanceLosses.SWF },
			{ _softwoodOther, disturbanceLosses.SWO },
			{ _softwoodCoarseRoots, disturbanceLosses.SWCR },
			{ _softwoodFineRoots, disturbanceLosses.SWFR },
			{ _hardwoodMerch, disturbanceLosses.HWM },
			{ _hardwoodFoliage, disturbanceLosses.HWF },
			{ _hardwoodOther, disturbanceLosses.HWO },
			{ _hardwoodCoarseRoots, disturbanceLosses.HWCR },
			{ _hardwoodFineRoots, disturbanceLosses.HWFR },
		};

		auto& data = e.extract<DynamicObject>();
		auto distMatrix = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();

		std::unordered_map<const moja::flint::IPool*, double> bioLossProportions;
		for (auto row : *distMatrix.get()) {
			//gather the total sinks from biomass pools
			const moja::flint::IPool* src = row.sourcePool();
			const moja::flint::IPool* sink = row.destPool();
			if (bioPools.count(src) == 1 && src != sink) {
				bioLossProportions[src] += row.proportion();
			}
		}

		std::unordered_map<const moja::flint::IPool*, double> adjustedBiomassRetained;
		for (auto p : bioPools) {
			double sawtoothLoss = sawtoothBioLosses[p];
			double cbmpool = currentBioPools[p];
			double retention = 0.0;
			if (cbmpool != 0 && sawtoothLoss <= cbmpool) {
				//if the pool was already 0 or the sawtooth loss is greater than the remaining cbmpool
				// then the retention is zero
				retention = 1.0 - (sawtoothLoss / cbmpool);
			}

			adjustedBiomassRetained[p] = retention;
		}

		auto mat = distMatrix.get();
		for (size_t r = 0; r < mat->size(); r++) {
			const moja::flint::IPool* src = mat->at(r).sourcePool();
			const moja::flint::IPool* sink = mat->at(r).destPool();
			if (bioPools.count(src) == 1 && sink != src) {
				double sinkC = mat->at(r).proportion();
				double adjustedSink = (1.0 - adjustedBiomassRetained[src])
					/ bioLossProportions[src] * sinkC;
				mat->at(r).setProportion(adjustedSink);
			}
		}
	}

	void SawtoothModule::onDisturbanceEvent(DynamicVar e) {
		if (WasDisturbed) {
			MOJA_LOG_FATAL << "multiple disturbance events in a single timestep not supported by sawtooth";
		}
		
		auto& data = e.extract<const DynamicObject>();

		int disturbance_type_code = data["disturbance_type_code"];
		Step(PlotId->value(),
			_landUnitData->timing()->curStartDate().year(), 
			disturbance_type_code);
		WasDisturbed = true;//prevent step getting called in doTimingStep for a second time
		
		if (standLevelResult.StandDensity->GetValue(0, 0) != 0.0) {
			//check if the disturbance is stand replacing
			//if not stand replacing, we will alter the matrix to compensate for 
			//differences between tree losses and CBM matrix proportional stand losses
			adjustPartialMatrix(e, cbmResult.Processes->Disturbance);
		}
		//Otherwise the matrix is stand replacing, so the CBM matrix will work 
		//Do nothing else, because the disturbance module is next to fire,
		// and 100% of the stand is disturbed
	}

	void SawtoothModule::doTimingShutdown() {
		MOJA_LOG_INFO << "doTimingShutdown " << (long)this;
		MOJA_LOG_INFO << "Sawtooth_Stand_Free " << (long)this;
		Sawtooth_Stand_Free(&sawtooth_error, Sawtooth_Stand_Handle);
		Sawtooth_Stand_Handle = NULL;
		if (sawtooth_error.Code != Sawtooth_NoError) {
			MOJA_LOG_FATAL << std::string(sawtooth_error.Message);
		}
	}

	void SawtoothModule::doSystemShutdown() {
		MOJA_LOG_INFO << "doSystemShutdown " << (long)this;
		if (Sawtooth_Handle != NULL) {
			MOJA_LOG_INFO << "Sawtooth_Free " << (long)this;
			Sawtooth_Free(&sawtooth_error, Sawtooth_Handle);
			if (sawtooth_error.Code != Sawtooth_NoError) {
				MOJA_LOG_FATAL << std::string(sawtooth_error.Message);
			}
			Sawtooth_Handle = NULL;
		}
	}
}}}