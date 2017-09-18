#ifndef species_parameter_h
#define species_parameter_h

#include <map>
namespace Sawtooth {
	namespace Parameter {
		struct SpeciesParameter {
			int DeciduousFlag;
			double Cag2H1;
			double Cag2H2;
			double Cag2H3;
			double M_Bave1000;
			double Rec_min;
			double Rec_max;
			double Rec_pN;
			double Rec_pB;
			double Cag2Cf1;
			double Cag2Cf2;
			double Cag2Cbr1;
			double Cag2Cbr2;
			double Cag2Cbk1;
			double Cag2Cbk2;
			SpeciesParameter() {}
			SpeciesParameter(std::map<std::string, double> values) {
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
			}
		};
	}
}
#endif // !species_parameter_h

