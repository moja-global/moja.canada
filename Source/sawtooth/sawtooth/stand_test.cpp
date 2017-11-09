//#define runStandTests 1
#ifdef runStandTests
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"
#include "stand.h"
#include "modelmeta.h"
#include <valarray>

size_t countVec(std::vector<int> v, int value) {
	return std::count(v.begin(), v.end(), value);
}
TEST_CASE("Stand init") {
	double area = 3.0;

	int numTrees = 10;
	auto speciesID = std::vector<int>(numTrees, 2);
	Sawtooth::Stand s(area, speciesID, numTrees);
	REQUIRE(s.Initialized() == false);
	REQUIRE(s.Area() == area);

	REQUIRE(s.MaxDensity() == numTrees);
	REQUIRE(s.Total_C_ag(0) == 0.0);
	REQUIRE(s.Total_C_ag(-1) == 0.0);
	REQUIRE(s.Mean_C_ag() == 0.0);
	REQUIRE(s.Max_C_ag() == 0.0);
	REQUIRE(s.TotalMortality_C_ag() == 0.0);
	REQUIRE(s.TotalDisturbance_C_ag() == 0.0);
	REQUIRE(s.Total_C_ag_g() == 0.0);
	REQUIRE(s.MeanHeight() == 0.0);
	REQUIRE(s.MeanAge(0) == 0.0);
	REQUIRE(s.MeanAge(-1) == 0.0);

	//check vector initializations
	for (int i = 0; i < numTrees; i++) {
		//REQUIRE(s.Age()[i] == 0);
		//REQUIRE(s.Height()[i] == 0);
		REQUIRE(countVec(s.iDead(),i) == 1);
		REQUIRE(countVec(s.iLive(), i) == 0);
		REQUIRE(s.IsLive(i) == false);
		REQUIRE(s.Mortality_C_ag(i) == 0);
		REQUIRE(s.C_ag(i) == 0.0);
		REQUIRE(s.C_ag_g(i) == 0.0);
		REQUIRE(s.Age(i) == 0);
		REQUIRE(s.Height(i) == 0);
		REQUIRE(s.GetMortalityType(i) == Sawtooth_None);
		REQUIRE(s.GetRecruitmentState(i) == false);
		REQUIRE(s.SpeciesId(i) == speciesID[i]);
	}
}


TEST_CASE("Tree Establishment") {
	Sawtooth::Rng::Random r(1);
	double area = 3.0;
	int numTrees = 10;
	auto speciesID = std::vector<int>(numTrees, 2);
	Sawtooth::Stand s(area, speciesID, numTrees);

	double total = numTrees / 2.0 * (numTrees + 1);

	for (int i = 0; i < 10; i++) {
		s.EstablishTree(i, i+1.0, i+1.0);
	}

	REQUIRE(s.Initialized() == true);
	REQUIRE(s.Area() == area);
	REQUIRE(s.MaxDensity() == numTrees);
	REQUIRE(s.StandDensity() == numTrees / area);
	REQUIRE(s.Total_C_ag(0) == total);
	REQUIRE(s.Total_C_ag(-1) == 0.0);
	REQUIRE(s.Mean_C_ag() == total / numTrees);
	REQUIRE(s.Max_C_ag() == 10.0);
	REQUIRE(s.TotalMortality_C_ag() == 0.0);
	REQUIRE(s.TotalDisturbance_C_ag() == 0.0);
	REQUIRE(s.Total_C_ag_g() == total);
	REQUIRE(s.MeanHeight() == total / numTrees);
	REQUIRE(s.MeanAge(0) == 1.0);
	REQUIRE(s.MeanAge(-1) == 0.0);

	//check vector initializations
	for (int i = 0; i < 10; i++) {
		//REQUIRE(s.Age()[i] == 1);
		//REQUIRE(s.Height()[i] == i+1);
		REQUIRE(countVec(s.iDead(),i) == 0);
		REQUIRE(countVec(s.iLive(),i) == 1);
		REQUIRE(s.IsLive(i) == true);
		REQUIRE(s.Mortality_C_ag(i) == 0);
		REQUIRE(s.C_ag(i) == i+1);
		REQUIRE(s.C_ag_g(i) == i+1);
		REQUIRE(s.Age(i) == 1);
		REQUIRE(s.Height(i) == i+1);
		REQUIRE(s.GetMortalityType(i) == Sawtooth_None);
		REQUIRE(s.GetRecruitmentState(i) == true);
		REQUIRE(s.SpeciesId(i) == speciesID[i]);
	}
	
}

