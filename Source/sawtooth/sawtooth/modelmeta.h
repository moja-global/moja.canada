#ifndef model_meta_h
#define model_meta_h

namespace Sawtooth {
	namespace Meta {
		enum MortalityModel {
			MortalityNone = 0,
			MortalityConstant = 1,
			MortalityDefault = 2,
			MortalityES1 = 3,
			MortalityES2 = 4,
			MortalityMLR35 = 5
		};

		enum GrowthModel {
			GrowthDefault = 0,
			GrowthES1 = 1,
			GrowthES2 = 2,
			GrowthES3 = 3
		};

		enum RecruitmentModel {
			RecruitmentDefault = 0,
		};

		enum MortalityType {
			None = 0,
			RegularMortality = 1,
			InsectAttack = 2,
			Pathogen = 3,
			SelfThinningMortality = 4,
			Disturbance = 5
		};

		struct ModelMeta {
			MortalityModel mortalityModel;
			GrowthModel growthModel;
			RecruitmentModel recruitmentModel;
		};
	}
}
#endif
