#ifndef sawtooth_es2_parameter_h
#define sawtooth_es2_parameter_h
#include "equationset.h"
namespace Sawtooth {
	namespace Parameter {
		struct ParameterGrowthES2 {
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
			ParameterGrowthES2() {}
			ParameterGrowthES2(const EquationSet& values) {
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

		struct ParameterMortalityES2 {
			double Int;
			double H1;
			double H2;
			double SBLT;
			double SB;
			double Wz1;
			double Wz2;
			double Wn;
			double Ez1;
			double Ez2;
			double En;
			double H1xWz1;
			double H1xWz2;
			double H2xWz1;
			double H2xWz2;
			double H1xEz1;
			double H1xEz2;
			double H2xEz1;
			double H2xEz2;
			double SBLTxWz1;
			double SBLTxWz2;
			double SBLTxEz1;
			double SBLTxEz2;
			double Wz1xWn;
			double Wz2xWn;
			double Ez1xEn;
			double Ez2xEn;
			double H1_mu;
			double H2_mu;
			double SBLT_mu;
			double SB_mu;
			double Wz1_mu;
			double Wz2_mu;
			double Wn_mu;
			double Ez1_mu;
			double Ez2_mu;
			double En_mu;
			double H1_sig;
			double H2_sig;
			double SBLT_sig;
			double SB_sig;
			double Wz1_sig;
			double Wz2_sig;
			double Wn_sig;
			double Ez1_sig;
			double Ez2_sig;
			double En_sig;
			double BiasAdj;


			ParameterMortalityES2() {}

			ParameterMortalityES2(const EquationSet& values) {
				Int = values.at("Int");
				H1 = values.at("H1");
				H2 = values.at("H2");
				SBLT = values.at("SBLT");
				SB = values.at("SB");
				Wz1 = values.at("Wz1");
				Wz2 = values.at("Wz2");
				Wn = values.at("Wn");
				Ez1 = values.at("Ez1");
				Ez2 = values.at("Ez2");
				En = values.at("En");
				H1xWz1 = values.at("H1xWz1");
				H1xWz2 = values.at("H1xWz2");
				H2xWz1 = values.at("H2xWz1");
				H2xWz2 = values.at("H2xWz2");
				H1xEz1 = values.at("H1xEz1");
				H1xEz2 = values.at("H1xEz2");
				H2xEz1 = values.at("H2xEz1");
				H2xEz2 = values.at("H2xEz2");
				SBLTxWz1 = values.at("SBLTxWz1");
				SBLTxWz2 = values.at("SBLTxWz2");
				SBLTxEz1 = values.at("SBLTxEz1");
				SBLTxEz2 = values.at("SBLTxEz2");
				Wz1xWn = values.at("Wz1xWn");
				Wz2xWn = values.at("Wz2xWn");
				Ez1xEn = values.at("Ez1xEn");
				Ez2xEn = values.at("Ez2xEn");
				H1_mu = values.at("H1_mu");
				H2_mu = values.at("H2_mu");
				SBLT_mu = values.at("SBLT_mu");
				SB_mu = values.at("SB_mu");
				Wz1_mu = values.at("Wz1_mu");
				Wz2_mu = values.at("Wz2_mu");
				Wn_mu = values.at("Wn_mu");
				Ez1_mu = values.at("Ez1_mu");
				Ez2_mu = values.at("Ez2_mu");
				En_mu = values.at("En_mu");
				H1_sig = values.at("H1_sig");
				H2_sig = values.at("H2_sig");
				SBLT_sig = values.at("SBLT_sig");
				SB_sig = values.at("SB_sig");
				Wz1_sig = values.at("Wz1_sig");
				Wz2_sig = values.at("Wz2_sig");
				Wn_sig = values.at("Wn_sig");
				Ez1_sig = values.at("Ez1_sig");
				Ez2_sig = values.at("Ez2_sig");
				En_sig = values.at("En_sig");
				BiasAdj = values.at("BiasAdj");

			}
		};
	}
}
#endif

