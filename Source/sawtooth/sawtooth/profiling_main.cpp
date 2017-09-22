#include "exports.h"
#include <vector>

#define sawtooth_profiling 1
#ifdef sawtooth_profiling

template<typename T>
T** allocateVariable(size_t xSize, size_t ySize, T value)
{
	T** val = new T*[xSize];
	for (size_t i = 0; i < xSize; i++) {
		val[i] = new T[ySize];
		for (size_t j = 0; j < ySize; j++) {
			val[i][j] = value;
		}
	}
		
	return val;

}

template<typename T>
T* allocateVariable(size_t size, T value)
{
	T* val = new T[size];
	for (size_t i = 0; i < size; i++) {
		val[i] = value;
	}
	return val;

}


Sawtooth_Matrix* allocateMatrix(int nStands, int nSteps) {
	Sawtooth_Matrix* m = new Sawtooth_Matrix[1];
	m->cols = nSteps;
	m->rows = nStands;
	m->values = new double[nStands*nSteps];
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
	char* dbpath = "M:\\Sawtooth\\Code\\cppVersion\\sqliteDb\\SawtoothParameters.db";
	int nStands = 200;
	int nSteps = 200;
	int nTree = 3500;

	Sawtooth_ModelMeta meta;
	meta.CBMEnabled = false;
	meta.growthModel = Sawtooth_GrowthDefault;
	meta.mortalityModel = Sawtooth_MortalityES2;
	meta.recruitmentModel = Sawtooth_RecruitmentDefault;

	void* handle = Sawtooth_Initialize(&err, dbpath, meta, 1);

	int** species = allocateVariable(nStands, nTree, 54);
	double** tmin = allocateVariable(nStands, nSteps, -10.995);
	double** tmean = allocateVariable(nStands, nSteps, 11.425);
	double** vpd = allocateVariable(nStands, nSteps, 0.0);
	double** etr = allocateVariable(nStands, nSteps, 3.363);
	double** eeq = allocateVariable(nStands, nSteps, 1.0);
	double** ws = allocateVariable(nStands, nSteps, 141.781);
	double** ca = allocateVariable(nStands, nSteps, 346.548);
	double** ndep = allocateVariable(nStands, nSteps, 1.672);
	double** ws_mjjas_z = allocateVariable(nStands, nSteps, -0.55);
	double* ws_mjjas_n = allocateVariable(nStands, 147.87);
	double** etr_mjjas_z = allocateVariable(nStands, nSteps, -0.73);
	double* etr_mjjas_n = allocateVariable(nStands, 2.09);
	int** disturbances = allocateVariable(nStands, nSteps, 0);
	Sawtooth_StandLevelResult* s = allocateStandLevelResult(nStands, nSteps);

	Sawtooth_Run(&err, handle, nStands, nSteps, nTree, species,
		tmin, tmean, vpd, etr, eeq, ws, ca, ndep, 
		ws_mjjas_z, ws_mjjas_n, etr_mjjas_z, etr_mjjas_n, disturbances,
		NULL, NULL, NULL, NULL, s, NULL, NULL);
}
#endif