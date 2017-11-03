#ifndef sawtooth_default1_param_h
#define sawtooth_default1_param_h
#include "equationset.h"
namespace Sawtooth {
	namespace Parameter {
		struct ParameterGrowthD1 {
			double Int;
			double LnB;
			double B;
			double SA;
			double SBLT;
			double SB;
			double LnB_mu;
			double B_mu;
			double SA_mu;
			double SBLT_mu;
			double SB_mu;
			double LnB_sig;
			double B_sig;
			double SA_sig;
			double SBLT_sig;
			double SB_sig;
			double LogCorrection;

			ParameterGrowthD1() {}
			ParameterGrowthD1(const EquationSet& values) {
				Int = values.at("Int");
				LnB = values.at("LnB");
				B = values.at("B");
				SA = values.at("SA");
				SBLT = values.at("SBLT");
				SB = values.at("SB");
				LnB_mu = values.at("LnB_mu");
				B_mu = values.at("B_mu");
				SA_mu = values.at("SA_mu");
				SBLT_mu = values.at("SBLT_mu");
				SB_mu = values.at("SB_mu");
				LnB_sig = values.at("LnB_sig");
				B_sig = values.at("B_sig");
				SA_sig = values.at("SA_sig");
				SBLT_sig = values.at("SBLT_sig");
				SB_sig = values.at("SB_sig");
				LogCorrection = values.at("LogCorrection");

			}
		};

		struct ParameterRecruitmentD1 {
			double Int;
			double BS;
			double BS_mu;
			double BS_sig;
			ParameterRecruitmentD1() {}
			ParameterRecruitmentD1(const EquationSet& values) {
				Int = values.at("Int");
				BS = values.at("BS");
				BS_mu = values.at("BS_mu");
				BS_sig = values.at("BS_sig");
			}
		};

		struct ParameterMortalityD1 {
			double Int;
			double B;
			double B2;
			double SA;
			double SBLT;
			double SB;
			double B_mu;
			double B2_mu;
			double SA_mu;
			double SBLT_mu;
			double SB_mu;
			double B_sig;
			double B2_sig;
			double SA_sig;
			double SBLT_sig;
			double SB_sig;

			ParameterMortalityD1() {}
			ParameterMortalityD1(const EquationSet& values) {
				Int = values.at("Int");
				B = values.at("B");
				B2 = values.at("B2");
				SA = values.at("SA");
				SBLT = values.at("SBLT");
				SB = values.at("SB");
				B_mu = values.at("B_mu");
				B2_mu = values.at("B2_mu");
				SA_mu = values.at("SA_mu");
				SBLT_mu = values.at("SBLT_mu");
				SB_mu = values.at("SB_mu");
				B_sig = values.at("B_sig");
				B2_sig = values.at("B2_sig");
				SA_sig = values.at("SA_sig");
				SBLT_sig = values.at("SBLT_sig");
				SB_sig = values.at("SB_sig");
			}
		};
	}
}
#endif // !default_param_h