TEST_CASE("Tree Establishment And Growth") {
	Sawtooth::Rng::Random r(1);
	double area = 3.0;
	int numTrees = 10;
	auto speciesID = std::vector<int>(numTrees, 2);
	Sawtooth::Stand s(area, speciesID, numTrees);

	double total = numTrees / 2.0 * (numTrees + 1);

	std::vector<double> growth;
	std::vector<double> tree_height;
	for (int i = 0; i < 10; i++) {
		s.EstablishTree(i, i + 1.0, i + 1.0);
		growth.push_back(1.0);
		tree_height.push_back(2*(i+1));
	}
	s.IncrementAgBiomass(growth);
	s.SetTreeHeight(tree_height);
	s.IncrementAge();

	REQUIRE(s.Initialized() == true);
	REQUIRE(s.Area() == area);
	REQUIRE(s.MaxDensity() == numTrees);
	REQUIRE(s.StandDensity() == numTrees / area);
	REQUIRE(s.Total_C_ag(0) == total + numTrees);
	REQUIRE(s.Total_C_ag(-1) == 0.0);
	REQUIRE(s.Mean_C_ag() == (total + numTrees) / numTrees);
	REQUIRE(s.Max_C_ag() == 11.0);
	REQUIRE(s.TotalMortality_C_ag() == 0.0);
	REQUIRE(s.TotalDisturbance_C_ag() == 0.0);
	REQUIRE(s.Total_C_ag_g() == total + numTrees);
	REQUIRE(s.MeanHeight() == 2 * total / numTrees);
	REQUIRE(s.MeanAge(0) == 2.0);
	REQUIRE(s.MeanAge(-1) == 0.0);

	//check vector initializations
	for (int i = 0; i < 10; i++) {
		//REQUIRE(s.Age()[i] == 2);
		//REQUIRE(s.Height()[i] == 2*(i+1));
		REQUIRE(countVec(s.iDead(),i) == 0);
		REQUIRE(countVec(s.iLive(),i) == 1);
		REQUIRE(s.IsLive(i) == true);
		REQUIRE(s.Mortality_C_ag(i) == 0);
		REQUIRE(s.C_ag(i) == i + 2);
		REQUIRE(s.C_ag_g(i) == i + 2);
		REQUIRE(s.Age(i) == 2);
		REQUIRE(s.Height(i) == 2*(i+1));
		REQUIRE(s.GetMortalityType(i) == Sawtooth_None);
		REQUIRE(s.GetRecruitmentState(i) == true);
		REQUIRE(s.SpeciesId(i) == speciesID[i]);
	}

}
TEST_CASE("Single Tree Mortality") {
	Sawtooth::Rng::Random r(1);
	double area = 3.0;
	int numTrees = 10;
	auto speciesID = std::vector<int>(numTrees, 2);
	Sawtooth::Stand s(area, speciesID, numTrees);

	s.EstablishTree(9, 1.0, 1.0);
	s.KillTree(9, Sawtooth_InsectAttack);

	REQUIRE(s.Initialized() == true);
	REQUIRE(s.Area() == area);
	REQUIRE(s.MaxDensity() == numTrees);
	REQUIRE(s.StandDensity() == 0.0);
	REQUIRE(s.Total_C_ag(0) == 0.0);
	REQUIRE(s.Total_C_ag(-1) == 0.0);
	REQUIRE(s.Mean_C_ag() == 0.0);
	REQUIRE(s.Max_C_ag() == 0.0);
	REQUIRE(s.TotalMortality_C_ag() == 1.0);
	REQUIRE(s.TotalDisturbance_C_ag() == 0.0);
	REQUIRE(s.Total_C_ag_g() == 1.0);
	REQUIRE(s.MeanHeight() == 0.0);
	REQUIRE(s.MeanAge(0) == 0.0);
	REQUIRE(s.MeanAge(-1) == 0.0);

	REQUIRE(s.Mortality_C_ag(9) == 1.0);
	REQUIRE(s.C_ag_g(9) == 1.0);
	REQUIRE(s.GetMortalityType(9) == Sawtooth_InsectAttack);
	REQUIRE(s.GetRecruitmentState(9) == true);

	//check vectors
	for (int i = 0; i < numTrees; i++) {
		//REQUIRE(s.Age()[i] == 0);
		//REQUIRE(s.Height()[i] == 0);
		REQUIRE(countVec(s.iDead(),i) == 1);
		REQUIRE(countVec(s.iLive(),i) == 0);
		REQUIRE(s.IsLive(i) == false);
		REQUIRE(s.C_ag(i) == 0.0);
		REQUIRE(s.Age(i) == 0);
		REQUIRE(s.Height(i) == 0);
		if (i != 9) {
			REQUIRE(s.Mortality_C_ag(i) == 0);
			REQUIRE(s.C_ag_g(i) == 0.0);
			REQUIRE(s.GetMortalityType(i) == Sawtooth_None);
			REQUIRE(s.GetRecruitmentState(i) == false);
		}
	}
}

