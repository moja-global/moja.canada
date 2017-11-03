#ifndef sawtooth_stand_h
#define sawtooth_stand_h

#include <unordered_set>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>
#include "random.h"
#include "modelmeta.h"
#include "results.h"
#include "sawtoothexception.h"

namespace Sawtooth {
	class Stand {

	private:

		bool initialized;

		//area in hectares, does not vary for the lifetime of the stand
		double _area;

		//the species code for the stand
		//does not vary in current implementation, 
		//but plausibly could in the future
		std::vector<int> _species;

		//the set of unique species ids contained in the _species vector
		std::unordered_set<int> _uniqueSpecies;

		//the number of trees in the stand (dead or alive)
		//constant, set on Stand init, and does not change
		int _maxDensity;

		//set from curr_totalC_AG at end of timestep
		double last_totalC_AG;
		//the total C_AG value in the stand
		//modified when:
		//1. stand growth (add sum of growth vector)
		//2. new tree established (add starting C_AG)
		//3. tree killed (deduct C_AG)
		double curr_totalC_AG;

		//the total growth in Carbon since the end of the last step
		double total_C_AG_G;
		
		//the total Carbon lost due to mortality (regular and self-thinning) 
		//since the end of the last step
		double mortality_C_AG;

		//the total Carbon lost due to disturbance since the end of the last 
		//step
		double disturbance_C_AG;

		//set from curr_avg_age at end of timestep
		double last_avg_age;
		//the mean age of live trees in the stand
		//modified when:
		//1. live tree ages are incremented (recompute all samples)
		//2. a new tree is established (add sample to mean)
		//3. a tree is killed (remove sample from mean)
		double curr_avg_age;

		//the mean height of the live trees in the stand
		double mean_height;

		int recruitmentCount;
		int mortalityCount;
		int disturbanceCount;
		//the set of indices that correspond to live trees
		std::unordered_set<int> ilive;

		size_t lastNLive;
		//the set of indices that correspond to live trees
		std::unordered_set<int> idead;

		// Tree age
		std::vector<int> age;

		// Tree height(m)
		std::vector<double> height;

		// Aboveground biomass (kg C tree-1)
		std::vector<double> _C_ag;

		// Aboveground biomass growth (kg C tree-1 year-1) 
		// (reset at the end of a timestep)
		std::vector<double> _C_ag_g;

		// aboveground biomass carbon that was lost to mortality 
		// (regular mortality and self thinning) (kg C tree-1)
		std::vector<double> _mortality_C_ag;

		// aboveground biomass carbon that was lost to mortality 
		// (disturbances) (kg C tree-1)
		std::vector<double> _disturbance_mortality_C_ag;

		std::vector<Sawtooth_MortalityType> mortalityTypes;

		std::vector<bool> recruitment;

		//cbm extension parameter id for top/stump allometry
		int StumpParameterId;
		//cbm extension parameter id for root allometry
		int RootParameterId;
		//cbm extension parameter id for litterfalls
		int TurnoverParameterId;
		//cbm spatial unit id for biomass utilization levels
		int RegionId;

		double MeanSubtract(double currentMean, size_t numSamples,
			double value) const
		{
			if (numSamples <= 1) {
				if (std::abs(currentMean - value) > 0.000001 
					|| numSamples < 1) {
					auto ex = SawtoothException(Sawtooth_StandStateError);
					ex.Message << "mean error";
					throw ex;
				}
				//if there is 1 sample remaining, and it is the sample 
				//we are removing, that's ok, and the new mean is 0
				return 0.0;
			}
			return ((currentMean * numSamples) - value) / (numSamples - 1);
		}
		//add the specified value to to an existing mean which currently has 
		//the specified number of samples and mean
		double MeanAdd(double currentMean, size_t numSamples, double value) const {
			if (numSamples == 0) {
				if(currentMean != 0)
				{
					auto ex = SawtoothException(Sawtooth_StandStateError);
					ex.Message << "mean error";
					throw ex;
				}
				//if this is the first sample the mean is the value
				return value;
			}
			return currentMean + ((value - currentMean) / (numSamples+1));
		}

	public:

		Stand(double area, std::vector<int> speciesCodes, int numTrees,
			int stumpParameterId = -1, int rootParameterId = -1,
			int turnoverParameterId = -1, int regionId = -1);


		//borrowed from https://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes
		template <typename T>
		std::vector<size_t> sort_indexes(const std::vector<T> &v) const {

			// initialize original index locations
			std::vector<size_t> idx(v.size());
			std::iota(idx.begin(), idx.end(), 0);

			// sort indexes based on comparing values in v
			std::sort(idx.begin(), idx.end(),
				[&v](size_t i1, size_t i2) {return v[i1] > v[i2]; });

			return idx;
		}
		//for lack of a better description this is the 
		//conversion of this matlab code:
		//  % Biomass of larger trees(Mg C ha - 1)
		//  B_Larger = zeros(1, n_tree);
		//  sr = sortrows([[1:n_tree]' B'], 2);
		//  sr = flipdim(sr, 1);
		//  sr(:, 3) = (cumsum(sr(:, 2)) - sr(:, 2)). / 1000;
		//  B_Larger(sr(:, 1)) = sr(:, 3)';
		std::vector<double> B_Larger(double scale) const {
			std::vector<double> res(_C_ag.begin(), _C_ag.end());
			std::vector<size_t> sortIndices = sort_indexes(res);

			double t1 = 0.0;
			res[sortIndices[0]] *= scale;
			for (size_t i = 1; i < res.size(); i++) {

				res[sortIndices[i]] *= scale;
				double t2 = t1;
				t1 = res[sortIndices[i]];
				res[sortIndices[i]] = res[sortIndices[i - 1]] + t2;
			}
			res[sortIndices[0]] = 0.0;

			return res;
		}

