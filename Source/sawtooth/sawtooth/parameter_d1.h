#ifndef default1_param_h
#define default1_param_h
#include "equationset.h"
namespace Sawtooth {
	namespace Parameter {
		struct ParameterGrowthD1 {
			double G_Int;
			double G_LnB;
			double G_B;
			double G_AS;
			double G_BS;
			double G_NS;
			double G_LnBxBS;
			double G_LnB_mu;
			double G_B_mu;
			double G_AS_mu;
			double G_BS_mu;
			double G_NS_mu;
			double G_LnB_sig;
			double G_B_sig;
			double G_AS_sig;
			double G_BS_sig;
			double G_NS_sig;
			double G_LogCorrection;
			double G_SBLT_mu;
			double G_SBLT_sig;
			double G_SA;
			double G_SBLT;
			double G_SB;


			ParameterGrowthD1() {}
			ParameterGrowthD1(const EquationSet& values) {
				G_Int = values.at("G_Int");
				G_LnB = values.at("G_LnB");
				G_B = values.at("G_B");
				G_AS = values.at("G_AS");
				G_BS = values.at("G_BS");
				G_NS = values.at("G_NS");
				G_LnBxBS = values.at("G_LnBxBS");
				G_LnB_mu = values.at("G_LnB_mu");
				G_B_mu = values.at("G_B_mu");
				G_AS_mu = values.at("G_AS_mu");
				G_BS_mu = values.at("G_BS_mu");
				G_NS_mu = values.at("G_NS_mu");
				G_LnB_sig = values.at("G_LnB_sig");
				G_B_sig = values.at("G_B_sig");
				G_AS_sig = values.at("G_AS_sig");
				G_BS_sig = values.at("G_BS_sig");
				G_NS_sig = values.at("G_NS_sig");
				G_LogCorrection = values.at("G_LogCorrection");
				G_SBLT_mu = values.at("G_SBLT_mu");
				G_SBLT_sig = values.at("G_SBLT_sig");
				G_SA = values.at("G_SA");
				G_SBLT = values.at("G_SBLT");
				G_SB = values.at("G_SB");
			}
		};

		struct ParameterRecruitmentD1 {
			double R_Int;
			double R_BS;
			double R_AS;
			double R_AS2;
			double R_BS_mu;
			double R_AS_mu;
			double R_AS2_mu;
			double R_BS_sig;
			double R_AS_sig;
			double R_AS2_sig;
			ParameterRecruitmentD1() {}
			ParameterRecruitmentD1(const EquationSet& values) {
				R_Int = values.at("R_Int");
				R_BS = values.at("R_BS");
				R_AS = values.at("R_AS");
				R_AS2 = values.at("R_AS2");
				R_BS_mu = values.at("R_BS_mu");
				R_AS_mu = values.at("R_AS_mu");
				R_AS2_mu = values.at("R_AS2_mu");
				R_BS_sig = values.at("R_BS_sig");
				R_AS_sig = values.at("R_AS_sig");
				R_AS2_sig = values.at("R_AS2_sig");
			}
		};

		struct ParameterMortalityD1 {
			double M_Int;
			double M_B;
			double M_B2;
			double M_AS;
			double M_BS;
			double M_BxBS;
			double M_B_mu;
			double M_B2_mu;
			double M_AS_mu;
			double M_BS_mu;
			double M_B_sig;
			double M_B2_sig;
			double M_AS_sig;
			double M_BS_sig;
			double M_SBLT_mu;
			double M_SBLT_sig;
			double M_SA;
			double M_SBLT;
			double M_SB;

			ParameterMortalityD1() {}
			ParameterMortalityD1(const EquationSet& values) {
				M_Int = values.at("M_Int");
				M_B = values.at("M_B");
				M_B2 = values.at("M_B2");
				M_AS = values.at("M_AS");
				M_BS = values.at("M_BS");
				M_BxBS = values.at("M_BxBS");
				M_B_mu = values.at("M_B_mu");
				M_B2_mu = values.at("M_B2_mu");
				M_AS_mu = values.at("M_AS_mu");
				M_BS_mu = values.at("M_BS_mu");
				M_B_sig = values.at("M_B_sig");
				M_B2_sig = values.at("M_B2_sig");
				M_AS_sig = values.at("M_AS_sig");
				M_BS_sig = values.at("M_BS_sig");
				M_SBLT_mu = values.at("M_SBLT_mu");
				M_SBLT_sig = values.at("M_SBLT_sig");
				M_SA = values.at("M_SA");
				M_SBLT = values.at("M_SBLT");
				M_SB = values.at("M_SB");
			}
		};
	}
}
#endif // !default_param_h