TEST_CASE("Small Scale Simulation") {
	Sawtooth::Rng::Random r(1);
	double area = 3.0;
	int numTrees = 500;
	auto speciesID = std::vector<int>(numTrees, 2);
	Sawtooth::Stand s(area, speciesID, numTrees);
	
	REQUIRE(s.MeanAge(0) == 0.0);

	double constantGrowthRate = 0.5;
	//std::valarray<double> growthVec(constantGrowthRate, numTrees);
	std::valarray<double> C_ag(0.0, numTrees);
	std::valarray<double> C_ag_g(0.0, numTrees);
	std::valarray<double> Height(0.0, numTrees);
	std::valarray<int> Ages(0, numTrees);
	std::valarray<int> Live(0, numTrees);

	for (int i = 0; i < numTrees / 2; i++) {//make half of the trees live
		C_ag[i] += constantGrowthRate;
		C_ag_g[i] = constantGrowthRate;
		s.EstablishTree(i, constantGrowthRate, 1);
		Ages[i] = 1;
		Live[i] = 1;
	}
	REQUIRE(s.Total_C_ag() == C_ag.sum());
	REQUIRE(s.Total_C_ag_g() == C_ag_g.sum());
	int nlive = Live.sum();
	int lastNLive = nlive;
	double lastMeanAge = (double)Ages.sum() / nlive;
	double lastTotalC_ag = C_ag.sum();
	s.EndStep();
	C_ag_g *= 0;

	int killIndex = 0;

	for (int i = 0; i < nlive/3; i++) {
		s.IncrementAge();

		s.IncrementAgBiomass(std::vector<double>(numTrees, constantGrowthRate));
		std::vector<double> height_tmp;
		for (size_t a = 0; a < Ages.size(); a++) {
			height_tmp.push_back(Ages[a]);
		}
		//set the height to the age for testing
		s.SetTreeHeight(height_tmp);
		//track the values independently
		for (int j = 0; j < numTrees; j++) {
			if (Live[j]) {
				C_ag[j] += constantGrowthRate;
				C_ag_g[j] = constantGrowthRate;
				Height[j] = Ages[j];
				Ages[j] += 1;
			}
		}

		double mortalityC_ag = 0;
		double disturbanceC_ag = 0;

		for (int j = 0; j < 3; j++) {
			if (j == 0) {
				s.KillTree(killIndex, Sawtooth_Disturbance);
				REQUIRE(s.GetMortalityType(killIndex) == Sawtooth_Disturbance);
				disturbanceC_ag += C_ag[killIndex];
			}
			else if(j== 1){
				s.KillTree(killIndex, Sawtooth_RegularMortality);
				REQUIRE(s.GetMortalityType(killIndex) == Sawtooth_RegularMortality);
				mortalityC_ag += C_ag[killIndex];
			}
			else if (j == 2) {
				s.KillTree(killIndex, Sawtooth_InsectAttack);
				REQUIRE(s.GetMortalityType(killIndex) == Sawtooth_InsectAttack);
				mortalityC_ag += C_ag[killIndex];
			}
			Live[killIndex] = 0;
			Height[killIndex] = 0.0;
			C_ag[killIndex] = 0.0;
			Ages[killIndex] = 0;
			
			killIndex++;
			
		}

		int estIndex = numTrees - 1 - i;
		s.EstablishTree(estIndex, constantGrowthRate, 1);
		Live[estIndex] = 1;
		Height[estIndex] = 1.0;
		C_ag[estIndex] += constantGrowthRate;
		C_ag_g[estIndex] = constantGrowthRate;
		Ages[estIndex] = 1;

		nlive = Live.sum();

		//check the stand statistics
		REQUIRE(s.NLive() == nlive);
		REQUIRE(s.NDead() == numTrees - nlive);

		REQUIRE(s.MeanHeight() == Approx((double)Height.sum() / nlive));
		
		REQUIRE(s.Total_C_ag(0) == C_ag.sum());
		REQUIRE(s.Total_C_ag(-1) == lastTotalC_ag);
		lastTotalC_ag = C_ag.sum();
		REQUIRE(s.Mean_C_ag() == Approx((double)C_ag.sum() / nlive));
		REQUIRE(s.Max_C_ag() == C_ag.max());
		REQUIRE(s.TotalMortality_C_ag() == mortalityC_ag);
		REQUIRE(s.TotalDisturbance_C_ag() == disturbanceC_ag);

		REQUIRE(s.MeanAge(0) == Approx((double)Ages.sum() / nlive));
		REQUIRE(s.MeanAge(-1) == Approx(lastMeanAge));
		lastMeanAge = (double)Ages.sum() / nlive;
		REQUIRE(s.StandDensity() == Approx((double)nlive / area));

		REQUIRE(s.RecruitmentRate() == Approx(lastNLive > 0 ? 100.0 / lastNLive : 0.0));
		REQUIRE(s.MortalityRate() == Approx(lastNLive > 0 ? 200.0 / lastNLive : 0.0));
		REQUIRE(s.DisturbanceMortalityRate() == Approx(lastNLive > 0 ? 100.0 / lastNLive : 0.0));
		lastNLive = nlive;
		s.EndStep();
	}
}
#endif