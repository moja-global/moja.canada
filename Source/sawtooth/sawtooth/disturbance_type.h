#ifndef sawtooth_disturbance_type_h
#define sawtooth_disturbance_type_h
#include <vector>
namespace Sawtooth {
namespace Parameter {
	class DisturbanceType {
	private:
		int Id;
		double _p_mortality;
		std::vector<int> EligibleSpecies;
	public:
		DisturbanceType(int id, double p_mortality, std::vector<int>& eligibleSpecies)
			: Id(id), _p_mortality(p_mortality), EligibleSpecies(eligibleSpecies) { }

		bool HasFilter() const { return EligibleSpecies.size() > 0; }

		double P_Mortality() const { return _p_mortality; }

		bool IsFiltered(int species_code) const {
			if (!HasFilter()) {
				return false;
			}
			return std::find(
					EligibleSpecies.begin(),
					EligibleSpecies.end(), 
					species_code) != EligibleSpecies.end();
		}

	};
}}
#endif