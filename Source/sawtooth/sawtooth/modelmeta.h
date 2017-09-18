#ifndef model_meta_h
#define model_meta_h


enum Sawtooth_MortalityModel {
	Sawtooth_MortalityNone = 0,
	Sawtooth_MortalityConstant = 1,
	Sawtooth_MortalityDefault = 2,
	Sawtooth_MortalityES1 = 3,
	Sawtooth_MortalityES2 = 4,
	Sawtooth_MortalityMLR35 = 5
};

enum Sawtooth_GrowthModel {
	Sawtooth_GrowthDefault = 0,
	Sawtooth_GrowthES1 = 1,
	Sawtooth_GrowthES2 = 2,
	Sawtooth_GrowthES3 = 3
};

enum Sawtooth_RecruitmentModel {
	Sawtooth_RecruitmentDefault = 0,
};

enum Sawtooth_MortalityType {
	Sawtooth_None = 0,
	Sawtooth_RegularMortality = 1,
	Sawtooth_InsectAttack = 2,
	Sawtooth_Pathogen = 3,
	Sawtooth_SelfThinningMortality = 4,
	Sawtooth_Disturbance = 5
};

struct Sawtooth_ModelMeta {
	Sawtooth_MortalityModel mortalityModel;
	Sawtooth_GrowthModel growthModel;
	Sawtooth_RecruitmentModel recruitmentModel;
};

#endif
