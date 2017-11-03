#ifndef sawtooth_core_parameter_h
#define sawtooth_core_parameter_h

#include "equationset.h"
namespace Sawtooth {
	namespace Parameter {
		struct ParameterCore {
			int DeciduousFlag;
			double Cag2H1;
			double Cag2H2;
			double Cag2H3;
			double M_Bave1000;
			double Cag2Cf1;
			double Cag2Cf2;
			double Cag2Cbr1;
			double Cag2Cbr2;
			double Cag2Cbk1;
			double Cag2Cbk2;
			double CagMerch_BCC;
			double CagMerch_BCI;
			double CagMerch_AB;
			double CagMerch_SK;
			double CagMerch_MB;
			double CagMerch_ON;
			double CagMerch_QC;
			double CagMerch_NB;
			double CagMerch_NS;
			double CagMerch_PEI;
			double CagMerch_NL;
			double CagMerch_YK;
			double Serotiny;
			double Sprouting;

			ParameterCore() {}
			ParameterCore(const EquationSet& values) {
				DeciduousFlag = (int)values.at("DeciduousFlag");
				Cag2H1 = values.at("Cag2H1");
				Cag2H2 = values.at("Cag2H2");
				Cag2H3 = values.at("Cag2H3");
				M_Bave1000 = values.at("M_Bave1000");
				Cag2Cf1 = values.at("Cag2Cf1");
				Cag2Cf2 = values.at("Cag2Cf2");
				Cag2Cbr1 = values.at("Cag2Cbr1");
				Cag2Cbr2 = values.at("Cag2Cbr2");
				Cag2Cbk1 = values.at("Cag2Cbk1");
				Cag2Cbk2 = values.at("Cag2Cbk2");
				CagMerch_BCC = values.at("CagMerch_BCC");
				CagMerch_BCI = values.at("CagMerch_BCI");
				CagMerch_AB = values.at("CagMerch_AB");
				CagMerch_SK = values.at("CagMerch_SK");
				CagMerch_MB = values.at("CagMerch_MB");
				CagMerch_ON = values.at("CagMerch_ON");
				CagMerch_QC = values.at("CagMerch_QC");
				CagMerch_NB = values.at("CagMerch_NB");
				CagMerch_NS = values.at("CagMerch_NS");
				CagMerch_PEI = values.at("CagMerch_PEI");
				CagMerch_NL = values.at("CagMerch_NL");
				CagMerch_YK = values.at("CagMerch_YK");
				Serotiny = values.at("Serotiny");
				Sprouting = values.at("Sprouting");

			}
		};
	}
}
#endif 

