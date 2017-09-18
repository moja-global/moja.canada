#include "stand.h"
#include "sawtoothexception.h"
namespace Sawtooth {

	Stand::Stand(double area, std::vector<int> speciesCodes, int maxDensity) {
		initialized = false;
		_area = area;
		_species = speciesCodes;
		_uniqueSpecies = std::unordered_set<int>(speciesCodes.begin(), speciesCodes.end());
		last_totalC_AG = 0.0;
		curr_totalC_AG = 0.0;
		total_C_AG_G = 0.0;
		mortality_C_AG = 0.0;
		disturbance_C_AG = 0.0;
		_maxDensity = maxDensity;
		last_avg_age = 0.0;
		curr_avg_age = 0.0;
		lastNLive = 0;
		recruitmentCount = 0;
		mortalityCount = 0;
		disturbanceCount = 0;

		for (int i = 0; i < maxDensity; i++) {
			idead.insert(i);
		}
		age = std::vector<int>(maxDensity, 0);
		height = std::vector<double>(maxDensity, 0.0);
		_C_ag = std::vector<double>(maxDensity, 0.0);
		_C_ag_g = std::vector<double>(maxDensity, 0.0);
		_mortality_C_ag = std::vector<double>(maxDensity, 0.0);
		_disturbance_mortality_C_ag = std::vector<double>(maxDensity, 0.0);
		mortalityTypes = std::vector<Meta::MortalityType>(maxDensity, Meta::None);
		recruitment = std::vector<bool>(maxDensity, false);
	}

	// returns the total stand aboveground Carbon for all live trees in the stand
	// kg C
	double Stand::Total_C_ag(int t) const {
		if (t == 0) {
			return curr_totalC_AG;
		}
		else if (t == -1) {
			return last_totalC_AG;
		}
		else {
			throw SawtoothException(Sawtooth_StandArgumentError, "Total_C_ag t must be either 0, or -1");
		}
	}
	
	// returns the mean stand aboveground Carbon for all live trees in the 
	// stand (excludes dead trees)
	// kg C
	double Stand::Mean_C_ag() const {
		size_t n = NLive();
		if (n > 0) {
			return Total_C_ag(0) / n;
		}
		return 0.0;
	}
	
	// returns the maximum aboveground Carbon value in the tree population
	// kg C
	double Stand::Max_C_ag() const {
		double max = 0.0;
		for (auto il : ilive) {
			double curr = _C_ag[il];
			if (curr > max) {
				max = curr;
			}
		}
		return max;
	}

	// Stand age (mean of live tree's ages)
	double Stand::MeanAge(int t) const {
		if (t == 0) {
			return curr_avg_age;
		}
		else if (t == -1) {
			return last_avg_age;
		}
		else {
			throw SawtoothException(Sawtooth_StandArgumentError, "MeanAge(t) t must be either 0, or -1");
		}
	}

	double Stand::RecruitmentRate() const {
		double l = lastNLive;
		if (l == 0) {
			return 0;
		}
		else {
			return recruitmentCount * 100.0 / l;
		}
	}

	double Stand::MortalityRate() const {
		double l = lastNLive;
		if (l == 0) {
			return 0;
		}
		else {
			return mortalityCount * 100.0 / l;
		}
	}

	double Stand::DisturbanceMortalityRate() const {
		double l = lastNLive;
		if (l == 0) {
			return 0;
		}
		else {
			return disturbanceCount * 100.0 / l;
		}
	}

	bool Stand::IsLive(int tree_index) const {
		size_t li = ilive.count(tree_index);
		size_t di = idead.count(tree_index);
		if (li == di) {
			//error state, tree_index is specified as both live or both dead
			throw SawtoothException(Sawtooth_StandStateError,
				"invalid ilive or idead state variables");
		}
		else {
			return li == 1;
		}
	}

	// kills the tree specified by tree_index
	void Stand::KillTree(int tree_index, Meta::MortalityType mtype) {
		if (ilive.count(tree_index) == 0) {
			//sanity check
			throw SawtoothException(Sawtooth_StandArgumentError,
				"KillTree: specified index is already set to dead");
		}
		//compute the new mean age by removing the dying tree's contribution
		curr_avg_age = MeanSubtract(curr_avg_age, NLive(), age[tree_index]);
		mean_height = MeanSubtract(mean_height, NLive(), height[tree_index]);
		age[tree_index] = 0;
		height[tree_index] = 0.0;
		//deduct the lost carbon from the current total
		double lost_C_ag = _C_ag[tree_index];
		if (mtype == Meta::SelfThinningMortality ||
			mtype == Meta::RegularMortality) {
			_mortality_C_ag[tree_index] += lost_C_ag;
			mortality_C_AG += lost_C_ag;
			mortalityCount++;
		}
		if (mtype == Meta::Disturbance) {
			_disturbance_mortality_C_ag[tree_index] = lost_C_ag;
			disturbance_C_AG += lost_C_ag;
			disturbanceCount++;
		}
		curr_totalC_AG -= lost_C_ag;
		_C_ag[tree_index] = 0.0;
		mortalityTypes[tree_index] = mtype;
		if (ilive.erase(tree_index) != 1) {
			throw SawtoothException(Sawtooth_StandStateError,
				"attempted to erase tree_index not found in ilive set");
		}
		if (!idead.insert(tree_index).second) {
			throw SawtoothException(Sawtooth_StandStateError,
				"attempted to insert tree_index already in idead set");
		}
	}

