#ifndef es1_parameter_h
#define es1_parameter_h
#include "equationset.h"
namespace Sawtooth {
	namespace Parameter {
		struct ParameterGrowthES1 {
			double G_Int;
			double G_LnB;
			double G_B;
			double G_AS;
			double G_BS;
			double G_NS;
			double G_Tmin;
			double G_T;
			double G_E;
			double G_W;
			double G_N;
			double G_C;
			double G_LnBxBS;
			double G_LnB_mu;
			double G_B_mu;
			double G_AS_mu;
			double G_BS_mu;
			double G_NS_mu;
			double G_Tmin_mu;
			double G_T_mu;
			double G_E_mu;
			double G_W_mu;
			double G_N_mu;
			double G_C_mu;
			double G_LnB_sig;
			double G_B_sig;
			double G_AS_sig;
			double G_BS_sig;
			double G_NS_sig;
			double G_Tmin_sig;
			double G_T_sig;
			double G_E_sig;
			double G_W_sig;
			double G_N_sig;
			double G_C_sig;
			double G_LogCorrection;
			ParameterGrowthES1() {}
			ParameterGrowthES1(const EquationSet& values) {
				G_Int = values.at("G_Int");
				G_LnB = values.at("G_LnB");
				G_B = values.at("G_B");
				G_AS = values.at("G_AS");
				G_BS = values.at("G_BS");
				G_NS = values.at("G_NS");
				G_Tmin = values.at("G_Tmin");
				G_T = values.at("G_T");
				G_E = values.at("G_E");
				G_W = values.at("G_W");
				G_N = values.at("G_N");
				G_C = values.at("G_C");
				G_LnBxBS = values.at("G_LnBxBS");
				G_LnB_mu = values.at("G_LnB_mu");
				G_B_mu = values.at("G_B_mu");
				G_AS_mu = values.at("G_AS_mu");
				G_BS_mu = values.at("G_BS_mu");
				G_NS_mu = values.at("G_NS_mu");
				G_Tmin_mu = values.at("G_Tmin_mu");
				G_T_mu = values.at("G_T_mu");
				G_E_mu = values.at("G_E_mu");
				G_W_mu = values.at("G_W_mu");
				G_N_mu = values.at("G_N_mu");
				G_C_mu = values.at("G_C_mu");
				G_LnB_sig = values.at("G_LnB_sig");
				G_B_sig = values.at("G_B_sig");
				G_AS_sig = values.at("G_AS_sig");
				G_BS_sig = values.at("G_BS_sig");
				G_NS_sig = values.at("G_NS_sig");
				G_Tmin_sig = values.at("G_Tmin_sig");
				G_T_sig = values.at("G_T_sig");
				G_E_sig = values.at("G_E_sig");
				G_W_sig = values.at("G_W_sig");
				G_N_sig = values.at("G_N_sig");
				G_C_sig = values.at("G_C_sig");
				G_LogCorrection = values.at("G_LogCorrection");
			}
		};

		struct ParameterRecruitmentES1 {
			double R_Int;
			double R_BS;
			double R_AS;
			double R_AS2;
			double R_Tm;
			double R_T;
			double R_E;
			double R_W;
			double R_N;
			double R_C;
			double R_BS_mu;
			double R_AS_mu;
			double R_AS2_mu;
			double R_Tm_mu;
			double R_T_mu;
			double R_E_mu;
			double R_W_mu;
			double R_N_mu;
			double R_C_mu;
			double R_BS_sig;
			double R_AS_sig;
			double R_AS2_sig;
			double R_Tm_sig;
			double R_T_sig;
			double R_E_sig;
			double R_W_sig;
			double R_N_sig;
			double R_C_sig;
			ParameterRecruitmentES1() {}
			ParameterRecruitmentES1(const EquationSet& values) {
				R_Int = values.at("R_Int");
				R_BS = values.at("R_BS");
				R_AS = values.at("R_AS");
				R_AS2 = values.at("R_AS2");
				R_Tm = values.at("R_Tm");
				R_T = values.at("R_T");
				R_E = values.at("R_E");
				R_W = values.at("R_W");
				R_N = values.at("R_N");
				R_C = values.at("R_C");
				R_BS_mu = values.at("R_BS_mu");
				R_AS_mu = values.at("R_AS_mu");
				R_AS2_mu = values.at("R_AS2_mu");
				R_Tm_mu = values.at("R_Tm_mu");
				R_T_mu = values.at("R_T_mu");
				R_E_mu = values.at("R_E_mu");
				R_W_mu = values.at("R_W_mu");
				R_N_mu = values.at("R_N_mu");
				R_C_mu = values.at("R_C_mu");
				R_BS_sig = values.at("R_BS_sig");
				R_AS_sig = values.at("R_AS_sig");
				R_AS2_sig = values.at("R_AS2_sig");
				R_Tm_sig = values.at("R_Tm_sig");
				R_T_sig = values.at("R_T_sig");
				R_E_sig = values.at("R_E_sig");
				R_W_sig = values.at("R_W_sig");
				R_N_sig = values.at("R_N_sig");
				R_C_sig = values.at("R_C_sig");
			}
		};

		struct ParameterMortalityES1 {
			double M_Int;
			double M_B;
			double M_B2;
			double M_AS;
			double M_BS;
			double M_Tm;
			double M_T;
			double M_E;
			double M_W;
			double M_N;
			double M_BxBS;
			double M_B_mu;
			double M_B2_mu;
			double M_AS_mu;
			double M_BS_mu;
			double M_Tm_mu;
			double M_T_mu;
			double M_E_mu;
			double M_W_mu;
			double M_N_mu;
			double M_B_sig;
			double M_B2_sig;
			double M_AS_sig;
			double M_BS_sig;
			double M_Tm_sig;
			double M_T_sig;
			double M_E_sig;
			double M_W_sig;
			double M_N_sig;
			ParameterMortalityES1(){}
			ParameterMortalityES1(const EquationSet& values) {
				M_Int = values.at("M_Int");
				M_B = values.at("M_B");
				M_B2 = values.at("M_B2");
				M_AS = values.at("M_AS");
				M_BS = values.at("M_BS");
				M_Tm = values.at("M_Tm");
				M_T = values.at("M_T");
				M_E = values.at("M_E");
				M_W = values.at("M_W");
				M_N = values.at("M_N");
				M_BxBS = values.at("M_BxBS");
				M_B_mu = values.at("M_B_mu");
				M_B2_mu = values.at("M_B2_mu");
				M_AS_mu = values.at("M_AS_mu");
				M_BS_mu = values.at("M_BS_mu");
				M_Tm_mu = values.at("M_Tm_mu");
				M_T_mu = values.at("M_T_mu");
				M_E_mu = values.at("M_E_mu");
				M_W_mu = values.at("M_W_mu");
				M_N_mu = values.at("M_N_mu");
				M_B_sig = values.at("M_B_sig");
				M_B2_sig = values.at("M_B2_sig");
				M_AS_sig = values.at("M_AS_sig");
				M_BS_sig = values.at("M_BS_sig");
				M_Tm_sig = values.at("M_Tm_sig");
				M_T_sig = values.at("M_T_sig");
				M_E_sig = values.at("M_E_sig");
				M_W_sig = values.at("M_W_sig");
				M_N_sig = values.at("M_N_sig");
			}

		};
	}
}

#endif