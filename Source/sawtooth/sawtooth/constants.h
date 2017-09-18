#ifndef sawtooth_constants_h
#define sawtooth_constants_h
namespace Sawtooth {
namespace Parameter {
	struct Constants
	{
		//maximum growth constraint
		double G_Max;
		//the number of trees on stand initialization
		int Seedling_n;
		//the above ground carbon mean of trees on stand initialization
		double Seedling_mu;
		//the above ground carbon standard deviation of trees on stand initialization
		double Seedling_sig;
		//the above ground carbon minimum of trees on stand initialization
		double Seedling_min;
		//the starting aboveground Carbon of trees established in recruitment
		double RecruitmentC;
		//the starting height of trees established in recruitment
		double RecruitmentH;
		//constant probability of mortality used by the MortalityModel.Constant model
		double Mortality_P_Regular;
		//constant probability of mortality used by the MortalityModel.Constant model
		double Mortality_P_Pathogen;
		//constant probability of mortality used by the MortalityModel.Constant model
		double Mortality_P_Insect;

		Constants() {}
		Constants(std::map<std::string, double> values) {
			G_Max = values.at("G_Max");
			Seedling_n = (int)values.at("Seedling_n");
			Seedling_mu = values.at("Seedling_mu");
			Seedling_sig = values.at("Seedling_sig");
			Seedling_min = values.at("Seedling_min");
			RecruitmentC = values.at("RecruitmentC");
			RecruitmentH = values.at("RecruitmentH");
			Mortality_P_Regular = values.at("Mortality_P_Regular");
			Mortality_P_Pathogen = values.at("Mortality_P_Pathogen");
			Mortality_P_Insect = values.at("Mortality_P_Insect");
		}
	};
}}
#endif 