		//set to true irreversibly for the life time of this stand the moment a
		//tree is established, used so that stands may 
		//persist through multiple calls of lib functions
		bool Initialized() const { return initialized; }

		// get the area of the stand [hectares]
		double Area() const { return _area; }

		int SpeciesId(size_t index) const { return _species[index]; }

		int GetStumpParameterId() const {
			return StumpParameterId;
		}
		int GetRootParameterId() const {
			return RootParameterId;
		}
		int GetTurnoverParameterId() const {
			return TurnoverParameterId;
		}
		int GetRegionId() const {
			return RegionId;
		}

		std::vector<int> UniqueSpecies() const { 
			return std::vector<int>(_uniqueSpecies.begin(),
				_uniqueSpecies.end());
		}

		//get the index of live trees
		std::vector<int> iLive() const { return std::vector<int>(ilive.begin(), ilive.end()); }

		//gets the indexes to live trees matching the specified species code
		std::vector<int> iLive(int speciesCode) const 
		{
			std::vector<int> result;
			std::copy_if(ilive.begin(), ilive.end(), std::back_inserter(result),
				[speciesCode, this](int i) {return speciesCode == _species[i]; });
			return result;
		}

		//gets the indexes to live trees matching the specified set of species code
		std::vector<int> iLive(const std::unordered_set<int>& speciesCodes) const
		{
			std::vector<int> result;
			std::copy_if(ilive.begin(), ilive.end(), std::back_inserter(result),
				[speciesCodes, this](int i) {return speciesCodes.count(_species[i]); });
			return result;
		}


		//get the index of dead trees
		std::vector<int> iDead() const { return std::vector<int>(idead.begin(), idead.end()); }

		//gets the indexes to dead trees matching the specified species code
		std::vector<int> iDead(int speciesCode) const
		{
			std::vector<int> result;
			std::copy_if(idead.begin(), idead.end(), std::back_inserter(result),
				[speciesCode, this](int i) {return speciesCode == _species[i]; });
			return result;
		}

		// *** stand statistics ***

		// get the number of trees (live or dead)
		int MaxDensity() const { return _maxDensity; }
		// Stand age (mean of live tree age)
		double MeanAge(int t = 0) const;
		// Stand density(stems ha - 1)
		double StandDensity() const { return NLive() / _area; }
		//get the number of dead trees
		size_t NDead() const { return idead.size(); }
		//get the number of live trees
		size_t NLive() const { return ilive.size(); }
		// returns the total stand aboveground Carbon for all live trees in the stand
		double Total_C_ag(int t=0) const;
		// returns the mean stand aboveground Carbon for all live trees in the stand
		double Mean_C_ag() const;
		// returns the maximum aboveground Carbon value in the tree population
		double Max_C_ag() const;
		// the total growth in aboveground Carbon since the end of the last step
		double Total_C_ag_g() const { return total_C_AG_G; }
		// the total Carbon lost to mortality (regular and self-thinning) since
		// the end of the last timestep
		double TotalMortality_C_ag() const { return mortality_C_AG; }
		// the total Carbon lost to disturbance since the end of the last timestep
		double TotalDisturbance_C_ag() const { return disturbance_C_AG; }
		// the mean height of live trees in the stand (m)
		double MeanHeight() const { return mean_height; }
		// Demographic recruitment rate (% yr-1)
		double RecruitmentRate() const;
		// Demographic mortality rate(% yr - 1)
		double MortalityRate() const;
		// Disturbance mortality rate(% yr - 1)
		double DisturbanceMortalityRate() const;

		// *** tree specific variables ***

		// the age of the tree at the specified index
		int Age(int tree_index) const { return age[tree_index]; }
		// get the aboveground carbon for the tree specified by tree_index [kg C]
		double C_ag(int tree_index) const { return _C_ag[tree_index]; }
		// get the aboveground carbon growth for the tree specified by 
		// tree_index [kg C year-1]
		double C_ag_g(int tree_index) const { return _C_ag_g[tree_index]; }
		// the Carbon lost due to mortality (regular and self-thinning) for the
		// specified tree
		double Mortality_C_ag(int tree_index) const { return _mortality_C_ag[tree_index]; }
		// the Carbon lost due to disturbance for the specified tree
		double Disturbance_C_ag(int tree_index) const { return _disturbance_mortality_C_ag[tree_index]; }
		// gets the tree height for the specified index
		double Height(int tree_index) const { return height[tree_index]; }
		// returns true if the specified tree_index corresponds to a live tree
		bool IsLive(int tree_index) const;
		Sawtooth_MortalityType GetMortalityType(int tree_index) const { return mortalityTypes[tree_index]; }
		bool GetRecruitmentState(int tree_index) const { return recruitment[tree_index]; }

		// *** stand altering functions ***

		// kills the tree specified by tree_index
		void KillTree(int tree_index, Sawtooth_MortalityType mtype);
		// kills all remaining trees in the stand
		void KillAllTrees(Sawtooth_MortalityType mtype);
		// establishes a tree at the specified tree index
		void EstablishTree(int tree_index, double initial_C_ag, double initial_height);
		//increment the age of all live trees in the stand
		void IncrementAge();
		// adds the specified growth increment to the stand's above ground 
		// biomass 
		void IncrementAgBiomass(std::vector<double> C_ag_G);
		// sets the height of all trees in the stand
		void SetTreeHeight(std::vector<double> treeHeight);

		//shifts variables to the t-1 position, initializes new variables for new timestep
		void EndStep();
	};
}
#endif // !stand_h
