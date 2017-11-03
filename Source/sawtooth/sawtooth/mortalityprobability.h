#ifndef sawtooth_mortality_probability_h
#define sawtooth_mortality_probability_h
#include <vector>
namespace Sawtooth {
	struct MortalityProbability
	{
		std::vector<double> P_Regular;
		std::vector<double> P_Insect;
		std::vector<double> P_Pathogen;
		MortalityProbability() { }
		MortalityProbability(size_t size) {
			P_Regular = std::vector<double>(size, 0.0);
			P_Insect = std::vector<double>(size, 0.0);
			P_Pathogen = std::vector<double>(size, 0.0);
		}
	};
}
#endif
