#include <moja/notificationcenter.h>
#include <moja/signals.h>
#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>

#include "moja/modules/cbm/sawtoothmodule.h"

namespace moja {
namespace modules {
namespace cbm {

	void SawtoothModule::configure(const DynamicObject& config) {

		Sawtooth_Error sawtooth_error;
		std::string sawtoothDbPath = config["sawtooth_db_path"];
		unsigned long long random_seed = config["random_seed"];
		Sawtooth_Max_Density = config["max_density"];
		
		speciesList = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, Sawtooth_Max_Density, 0);

		tmin_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		tmean_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		vpd_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etr_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		eeq_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ca_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ndep_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_mjjas_z_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_mjjas_n_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etr_mjjas_z_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etr_mjjas_n_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		disturbance_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		sl_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		twi_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		casl_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);

		spatialVar.tmin = *tmin_mat.Get();
		spatialVar.tmean = *tmean_mat.Get();
		spatialVar.vpd = *vpd_mat.Get();
		spatialVar.etr = *etr_mat.Get();
		spatialVar.eeq = *eeq_mat.Get();
		spatialVar.ws = *ws_mat.Get();
		spatialVar.ca = *ca_mat.Get();
		spatialVar.ndep= *ndep_mat.Get();
		spatialVar.ws_mjjas_z = *ws_mjjas_z_mat.Get();
		spatialVar.ws_mjjas_n = *ws_mjjas_n_mat.Get();
		spatialVar.etr_mjjas_z = *etr_mjjas_z_mat.Get();
		spatialVar.etr_mjjas_n = *etr_mjjas_n_mat.Get();
		spatialVar.disturbances = *disturbance_mat.Get();
		spatialVar.sl = *sl_mat.Get();
		spatialVar.twi = *twi_mat.Get();
		spatialVar.casl = *casl_mat.Get();

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

		Sawtooth_Handle = Sawtooth_Initialize(&sawtooth_error,
			sawtoothDbPath.c_str(), InitializeModelMeta(config), random_seed);

		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	Sawtooth_ModelMeta SawtoothModule::InitializeModelMeta(const DynamicObject& config) {
		Sawtooth_ModelMeta meta;

		std::string mortality_model = config["mortality_model"];
		if (mortality_model == "Sawtooth_MortalityNone") meta.mortalityModel = Sawtooth_MortalityNone;
		else if (mortality_model == "Sawtooth_MortalityConstant") meta.mortalityModel = Sawtooth_MortalityConstant;
		else if (mortality_model == "Sawtooth_MortalityD1") meta.mortalityModel = Sawtooth_MortalityD1;
		else if (mortality_model == "Sawtooth_MortalityD2") meta.mortalityModel = Sawtooth_MortalityD2;
		else if (mortality_model == "Sawtooth_MortalityES1") meta.mortalityModel = Sawtooth_MortalityES1;
		else if (mortality_model == "Sawtooth_MortalityES2") meta.mortalityModel = Sawtooth_MortalityES2;
		else if (mortality_model == "Sawtooth_MortalityMLR35") meta.mortalityModel = Sawtooth_MortalityMLR35;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth mortality_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		std::string growth_model = config["growth_model"];
		if (growth_model == "Sawtooth_GrowthD1") meta.growthModel = Sawtooth_GrowthD1;
		else if (growth_model == "Sawtooth_GrowthD2") meta.growthModel = Sawtooth_GrowthD2;
		else if (growth_model == "Sawtooth_GrowthES1") meta.growthModel = Sawtooth_GrowthES1;
		else if (growth_model == "Sawtooth_GrowthES2") meta.growthModel = Sawtooth_GrowthES2;
		else if (growth_model == "Sawtooth_GrowthES3") meta.growthModel = Sawtooth_GrowthES3;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth growth_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		std::string recruitment_model = config["recruitment_model"];
		if (recruitment_model == "Sawtooth_RecruitmentD1") meta.recruitmentModel = Sawtooth_RecruitmentD1;
		else if (recruitment_model == "Sawtooth_RecruitmentD2") meta.recruitmentModel = Sawtooth_RecruitmentD2;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth recruitment_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		return meta;
	}

