#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "sawtooth/exports.h"
#include <moja/timeseries.h>
#include <unordered_map>
#include <string>
namespace moja {
namespace modules {
namespace cbm {

	struct Environment_data {

		double tmean_ann;
		double tmin_ann;
		double tmean_gs;
		double etp_gs;
		double ws_gs;
		double etp_gs_z;
		double ws_gs_z;
		double etp_gs_n;
		double ws_gs_n;
		double ca;
		double ndep;
	};

	struct Site_data {
		int ID_Spc1;
		int ID_Spc2;
		int ID_Spc3;
		int ID_Spc4;
		double Frac_Spc1;
		double Frac_Spc2;
		double Frac_Spc3;
		double Frac_Spc4;
		int Slope;
		int Aspect;
		int TWI;
	};

	class EnvironmentTimeSeries {
	private:
		std::unordered_map<int, std::shared_ptr<Environment_data>> series;
	public:

		void Add(int year, const Environment_data& data) {
			if (year < 0)
			{
				BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
					<< moja::flint::Details("negative year")
					<< moja::flint::LibraryName("moja.modules.cbm")
					<< moja::flint::ModuleName("sawtoothmodule"));
			}
			if (series.count(year) != 0) {
				BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
					<< moja::flint::Details("duplicate year")
					<< moja::flint::LibraryName("moja.modules.cbm")
					<< moja::flint::ModuleName("sawtoothmodule"));
			}
			series[year] = std::make_shared<Environment_data>(Environment_data(data));
		}
		std::shared_ptr<Environment_data> Get(int year) {
			return series.at(year);
		}

	};

	class SawtoothPlotVariables {
	private:
		std::unordered_map<long, std::shared_ptr<EnvironmentTimeSeries>> env;
		std::unordered_map<long, std::shared_ptr<Site_data>> site;
	public:
		void AddEnvironmentData(long plot_id, int year, const Environment_data& data) {
			if (env.count(plot_id) == 0) {
				auto value = std::make_shared<EnvironmentTimeSeries>();
				value->Add(year, data);
				env[plot_id] = value;
			}
			else {
				env[plot_id]->Add(year, data);
			}
		}
		std::shared_ptr<Environment_data> GetEnvironmentData(long plot_id, int year) {
			return env.at(plot_id)->Get(year);
		}
		void AddSiteData(long plot_id, const Site_data& data) {
			if (site.count(plot_id) != 0) {
				BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
					<< moja::flint::Details("duplicate id in site data records")
					<< moja::flint::LibraryName("moja.modules.cbm")
					<< moja::flint::ModuleName("sawtoothmodule"));
			}
			site[plot_id] = std::make_shared<Site_data>(Site_data(data));
		}
		std::shared_ptr<Site_data> GetSiteData(long plot_id) {
			return site.at(plot_id);
		}
	};

	template <class TMat, class TElem>
	class SawtoothMatrixWrapper {
	private:
		std::shared_ptr<TMat> Mat;
		std::shared_ptr<TElem> Values;
	public:
		SawtoothMatrixWrapper() { }
		SawtoothMatrixWrapper(size_t nrow, size_t ncol, TElem defaultValue = (TElem)0) {
			Mat = std::make_shared<TMat>();
			Values = std::make_shared<TElem>(nrow*ncol);
			std::fill_n(Values.get(), nrow*ncol, defaultValue);
			Mat->rows = nrow;
			Mat->cols = ncol;
		}

		TMat* Get() {
			Mat->values = Values.get();
			return Mat.get();
		}

		double GetValue(size_t row, size_t col) {
			return Get()->GetValue(row, col);
		}

		void SetValue(size_t row, size_t col, TElem value) {
			Get()->SetValue(row, col, value);
		}
	};

	class CBM_API SawtoothModule : public CBMModuleBase {
	public:
		SawtoothModule() : CBMModuleBase() { }
		virtual ~SawtoothModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void doLocalDomainInit() override;
		void doTimingInit() override;
		void doTimingStep() override;
		void doTimingShutdown() override;
		void doSystemShutdown() override;
		void onDisturbanceEvent(DynamicVar e) override;

	private:


		void* Sawtooth_Handle;
		void* Sawtooth_Stand_Handle;
		size_t Sawtooth_Max_Density;

		Sawtooth_Error sawtooth_error;
		Sawtooth_Spatial_Variable spatialVar;
		SawtoothPlotVariables sawtoothVariables;

		Sawtooth_CBM_Variable cbmVariables;

		Sawtooth_StandLevelResult standLevelResult;
		Sawtooth_CBMResult cbmResult;
		std::shared_ptr<Sawtooth_CBMAnnualProcesses> annualProcess;
		bool WasDisturbed;
		void Step(long plot_id, int year, int disturbance_type_id);
 
		flint::IVariable* PlotId;
		int RCP_Id;
		int GCM_Id;

		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> speciesList;

		SawtoothMatrixWrapper<Sawtooth_Matrix, double> tmin_ann_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> tmean_gs_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> vpd_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> etp_gs_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> eeq_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ws_gs_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ca_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ndep_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ws_gs_z_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ws_gs_n_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> etp_gs_z_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> etp_gs_n_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> disturbance_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> slope_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> twi_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> aspect_mat;

		SawtoothMatrixWrapper<Sawtooth_Matrix, double> MeanAge_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> MeanHeight_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> StandDensity_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> TotalBiomassCarbon_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> TotalBiomassCarbonGrowth_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> MeanBiomassCarbon_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> RecruitmentRate_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> MortalityRate_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> MortalityCarbon_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> DisturbanceType_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> DisturbanceMortalityRate_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> DisturbanceMortalityCarbon_mat;

		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> StumpParmeterId_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> RootParameterId_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> TurnoverParameterId_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> RegionId_mat;

		const flint::IPool* _softwoodMerch;
		const flint::IPool* _softwoodOther;
		const flint::IPool* _softwoodFoliage;
		const flint::IPool* _softwoodCoarseRoots;
		const flint::IPool* _softwoodFineRoots;

		const flint::IPool* _hardwoodMerch;
		const flint::IPool* _hardwoodOther;
		const flint::IPool* _hardwoodFoliage;
		const flint::IPool* _hardwoodCoarseRoots;
		const flint::IPool* _hardwoodFineRoots;

		const flint::IPool* _aboveGroundVeryFastSoil;
		const flint::IPool* _aboveGroundFastSoil;
		const flint::IPool* _belowGroundVeryFastSoil;
		const flint::IPool* _belowGroundFastSoil;
		const flint::IPool* _softwoodStemSnag;
		const flint::IPool* _softwoodBranchSnag;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;
		const flint::IPool* _mediumSoil;
		const flint::IPool* _atmosphere;

		flint::IVariable* _age;
		flint::IVariable* _turnoverRates;
		flint::IVariable* _regenDelay;

		flint::IVariable* _isForest;

		flint::IVariable* _standSPUID;

		double _softwoodFoliageFallRate;
		double _hardwoodFoliageFallRate;
		double _stemAnnualTurnOverRate;
		double _softwoodBranchTurnOverRate;
		double _hardwoodBranchTurnOverRate;
		double _otherToBranchSnagSplit;
		double _stemSnagTurnoverRate;
		double _branchSnagTurnoverRate;
		double _coarseRootSplit;
		double _coarseRootTurnProp;
		double _fineRootAGSplit;
		double _fineRootTurnProp;

		bool shouldRun() const;
		Sawtooth_ModelMeta InitializeModelMeta(const DynamicObject& config);
	};
}}}