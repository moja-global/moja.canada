#include "exports.h"
#include <vector>

#define sawtooth_profiling
#ifdef sawtooth_profiling

Sawtooth_Matrix* allocateMatrix(int nrow, int ncol, double value = 0.0) {
	Sawtooth_Matrix* m = new Sawtooth_Matrix;
	m->rows = nrow;
	m->cols = ncol;

	m->values = new double[nrow*ncol];
	std::fill_n(m->values, nrow*ncol, value);
	return m;
}

Sawtooth_Matrix_Int* allocateMatrixInt(int nrow, int ncol, int value = 0) {
	Sawtooth_Matrix_Int* m = new Sawtooth_Matrix_Int;
	m->rows = nrow;
	m->cols = ncol;

	m->values = new int[nrow*ncol];
	std::fill_n(m->values, nrow*ncol, value);
	return m;
}


Sawtooth_StandLevelResult* allocateStandLevelResult(int nStands, int nSteps) {
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
	int nStands = 200;
	int nSteps = 200;
	int nTree = 3500;

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
	
	var.tmin = *allocateMatrix(nStands, nSteps, -10.995);
	var.tmean = *allocateMatrix(nStands, nSteps, 11.425);
	var.vpd = *allocateMatrix(nStands, nSteps, 0.0);
	var.etr = *allocateMatrix(nStands, nSteps, 3.363);
	var.eeq = *allocateMatrix(nStands, nSteps, 1.0);
	var.ws = *allocateMatrix(nStands, nSteps, 141.781);
	var.ca = *allocateMatrix(nStands, nSteps, 346.548);
	var.ndep = *allocateMatrix(nStands, nSteps, 1.672);
	var.ws_mjjas_z = *allocateMatrix(nStands, nSteps, -0.55);
	var.ws_mjjas_n = *allocateMatrix(nStands, 1, 147.87);
	var.etr_mjjas_z = *allocateMatrix(nStands, nSteps, -0.73);
	var.etr_mjjas_n = *allocateMatrix(nStands, 1, 2.09);
	var.disturbances = *allocateMatrixInt(nStands, nSteps, 0);

	Sawtooth_StandLevelResult* s = allocateStandLevelResult(nStands, nSteps);

	Sawtooth_Run(&err, handle, nStands, nSteps, nTree, species, var, NULL, s, NULL, NULL);

	if (err.Code != Sawtooth_NoError) {
		throw std::runtime_error(err.Message);
	}
}
#endif