	bool SawtoothModule::shouldRun() const {
		bool isForest = _isForest->value();
		return isForest;
	}

	void SawtoothModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connectSignal(signals::LocalDomainInit, &SawtoothModule::onLocalDomainInit, *this);
		notificationCenter.connectSignal(signals::DisturbanceEvent, &SawtoothModule::onDisturbanceEvent, *this);
		notificationCenter.connectSignal(signals::TimingInit, &SawtoothModule::onTimingInit, *this);
		notificationCenter.connectSignal(signals::TimingStep, &SawtoothModule::onTimingStep, *this);
		notificationCenter.connectSignal(signals::TimingShutdown, &SawtoothModule::onTimingShutdown, *this);
		notificationCenter.connectSignal(signals::SystemShutdown, &SawtoothModule::onSystemShutdown, *this);
	}

	void SawtoothModule::doLocalDomainInit() {

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

		_isForest = _landUnitData->getVariable("is_forest");

		std::string gcm_name = _landUnitData->getVariable("GCM")->value();
		if (gcm_name == "CanESM2") { GCM_Id = 1; }
		else if (gcm_name == "GFDL_ESM2G") { GCM_Id = 2; }
		else if (gcm_name == "IPSL_CM5A_LR") { GCM_Id = 3; }
		else if (gcm_name == "MIROC_ESM") { GCM_Id = 4; }
		else if (gcm_name == "NorESM1_M") { GCM_Id = 5; }
		else {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("Invalid GCM string")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}

		std::string rcp_name = _landUnitData->getVariable("RCP")->value();
		if (rcp_name == "CON") { RCP_Id = 1; }
		else if (rcp_name == "4.5") { RCP_Id = 2; }
		else if (rcp_name == "8.5") { RCP_Id = 3; }
		else {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("Invalid RCP string")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}

		const auto environment_data = _landUnitData
			->getVariable("Environment_Data")
			->value().extract<const std::vector<DynamicObject>>();

		for (const auto& row : environment_data) {
			int GCM = row["GCM"];
			int RCP = row["RCP"];
			if (GCM != GCM_Id || RCP != RCP_Id) {
				continue;
			}
			Environment_data dat;
			dat.tmean_ann = row["tmean_ann"];
			dat.tmin_ann = row["tmin_ann"];
			dat.tmean_gs = row["tmean_gs"];
			dat.etp_gs = row["etp_gs"];
			dat.ws_gs = row["ws_gs"];
			dat.etp_gs_z = row["etp_gs_z"];
			dat.ws_gs_z = row["ws_gs_z"];
			dat.etp_gs_n = row["etp_gs_n"];
			dat.ws_gs_n = row["ws_gs_n"];
			dat.ca = row["ca"];
			dat.ndep = row["ndep"];
			sawtoothVariables.AddEnvironmentData(
				row["ID_Plot"],
				row["Year"],
				dat);

		}

		const auto site_data = _landUnitData
			->getVariable("Site_Data")
			->value().extract<const std::vector<DynamicObject>>();

		for (const auto& row : site_data) {
			Site_data dat;
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
			sawtoothVariables.AddSiteData(row["ID_Plot"], dat);
		}
	}

	void SawtoothModule::doTimingInit() {

		StumpParmeterId_mat.SetValue(0, 0, _landUnitData->getVariable("StumpParameterId")->value());
		RootParameterId_mat.SetValue(0, 0, _landUnitData->getVariable("RootParameterId")->value());
		TurnoverParameterId_mat.SetValue(0, 0, _landUnitData->getVariable("TurnoverParameterId")->value());
		RegionId_mat.SetValue(0, 0, _landUnitData->getVariable("RegionId")->value());
		std::fill_n(speciesList.Get()->values, Sawtooth_Max_Density, _landUnitData->getVariable("Species")->value());
		Sawtooth_Stand_Handle = Sawtooth_Stand_Alloc(&sawtooth_error, 1,
			Sawtooth_Max_Density, *speciesList.Get(), &cbmVariables);
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}

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