	// kills all live trees in the stand
	void Stand::KillAllTrees(Meta::MortalityType mtype) {

		//independent sequence required because of modification in-loop
		std::vector<int> indices(ilive.begin(), ilive.end());

		for (auto tree_index : indices) {
			KillTree(tree_index, mtype);
		}
		//reset statistics
		curr_avg_age = 0.0;
		curr_totalC_AG = 0.0;
		mean_height = 0.0;
		if (NLive() != 0 && NDead() != _maxDensity) {
			throw SawtoothException(Sawtooth_StandStateError,
				"invalid ilive or idead state variables");
		}
	}

	// establishes a tree at the specified tree index
	void Stand::EstablishTree(int tree_index, double initial_C_ag,
		double initial_height) {
		initialized = true;
		if (ilive.count(tree_index) == 1) {
			//sanity check
			throw SawtoothException(Sawtooth_StandArgumentError,
				"EstablishTree: specified index is already set to live");
		}
		_C_ag[tree_index] = initial_C_ag;
		_C_ag_g[tree_index] += initial_C_ag;
		curr_totalC_AG += _C_ag[tree_index];
		total_C_AG_G += initial_C_ag;
		height[tree_index] = initial_height;
		mean_height = MeanAdd(mean_height, NLive(), initial_height);
		age[tree_index] = 1;
		//update the current mean age using an online mean
		curr_avg_age = MeanAdd(curr_avg_age, NLive(), age[tree_index]);
		recruitment[tree_index] = true;
		recruitmentCount++;
		if (idead.erase(tree_index) != 1) {
			throw SawtoothException(Sawtooth_StandStateError,
				"attempted to erase tree_index not found in idead set");
		}
		if (!ilive.insert(tree_index).second) {
			throw SawtoothException(Sawtooth_StandStateError,
				"attempted to insert tree_index already in ilive set");
		}
	}

	//increment the age of all live trees in the stand
	void Stand::IncrementAge() {
		int nlive = NLive();
		if (nlive == 0) {
			curr_avg_age = 0;
			return;
		}
		double sum = 0;
		for (auto liveIndex : ilive) {
			age[liveIndex] ++;
			sum += age[liveIndex];
		}
		curr_avg_age = sum / nlive;
	}

	// adds the specified growth increment to the stand's above ground 
	// biomass 
	void Stand::IncrementAgBiomass(std::vector<double> C_ag_G)
	{
		if (_C_ag.size() != C_ag_G.size()) {
			throw SawtoothException(Sawtooth_StandArgumentError, "Carbon vectors of unequal size");
		}
		for (auto liveIndex : ilive) {
			double c = C_ag_G[liveIndex];
			curr_totalC_AG += c;
			_C_ag[liveIndex] += c;
			_C_ag_g[liveIndex] += c;
			total_C_AG_G += c;
		}
	}

	// sets the height of all trees in the stand
	void Stand::SetTreeHeight(std::vector<double> treeHeight)
	{
		if (height.size() != treeHeight.size()) {
			throw SawtoothException(Sawtooth_StandArgumentError, "tree height vectors of unequal size");
		}
		int nlive = NLive();
		if (nlive == 0) {
			mean_height = 0;
			return;
		}
		height = treeHeight;
		double sum = 0;
		for (auto li : ilive) {
			//non-live indices are ignored
			sum += treeHeight[li];
		}
		mean_height = sum / nlive;
	}

	void Stand::EndStep()
	{
		recruitmentCount = 0;
		mortalityCount = 0;
		disturbanceCount = 0;
		last_totalC_AG = curr_totalC_AG;
		total_C_AG_G = 0.0;
		last_avg_age = curr_avg_age;
		mortality_C_AG = 0.0;
		disturbance_C_AG = 0.0;
		lastNLive = NLive();
		//reset the growth vector to 0
		std::fill(_C_ag_g.begin(), _C_ag_g.end(), 0);
		std::fill(mortalityTypes.begin(), mortalityTypes.end(), Meta::None);
		std::fill(recruitment.begin(), recruitment.end(), false);
		std::fill(_mortality_C_ag.begin(), _mortality_C_ag.end(), 0.0);
		std::fill(_disturbance_mortality_C_ag.begin(), 
			_disturbance_mortality_C_ag.end(), 0.0);
	}
}