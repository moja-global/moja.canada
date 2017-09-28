#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "sawtooth/exports.h"

namespace moja {
namespace modules {
namespace cbm {

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
			std::fill_n(Values, nrow*ncol, defaultValue);
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

	private:
		void* Sawtooth_Handle;
		void* Sawtooth_Stand_Handle;
		size_t Sawtooth_Max_Density;
		Sawtooth_Error sawtooth_error;
		Sawtooth_Spatial_Variable spatialVar;
		Sawtooth_CBM_Variable cbmVariables;



		Sawtooth_StandLevelResult standLevelResult;
		Sawtooth_CBMResult cbmResult;
		std::shared_ptr<Sawtooth_CBMAnnualProcesses> annualProcess;

		//sawtooth spatial variables
		moja::flint::IVariable* tmin;
		moja::flint::IVariable* tmean;
		moja::flint::IVariable* vpd;
		moja::flint::IVariable* etr;
		moja::flint::IVariable* eeq;
		moja::flint::IVariable* ws;
		moja::flint::IVariable* ca;
		moja::flint::IVariable* ndep;
		moja::flint::IVariable* ws_mjjas_z;
		moja::flint::IVariable* ws_mjjas_n;
		moja::flint::IVariable* etr_mjjas_z;
		moja::flint::IVariable* etr_mjjas_n;
		moja::flint::IVariable* disturbance;

		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> speciesList;

		SawtoothMatrixWrapper<Sawtooth_Matrix, double> tmin_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> tmean_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> vpd_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> etr_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> eeq_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ws_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ca_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ndep_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ws_mjjas_z_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> ws_mjjas_n_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> etr_mjjas_z_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix, double> etr_mjjas_n_mat;
		SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int> disturbance_mat;

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

		flint::IVariable* _turnoverRates;
		flint::IVariable* _regenDelay;
		flint::IVariable* _currentLandClass;

		flint::IVariable* _isForest;

		flint::IVariable* _species_id;
		flint::IVariable* _standSPUID;

		std::shared_ptr<SoftwoodRootBiomassEquation> SWRootBio;
		std::shared_ptr<HardwoodRootBiomassEquation> HWRootBio;

		Sawtooth_ModelMeta InitializeModelMeta(const DynamicObject& config);

	};





}}}