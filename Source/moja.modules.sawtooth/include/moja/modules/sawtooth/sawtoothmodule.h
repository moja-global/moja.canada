#ifndef MOJA_MODULES_SAWTOOTH_SAWTOOTHMODULE_H_
#define MOJA_MODULES_SAWTOOTH_SAWTOOTHMODULE_H_
#include "moja/modules/sawtooth/_modules.sawtooth_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "sawtooth/exports.h"
#include <moja/timeseries.h>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <random>
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

	template <class TMat, class TElem>
	class SawtoothMatrixWrapper {
	private:
		std::shared_ptr<TMat> Mat;
		std::shared_ptr<TElem> Values;


	public:
		SawtoothMatrixWrapper() { }
		SawtoothMatrixWrapper(size_t nrow, size_t ncol, TElem defaultValue = (TElem)0) {
			Mat = std::make_shared<TMat>();
			Values = std::shared_ptr<TElem>(new TElem[nrow*ncol]);
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

	class SAWTOOTH_API SawtoothModule : public CBMModuleBase {
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
		void GetSiteData(Site_data& site);

		Environment_data GetEnvironmentData(int year);
		void LoadEnvironmentData();
		int environmentDataBaseYear;
		std::vector<Environment_data> environmentData;


		size_t Sawtooth_Max_Density;
		std::default_random_engine generator;

		Sawtooth_Error sawtooth_error;
		Sawtooth_Spatial_Variable spatialVar;
		//SawtoothPlotVariables sawtoothVariables;

		Sawtooth_CBM_Variable cbmVariables;

		Sawtooth_StandLevelResult standLevelResult;
		Sawtooth_CBMResult cbmResult;
		std::shared_ptr<Sawtooth_CBMAnnualProcesses> annualProcess;
		bool WasDisturbed;
		void Step(long plot_id, int year, int disturbance_type_id);
 
		flint::IVariable* PlotId;
		int RCP_Id;
		int GCM_Id;
		std::unordered_set<const moja::flint::IPool*> bioPools;

		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> speciesList;

		SawtoothMatrixWrapper<Sawtooth_Matrix, double> tmean_ann_mat;
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
		void AllocateSpecies(int* species, size_t max_density, const Site_data& site_data);
		void SawtoothModule::adjustPartialMatrix(DynamicVar e,
			const Sawtooth_CBMBiomassPools& disturbanceLosses);
	};
}}}
#endif