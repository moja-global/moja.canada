#ifndef sawtooth_results_h
#define sawtooth_results_h

#include "sawtoothmatrix.h"

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

//tracks stand level aggregate results by stand (rows) by timestep (cols)
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

struct Sawtooth_CBMBiomassPools {
	Sawtooth_CBMBiomassPools() {
		SWM = 0.0;
		SWF = 0.0;
		SWO = 0.0;
		SWCR = 0.0;
		SWFR = 0.0;
		HWM = 0.0;
		HWF = 0.0;
		HWO = 0.0;
		HWCR = 0.0;
		HWFR = 0.0;
	}
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

Sawtooth_CBMBiomassPools operator+(const Sawtooth_CBMBiomassPools& lh,
	const Sawtooth_CBMBiomassPools& rh);

Sawtooth_CBMBiomassPools operator-(const Sawtooth_CBMBiomassPools& lh,
	const Sawtooth_CBMBiomassPools& rh);

//summary of partitioned flows for a single step
struct Sawtooth_CBMAnnualProcesses {
	//annual gross growth and litter fall
	Sawtooth_CBMBiomassPools NPP;
	//gross growth of live trees
	Sawtooth_CBMBiomassPools GrossGrowth;
	//losses due to litterfalls
	Sawtooth_CBMBiomassPools Litterfall;
	//losses due to annual mortality
	Sawtooth_CBMBiomassPools Mortality;
	//losses due to a prescribed disturbance event
	Sawtooth_CBMBiomassPools Disturbance;
};

//one instance of Sawtooth_CBM_Result is required per stand
struct Sawtooth_CBMResult {
	//one instance of CBMAnnualProcesses per timestep
	Sawtooth_CBMAnnualProcesses* Processes;
};

#endif // 

