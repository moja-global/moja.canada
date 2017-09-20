#include <moja/notificationcenter.h>
#include <moja/signals.h>
#include <moja/flint/variable.h>

#include "moja/modules/cbm/sawtoothmodule.h"

// macro to create a scope-temporary pointer to pointer variable with a single
// element for sawtooth interoperability
#define sawtooth_temp_pp(x,y, T) T y ## _temp[] = { x }; T* y[1] = { y ## _temp };

namespace moja {
namespace modules {
namespace cbm {

	void SawtoothModule::configure(const DynamicObject& config) {

		Sawtooth_Error sawtooth_error;
		std::string sawtoothDbPath = config["sawtooth_db_path"];
		unsigned long long random_seed = config["random_seed"];
		Sawtooth_Max_Density = config["max_density"];
		
		speciesList = std::vector<int>(Sawtooth_Max_Density);

		Sawtooth_Handle = Sawtooth_Initialize(&sawtooth_error,
			sawtoothDbPath.c_str(), InitializeModelMeta(config), random_seed);

		standLevelResult = std::shared_ptr<SawtoothStandLevelResultsWrapper>(
			new SawtoothStandLevelResultsWrapper(1, 1));
		treeLevelResults = std::shared_ptr<SawtoothTreeLevelResultsWrapper>(
			new SawtoothTreeLevelResultsWrapper(1, Sawtooth_Max_Density));

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
		else if (mortality_model == "Sawtooth_MortalityDefault") meta.mortalityModel = Sawtooth_MortalityDefault;
		else if (mortality_model == "Sawtooth_MortalityES1") meta.mortalityModel = Sawtooth_MortalityES1;
		else if (mortality_model == "Sawtooth_MortalityES2") meta.mortalityModel = Sawtooth_MortalityES2;
		else if (mortality_model == "Sawtooth_MortalityMLR35") meta.mortalityModel = Sawtooth_MortalityMLR35;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth mortality_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		std::string growth_model = config["growth_model"];
		if (growth_model == "Sawtooth_GrowthDefault") meta.growthModel = Sawtooth_GrowthDefault;
		else if (growth_model == "Sawtooth_GrowthES1") meta.growthModel = Sawtooth_GrowthES1;
		else if (growth_model == "Sawtooth_GrowthES2") meta.growthModel = Sawtooth_GrowthES2;
		else if (growth_model == "Sawtooth_GrowthES3") meta.growthModel = Sawtooth_GrowthES3;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth growth_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		std::string recruitment_model = config["recruitment_model"];
		if (recruitment_model == "Sawtooth_RecruitmentDefault") meta.recruitmentModel = Sawtooth_RecruitmentDefault;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth recruitment_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		return meta;
	}

	void SawtoothModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connectSignal(signals::LocalDomainInit, &SawtoothModule::onLocalDomainInit, *this);
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

		_turnoverRates = _landUnitData->getVariable("turnover_rates");

		_regenDelay = _landUnitData->getVariable("regen_delay");

		_currentLandClass = _landUnitData->getVariable("current_land_class");

		auto rootParams = _landUnitData->getVariable("root_parameters")->value().extract<DynamicObject>();
		SWRootBio = std::make_shared<SoftwoodRootBiomassEquation>(
			rootParams["sw_a"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
		HWRootBio = std::make_shared<HardwoodRootBiomassEquation>(
			rootParams["hw_a"], rootParams["hw_b"], rootParams["frp_a"], 
			rootParams["frp_b"], rootParams["frp_c"]);
	}

	void SawtoothModule::doTimingInit() {

		int* specList[1] = { speciesList.data() };
		Sawtooth_Stand_Handle = Sawtooth_Stand_Alloc(&sawtooth_error, 1,
			Sawtooth_Max_Density, specList);
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	void SawtoothModule::doTimingStep() {

		sawtooth_temp_pp(tmin->value().extract<double>(), tmin_pp, double);
		sawtooth_temp_pp(tmean->value().extract<double>(), tmean_pp, double);
		sawtooth_temp_pp(vpd->value().extract<double>(), vpd_pp, double);
		sawtooth_temp_pp(etr->value().extract<double>(), etr_pp, double);
		sawtooth_temp_pp(eeq->value().extract<double>(), eeq_pp, double);
		sawtooth_temp_pp(ws->value().extract<double>(), ws_pp, double);
		sawtooth_temp_pp(ca->value().extract<double>(), ca_pp, double);
		sawtooth_temp_pp(ndep->value().extract<double>(), ndep_pp, double);
		sawtooth_temp_pp(ws_mjjas_z->value().extract<double>(), ws_mjjas_z_pp, double);
		double ws_mjjas_n_p[1] = { ws_mjjas_n->value().extract<double>() };
		sawtooth_temp_pp(etr_mjjas_z->value().extract<double>(), etr_mjjas_z_pp, double);
		double etr_mjjas_n_p[1] = { etr_mjjas_n->value().extract<double>() };
		sawtooth_temp_pp(disturbance->value().extract<int>(), disturbances_pp, int);
		
		Sawtooth_Step(&sawtooth_error, Sawtooth_Handle, Sawtooth_Stand_Handle,
			1, tmin_pp, tmean_pp, vpd_pp, etr_pp, eeq_pp, ws_pp, ca_pp,
			ndep_pp, ws_mjjas_z_pp, ws_mjjas_n_p, etr_mjjas_z_pp,
			etr_mjjas_n_p, disturbances_pp, standLevelResult->Get(), 
			treeLevelResults->Get());
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
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