	void SawtoothModule::Step(int disturbanceTypeId) {
		int regenDelay = _regenDelay->value();
		if (regenDelay > 0) {
			_regenDelay->set_value(--regenDelay);
			return;
		}

		if (!shouldRun()){
			return;
		}

		//tmin_mat.SetValue(0, 0, tmin->value());
		//tmean_mat.SetValue(0, 0, tmean->value());
		//vpd_mat.SetValue(0, 0, vpd->value());
		//etr_mat.SetValue(0, 0, etr->value());
		//eeq_mat.SetValue(0, 0, eeq->value());
		//ws_mat.SetValue(0, 0, ws->value());
		//ca_mat.SetValue(0, 0, ca->value());
		//ndep_mat.SetValue(0, 0, ndep->value());
		//ws_mjjas_z_mat.SetValue(0, 0, ws_mjjas_z->value());
		//ws_mjjas_n_mat.SetValue(0, 0, ws_mjjas_n->value());
		//etr_mjjas_z_mat.SetValue(0, 0, etr_mjjas_z->value());
		//etr_mjjas_n_mat.SetValue(0, 0, etr_mjjas_n->value());
		//disturbance_mat.SetValue(0, 0, disturbanceTypeId);
		//sl_mat.SetValue(0, 0, sl->value());
		//twi_mat.SetValue(0, 0, twi->value());
		//casl_mat.SetValue(0, 0, casl->value());

		Sawtooth_Step(&sawtooth_error, Sawtooth_Handle, Sawtooth_Stand_Handle,
			1, spatialVar, &standLevelResult, NULL, &cbmResult);

		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}

		const auto grossGrowth = cbmResult.Processes[0].GrossGrowth;
		auto growth = _landUnitData->createStockOperation();
		growth
			->addTransfer(_atmosphere, _softwoodMerch, grossGrowth.SWM)
			->addTransfer(_atmosphere, _softwoodFoliage, grossGrowth.SWF)
			->addTransfer(_atmosphere, _softwoodOther, grossGrowth.SWO)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, grossGrowth.SWCR)
			->addTransfer(_atmosphere, _softwoodFineRoots, grossGrowth.SWFR)
			->addTransfer(_atmosphere, _hardwoodMerch, grossGrowth.HWM)
			->addTransfer(_atmosphere, _hardwoodFoliage, grossGrowth.HWF)
			->addTransfer(_atmosphere, _hardwoodOther, grossGrowth.HWO)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, grossGrowth.HWCR)
			->addTransfer(_atmosphere, _hardwoodFineRoots, grossGrowth.HWFR);

		_landUnitData->submitOperation(growth);
		_landUnitData->applyOperations();

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
			Step(-1);
		}
	}

	void SawtoothModule::onDisturbanceEvent(DynamicVar e) {
		if (WasDisturbed) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("multiple disturbance events in a single timestep not supported by sawtooth")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
		
		auto& data = e.extract<const DynamicObject>();
		int disturbance_type_code = data["disturbance_type_code"];
		Step(disturbance_type_code);
		WasDisturbed = true;//prevent step getting called in doTimingStep for a second time
		
		if (standLevelResult.DisturbanceMortalityRate->GetValue(0, 0) >= 100.0) {
			//check if the disturbance is stand replacing
			//if not stand replacing, we will alter the matrix to compensate for 
			//differences between tree losses and CBM matrix proportional stand losses
			//(TODO, not yet supported for initial testing)
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details("partial disturbance not yet supported")
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
		//Otherwise the matrix is stand replacing, so the CBM matrix will work 
		//Do nothing else, because the disturbance module is next to fire,
		// and 100% of the stand is disturbed
	}


	void SawtoothModule::doTimingShutdown() {
		Sawtooth_Stand_Free(&sawtooth_error, Sawtooth_Stand_Handle);
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	void SawtoothModule::doSystemShutdown() {
		Sawtooth_Free(&sawtooth_error, Sawtooth_Handle);
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}



}}}