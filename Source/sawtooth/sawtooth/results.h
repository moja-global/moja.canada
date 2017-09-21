#ifndef results_h
#define results_h


struct Sawtooth_Matrix {
	//number of rows in the matrix
	size_t rows;
	//number of columns in the matrix
	size_t cols;
	//matrix storage
	double* values;
	//total number of elements in storage
	size_t size() { return rows*cols; }

	double GetValue(size_t row, size_t col) {
		return values[row * cols + col];
	}

	void SetValue(size_t row, size_t col, double value) {
		values[row * cols + col] = value;
	}
};

//tracks tree level results by timestep, by tree index
//one TreeLevelResult struct instance is required per stand
//all matrices are by step number (rows), by tree index (cols)
struct Sawtooth_TreeLevelResult {
	//age (years)
	Sawtooth_Matrix* Age;
	//tree height (m)
	Sawtooth_Matrix* Height;
	//aboveground live Carbon (kg C)
	Sawtooth_Matrix* C_AG;
	//aboveground Carbon growth (kg C yr-1)
	Sawtooth_Matrix* C_AG_G;
	//live 1, or dead 0
	Sawtooth_Matrix* Live;
	//recruitment (if value=1, otherwise 0)
	Sawtooth_Matrix* Recruitment;
	//aboveground live Carbon losses due to annual mortality, and 
	//self-thinning (kg C)
	Sawtooth_Matrix* Mortality_C_ag;
	//regular or thinning mortality (if value=1, otherwise 0)
	Sawtooth_Matrix* MortalityCode;
	//aboveground live Carbon losses due to disturbance (kg C)
	Sawtooth_Matrix* Disturbance_C_ag;
	//the disturbance type that occurred for specified trees/timesteps
	Sawtooth_Matrix* DisturbanceType;
};

//tracks stand level aggregate results by timestep (rows) by stand (cols)
//one struct instance is required for the entire simulation space 
struct Sawtooth_StandLevelResult {
	Sawtooth_Matrix* MeanAge;
	Sawtooth_Matrix* MeanHeight;
	// stand density (stems ha^-1)
	Sawtooth_Matrix* StandDensity;
	// total biomass carbon (Mg C ha-1)
	Sawtooth_Matrix* TotalBiomassCarbon;
	// total biomass carbon growth (Mg C ha-1 yr-1)
	Sawtooth_Matrix* TotalBiomassCarbonGrowth;
	// mean biomass carbon (kg C)
	Sawtooth_Matrix* MeanBiomassCarbon;
	// Demographic recruitment rate (% yr-1)
	Sawtooth_Matrix* RecruitmentRate;
	// Demographic mortality rate (% yr-1) (regular mortality and thinning)
	Sawtooth_Matrix* MortalityRate;
	// Total mortality Carbon (Mg C ha-1 yr-1) (regular mortality and thinning)
	Sawtooth_Matrix* MortalityCarbon;
	// Disturbance type code 
	Sawtooth_Matrix* DisturbanceType;
	// Demographic mortality rate (% yr-1) (disturbance)
	Sawtooth_Matrix* DisturbanceMortalityRate;
	// Total disturbance mortality (Mg C ha-1 yr-1)
	Sawtooth_Matrix* DisturbanceMortalityCarbon;
};

struct CBMBiomassPools {
	double SWM;
	double SWF;
	double SWO;
	double SWCR;
	double SWFR;
	double HWM;
	double HWF;
	double HWO;
	double HWCR;
	double HWFR;
};

//summary of partitioned flows for a single step
struct CBMAnnualProcesses {
	//total annual growth including litter fall (NPP)
	CBMBiomassPools GrossGrowth;
	//the annual net change in stand biomass (equal to 
	//GrossGrowth - Litterfall)
	CBMBiomassPools NetGrowth;
	//losses due to litterfalls
	CBMBiomassPools Litterfall;
	//losses due to annual mortality
	CBMBiomassPools Mortality;
	//losses due to a prescribed disturbance event
	CBMBiomassPools Disturbance;
};

//one instance of Sawtooth_CBM_Result is required per stand
struct Sawtooth_CBM_Result {
	//one instance of CBMAnnualProcesses per timestep
	CBMAnnualProcesses* processes;
};

#endif // 

