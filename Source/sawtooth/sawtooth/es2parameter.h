#ifndef es2_parameter_h
#define es2_parameter_h

namespace Sawtooth {
	namespace Parameter {
		struct ES2GrowthParameter {
			double G_Int;
			double G_LnB;
			double G_B;
			double G_AS;
			double G_BS;
			double G_NS;
			double G_Tm;
			double G_T;
			double G_E;
			double G_W;
			double G_N;
			double G_C;
			double G_NxT;
			double G_NxT2;
			double G_NxE;
			double G_NxE2;
			double G_NxW;
			double G_NxW2;
			double G_CxT;
			double G_CxT2;
			double G_CxE;
			double G_CxE2;
			double G_CxW;
			double G_CxW2;
			double G_CxN;
			double G_CxN2;
			double G_LogCorrection;
			double G_LnB_mu;
			double G_B_mu;
			double G_AS_mu;
			double G_BS_mu;
			double G_NS_mu;
			double G_Tm_mu;
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
			double G_Tm_sig;
			double G_T_sig;
			double G_E_sig;
			double G_W_sig;
			double G_N_sig;
			double G_C_sig;
			ES2GrowthParameter() {}
			ES2GrowthParameter(std::map<std::string, double> values) {
				G_Int = values.at("G_Int");
				G_LnB = values.at("G_LnB");
				G_B = values.at("G_B");
				G_AS = values.at("G_AS");
				G_BS = values.at("G_BS");
				G_NS = values.at("G_NS");
				G_Tm = values.at("G_Tm");
				G_T = values.at("G_T");
				G_E = values.at("G_E");
				G_W = values.at("G_W");
				G_N = values.at("G_N");
				G_C = values.at("G_C");
				G_NxT = values.at("G_NxT");
				G_NxT2 = values.at("G_NxT2");
				G_NxE = values.at("G_NxE");
				G_NxE2 = values.at("G_NxE2");
				G_NxW = values.at("G_NxW");
				G_NxW2 = values.at("G_NxW2");
				G_CxT = values.at("G_CxT");
				G_CxT2 = values.at("G_CxT2");
				G_CxE = values.at("G_CxE");
				G_CxE2 = values.at("G_CxE2");
				G_CxW = values.at("G_CxW");
				G_CxW2 = values.at("G_CxW2");
				G_CxN = values.at("G_CxN");
				G_CxN2 = values.at("G_CxN2");
				G_LogCorrection = values.at("G_LogCorrection");
				G_LnB_mu = values.at("G_LnB_mu");
				G_B_mu = values.at("G_B_mu");
				G_AS_mu = values.at("G_AS_mu");
				G_BS_mu = values.at("G_BS_mu");
				G_NS_mu = values.at("G_NS_mu");
				G_Tm_mu = values.at("G_Tm_mu");
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
				G_Tm_sig = values.at("G_Tm_sig");
				G_T_sig = values.at("G_T_sig");
				G_E_sig = values.at("G_E_sig");
				G_W_sig = values.at("G_W_sig");
				G_N_sig = values.at("G_N_sig");
				G_C_sig = values.at("G_C_sig");
			}
		};

		struct ES2MortalityParameter {
			double M_Int;
			double M_H1;
			double M_H2;
			double M_CI;
			double M_W1;
			double M_W2;
			double M_E1;
			double M_E2;
			double M_H1xW1;
			double M_H1xW2;
			double M_H2xW1;
			double M_H2xW2;
			double M_H1xE1;
			double M_H1xE2;
			double M_H2xE1;
			double M_H2xE2;
			double M_NWxW1;
			double M_NWxW2;
			double M_NExE1;
			double M_NExE2;
			double M_H1_mu;
			double M_H2_mu;
			double M_CI_mu;
			double M_W1_mu;
			double M_W2_mu;
			double M_E1_mu;
			double M_E2_mu;
			double M_NW_mu;
			double M_NE_mu;
			double M_H1_sig;
			double M_H2_sig;
			double M_CI_sig;
			double M_W1_sig;
			double M_W2_sig;
			double M_E1_sig;
			double M_E2_sig;
			double M_NW_sig;
			double M_NE_sig;
			double M_BiasAdj;


			ES2MortalityParameter() {}

			ES2MortalityParameter(std::map<std::string, double> values) {
				M_Int = values.at("M_Int");
				M_H1 = values.at("M_H1");
				M_H2 = values.at("M_H2");
				M_CI = values.at("M_CI");
				M_W1 = values.at("M_W1");
				M_W2 = values.at("M_W2");
				M_E1 = values.at("M_E1");
				M_E2 = values.at("M_E2");
				M_H1xW1 = values.at("M_H1xW1");
				M_H1xW2 = values.at("M_H1xW2");
				M_H2xW1 = values.at("M_H2xW1");
				M_H2xW2 = values.at("M_H2xW2");
				M_H1xE1 = values.at("M_H1xE1");
				M_H1xE2 = values.at("M_H1xE2");
				M_H2xE1 = values.at("M_H2xE1");
				M_H2xE2 = values.at("M_H2xE2");
				M_NWxW1 = values.at("M_NWxW1");
				M_NWxW2 = values.at("M_NWxW2");
				M_NExE1 = values.at("M_NExE1");
				M_NExE2 = values.at("M_NExE2");
				M_H1_mu = values.at("M_H1_mu");
				M_H2_mu = values.at("M_H2_mu");
				M_CI_mu = values.at("M_CI_mu");
				M_W1_mu = values.at("M_W1_mu");
				M_W2_mu = values.at("M_W2_mu");
				M_E1_mu = values.at("M_E1_mu");
				M_E2_mu = values.at("M_E2_mu");
				M_NW_mu = values.at("M_NW_mu");
				M_NE_mu = values.at("M_NE_mu");
				M_H1_sig = values.at("M_H1_sig");
				M_H2_sig = values.at("M_H2_sig");
				M_CI_sig = values.at("M_CI_sig");
				M_W1_sig = values.at("M_W1_sig");
				M_W2_sig = values.at("M_W2_sig");
				M_E1_sig = values.at("M_E1_sig");
				M_E2_sig = values.at("M_E2_sig");
				M_NW_sig = values.at("M_NW_sig");
				M_NE_sig = values.at("M_NE_sig");
				M_BiasAdj = values.at("M_BiasAdj");

			}
		};
	}
}
#endif

