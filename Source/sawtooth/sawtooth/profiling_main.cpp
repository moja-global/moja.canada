#include "exports.h"
#include <vector>

#define sawtooth_profiling
#ifdef sawtooth_profiling


#define sawtooth_cbm_extension_enabled

Sawtooth_Matrix* allocateMatrix(size_t nrow, size_t ncol, double value = 0.0) {
	Sawtooth_Matrix* m = new Sawtooth_Matrix;
	m->rows = nrow;
	m->cols = ncol;

	m->values = new double[nrow*ncol];
	std::fill_n(m->values, nrow*ncol, value);
	return m;
}

Sawtooth_Matrix_Int* allocateMatrixInt(size_t nrow, size_t ncol, int value = 0) {
	Sawtooth_Matrix_Int* m = new Sawtooth_Matrix_Int;
	m->rows = nrow;
	m->cols = ncol;

	m->values = new int[nrow*ncol];
	std::fill_n(m->values, nrow*ncol, value);
	return m;
}


Sawtooth_StandLevelResult* allocateStandLevelResult(size_t nStands, size_t nSteps) {
	Sawtooth_StandLevelResult* s = new Sawtooth_StandLevelResult[1];
	s->MeanAge = allocateMatrix(nStands, nSteps);
	s->MeanHeight = allocateMatrix(nStands, nSteps);
	s->StandDensity = allocateMatrix(nStands, nSteps);
	s->TotalBiomassCarbon = allocateMatrix(nStands, nSteps);
	s->TotalBiomassCarbonGrowth = allocateMatrix(nStands, nSteps);
	s->MeanBiomassCarbon = allocateMatrix(nStands, nSteps);
	s->RecruitmentRate = allocateMatrix(nStands, nSteps);
	s->MortalityRate = allocateMatrix(nStands, nSteps);
	s->MortalityCarbon = allocateMatrix(nStands, nSteps);
	s->DisturbanceType = allocateMatrix(nStands, nSteps);
	s->DisturbanceMortalityRate = allocateMatrix(nStands, nSteps);
	s->DisturbanceMortalityCarbon = allocateMatrix(nStands, nSteps);
	return s;
}

int main(char argc, char** argv) {

	Sawtooth_Error err;
	char* dbpath = "M:\\Sawtooth\\Parameters\\SawtoothParameters.db";
	size_t nStands = 200;
	size_t nSteps = 200;
	size_t nTree = 3500;

	Sawtooth_ModelMeta meta;
	meta.CBMEnabled = false;
	meta.growthModel = Sawtooth_GrowthD1;
	meta.mortalityModel = Sawtooth_MortalityES2;
	meta.recruitmentModel = Sawtooth_RecruitmentD1;

	void* handle = Sawtooth_Initialize(&err, dbpath, meta, 1);
	if (err.Code != Sawtooth_NoError) {
		throw std::runtime_error(err.Message);
	
	}
	Sawtooth_Matrix_Int species = *allocateMatrixInt(nStands, nTree, 54);

	Sawtooth_Spatial_Variable var;
	
	var.tmin_ann = *allocateMatrix(nStands, nSteps, -10.995);
	var.tmean_gs = *allocateMatrix(nStands, nSteps, 11.425);
	var.vpd = *allocateMatrix(nStands, nSteps, 0.0);
	var.etp_gs = *allocateMatrix(nStands, nSteps, 3.363);
	var.eeq = *allocateMatrix(nStands, nSteps, 1.0);
	var.ws_gs = *allocateMatrix(nStands, nSteps, 141.781);
	var.ca = *allocateMatrix(nStands, nSteps, 346.548);
	var.ndep = *allocateMatrix(nStands, nSteps, 1.672);
	var.ws_gs_z = *allocateMatrix(nStands, nSteps, -0.55);
	var.ws_gs_n = *allocateMatrix(nStands, 1, 147.87);
	var.etp_gs_z = *allocateMatrix(nStands, nSteps, -0.73);
	var.etp_gs_n = *allocateMatrix(nStands, 1, 2.09);
	var.disturbances = *allocateMatrixInt(nStands, nSteps, 0);

	Sawtooth_StandLevelResult* s = allocateStandLevelResult(nStands, nSteps);

	Sawtooth_CBM_Variable* cbmVar = NULL;
	Sawtooth_CBMResult* cbmRes = NULL;
#ifdef sawtooth_cbm_extension_enabled
	meta.CBMEnabled = true;
	cbmVar = new Sawtooth_CBM_Variable;
	cbmVar->RegionId = *allocateMatrixInt(nStands, 1, 42);
	cbmVar->RootParameterId = *allocateMatrixInt(nStands, 1, 1);
	cbmVar->StumpParameterId = *allocateMatrixInt(nStands, 1, 1);
	cbmVar->TurnoverParameterId = *allocateMatrixInt(nStands, 1, 1);

	cbmRes = new Sawtooth_CBMResult[nStands];
	for (size_t i = 0; i < nStands; i++) {
		cbmRes[i].Processes = new Sawtooth_CBMAnnualProcesses[nSteps];
	}
#endif 


	Sawtooth_Run(&err, handle, nStands, nSteps, nTree, species, var, cbmVar, s, NULL, cbmRes);

	if (err.Code != Sawtooth_NoError) {
		throw std::runtime_error(err.Message);
	}
}
#endif