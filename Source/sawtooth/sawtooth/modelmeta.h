#ifndef sawtooth_model_meta_h
#define sawtooth_model_meta_h


enum Sawtooth_MortalityModel {
	Sawtooth_MortalityNone = 0,
	Sawtooth_MortalityConstant = 1,
	Sawtooth_MortalityD1 = 2,
	Sawtooth_MortalityD2 = 3,
	Sawtooth_MortalityES1 = 4,
	Sawtooth_MortalityES2 = 5,
	Sawtooth_MortalityMLR35 = 6
};

enum Sawtooth_GrowthModel {
	Sawtooth_GrowthD1 = 0,
	Sawtooth_GrowthD2 = 1,
	Sawtooth_GrowthES1 = 2,
	Sawtooth_GrowthES2 = 3,
	Sawtooth_GrowthES3 = 4
};

enum Sawtooth_RecruitmentModel {
	Sawtooth_RecruitmentD1 = 0,
	Sawtooth_RecruitmentD2 = 1
};

enum Sawtooth_MortalityType {
	Sawtooth_None = 0,
	Sawtooth_RegularMortality = 1,
	Sawtooth_InsectAttack = 2,
	Sawtooth_Pathogen = 3,
	Sawtooth_Disturbance = 5
};

struct Sawtooth_ModelMeta {
	int CBMEnabled;
	Sawtooth_MortalityModel mortalityModel;
	Sawtooth_GrowthModel growthModel;
	Sawtooth_RecruitmentModel recruitmentModel;
};

#endif
