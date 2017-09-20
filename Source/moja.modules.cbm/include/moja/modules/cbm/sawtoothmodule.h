
#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/rootbiomassequation.h"

#include "sawtooth/exports.h"

namespace moja {
namespace modules {
namespace cbm {
	class SawtoothStandLevelResultsWrapper;
	class SawtoothTreeLevelResultsWrapper;
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
		std::vector<int> speciesList;
		std::shared_ptr<SawtoothStandLevelResultsWrapper> standLevelResult;
		std::shared_ptr<SawtoothTreeLevelResultsWrapper> treeLevelResults;

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

		flint::IVariable* _cbm_species_id;
		flint::IVariable* _standSPUID;

		std::shared_ptr<SoftwoodRootBiomassEquation> SWRootBio;
		std::shared_ptr<HardwoodRootBiomassEquation> HWRootBio;

		Sawtooth_ModelMeta InitializeModelMeta(const DynamicObject& config);

	};

	class SawtoothMatrixWrapper {
	private:
		std::shared_ptr<Sawtooth_Matrix> mat;
		std::vector<double> Values;
	public:
		SawtoothMatrixWrapper() { }
		SawtoothMatrixWrapper(size_t nrow, size_t ncol, std::vector<double> values)
			: Values(values){
			mat = std::shared_ptr<Sawtooth_Matrix>(new Sawtooth_Matrix);
			mat->cols = ncol;
			mat->rows = nrow;
			mat->values = Values.data();
		}

		Sawtooth_Matrix* Get() {
			return mat.get();
		}

		double GetValue(size_t row, size_t col) {
			return mat->GetValue(row, col);
		}
		void SetValue(size_t row, size_t col, double value) {
			mat->SetValue(row, col, value);
		}
	};
	
	class SawtoothStandLevelResultsWrapper {
	private:
		std::shared_ptr<Sawtooth_StandLevelResult> _results;
		std::map<std::string, SawtoothMatrixWrapper> matrices;

	public:
		SawtoothStandLevelResultsWrapper(size_t nrow, size_t ncol) {
			_results = std::shared_ptr<Sawtooth_StandLevelResult>(new Sawtooth_StandLevelResult);
			matrices["MeanAge"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["MeanHeight"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["StandDensity"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["TotalBiomassCarbon"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["TotalBiomassCarbonGrowth"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["MeanBiomassCarbon"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["RecruitmentRate"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["MortalityRate"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["MortalityCarbon"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["DisturbanceType"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["DisturbanceMortalityRate"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["DisturbanceMortalityCarbon"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
		}
		Sawtooth_StandLevelResult* Get() {
			_results->MeanAge = matrices.at("MeanAge").Get();
			_results->MeanHeight = matrices.at("MeanHeight").Get();
			_results->StandDensity = matrices.at("StandDensity").Get();
			_results->TotalBiomassCarbon = matrices.at("TotalBiomassCarbon").Get();
			_results->TotalBiomassCarbonGrowth = matrices.at("TotalBiomassCarbonGrowth").Get();
			_results->MeanBiomassCarbon = matrices.at("MeanBiomassCarbon").Get();
			_results->RecruitmentRate = matrices.at("RecruitmentRate").Get();
			_results->MortalityRate = matrices.at("MortalityRate").Get();
			_results->MortalityCarbon = matrices.at("MortalityCarbon").Get();
			_results->DisturbanceType = matrices.at("DisturbanceType").Get();
			_results->DisturbanceMortalityRate = matrices.at("DisturbanceMortalityRate").Get();
			_results->DisturbanceMortalityCarbon = matrices.at("DisturbanceMortalityCarbon").Get();
			return _results.get();
		};
	};

	class SawtoothTreeLevelResultsWrapper {
	private:
		std::shared_ptr<Sawtooth_TreeLevelResult> _results;
		std::map<std::string, SawtoothMatrixWrapper> matrices;

	public:
		SawtoothTreeLevelResultsWrapper(size_t nrow, size_t ncol) {
			_results = std::shared_ptr<Sawtooth_TreeLevelResult>(new Sawtooth_TreeLevelResult);
			matrices["Age"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["Height"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["C_AG"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["C_AG_G"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["Live"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["Recruitment"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["Mortality_C_ag"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["Disturbance_C_ag"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
			matrices["DisturbanceType"] = SawtoothMatrixWrapper(nrow, ncol, std::vector<double>(nrow*ncol, 0.0));
		}
		Sawtooth_TreeLevelResult* Get() {
			_results->Age = matrices.at("Age").Get();
			_results->Height = matrices.at("Height").Get();
			_results->C_AG = matrices.at("C_AG").Get();
			_results->C_AG_G = matrices.at("C_AG_G").Get();
			_results->Live = matrices.at("Live").Get();
			_results->Recruitment = matrices.at("Recruitment").Get();
			_results->Mortality_C_ag = matrices.at("Mortality_C_ag").Get();
			_results->MortalityCode = matrices.at("MortalityCode").Get();
			_results->Disturbance_C_ag = matrices.at("Disturbance_C_ag").Get();
			_results->DisturbanceType = matrices.at("DisturbanceType").Get();
			return _results.get();
		};
	};
}}}