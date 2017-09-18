#include "sawtoothmodel.h"
#include "random.h"
#include <random>
namespace Sawtooth {

	SawtoothModel::SawtoothModel(Meta::ModelMeta meta, Parameter::ParameterSet& params,
		Rng::Random& r) : Parameters(params), random(r), Meta(meta){
		constants = params.GetConstants();
	}

	void SawtoothModel::InitializeStand(Stand& stand) {
		if (!stand.Initialized()) {
			int initialLive = constants.Seedling_n;
			double seed_mu = constants.Seedling_mu;
			double seed_sig = constants.Seedling_sig;
			double seed_min = constants.Seedling_min;
			std::vector<double> initialC_ag = random.randnorm(initialLive, seed_mu, seed_sig);
			for (int i = 0; i < initialLive; i++) {
				
				stand.EstablishTree(i, std::max(seed_min, initialC_ag[i]), 0.0);
			}
			stand.SetTreeHeight(ComputeHeight(stand));
		}
	}

	void SawtoothModel::Step(Stand& stand, int t, int s,
		const Parameter::ClimateVariable& climate, 
		int disturbance, StandLevelResult& standlevel) {

		Step(stand, t, s, climate, disturbance, standlevel,
			nullTreeLevelResult);
	}

	void SawtoothModel::Step(Stand& stand, int t, int s,
		const Parameter::ClimateVariable& climate, 
		int disturbance, StandLevelResult& standlevel,
		TreeLevelResult& treeLevel) {

		// Update tree age
		stand.IncrementAge();

		// Recruitment (% yr-1)

		std::vector<double> Pr;
		if (stand.NDead() > 0) {
			switch (Meta.recruitmentModel)
			{
			case Meta::RecruitmentDefault:
				Pr = ComputeRecruitmentDefault(stand);
				break;
			}
		}
		Recruitment(stand, Pr, 
			constants.RecruitmentC, 
			constants.RecruitmentH);

		//// Growth of aboveground carbon(kg C tree^-^ 1 yr^-^ 1)

		std::vector<double> C_ag_G;
		switch (Meta.growthModel) {
		case Meta::GrowthDefault:
			C_ag_G = ComputeGrowthDefault(stand);
			break;
		case Meta::GrowthES1:
			C_ag_G = ComputeGrowthES1(climate, stand);
			break;
		case Meta::GrowthES2:
			C_ag_G = ComputeGrowthES2(climate, stand);
			break;
		case Meta::GrowthES3:
			C_ag_G = ComputeGrowthES3(climate, stand);
			break;
		}

		// Update state variables

		// Update aboveground biomass(kg C tree - 1)
		stand.IncrementAgBiomass(C_ag_G);

		// Update tree height(m)
		stand.SetTreeHeight(ComputeHeight(stand));

		// Mortality, regular(% yr - 1)

		MortalityProbability Pm(stand.MaxDensity());
		switch (Meta.mortalityModel) {
		case Meta::MortalityNone:
			//do nothing probabilities are set to 0 already
			break;
		case Meta::MortalityConstant:
			std::fill(
				Pm.P_Regular.begin(),
				Pm.P_Regular.end(),
				constants.Mortality_P_Regular);

			std::fill(
				Pm.P_Pathogen.begin(),
				Pm.P_Pathogen.end(),
				constants.Mortality_P_Pathogen);

			std::fill(
				Pm.P_Insect.begin(),
				Pm.P_Insect.end(),
				constants.Mortality_P_Insect);

			break;
		case Meta::MortalityDefault:
			ComputeMortalityDefault(stand, Pm);
			break;
		case Meta::MortalityES1:
			ComputeMortalityES1(stand, climate, Pm);
			break;
		case Meta::MortalityES2:
			ComputeMortalityES2(stand, climate, Pm);
			break;
		case Meta::MortalityMLR35:
			ComputeMortalityMLR35(stand, climate, Pm);
			break;
		}

		//Kill the trees due to regular mortality
		Mortality(stand, Pm);

		if (disturbance > 0) {
			Disturbance(stand, disturbance);
		}

		ProcessResults(standlevel, treeLevel, stand, t, s, disturbance);
		stand.EndStep();
	}

	void SawtoothModel::ProcessResults(StandLevelResult& standLevel,
		TreeLevelResult& treeLevel, Stand& stand, int t, int s,
		int dist) {
		if (&treeLevel != &nullTreeLevelResult) {
			for (int i = 0; i < stand.MaxDensity(); i++) {
				treeLevel.Age->SetValue(t, i, stand.Age(i));
				treeLevel.Height->SetValue(t, i, stand.Height(i));
				treeLevel.C_AG->SetValue(t, i, stand.C_ag(i));
				treeLevel.C_AG_G->SetValue(t, i, stand.C_ag_g(i));
				treeLevel.Live->SetValue(t, i, stand.IsLive(i));
				treeLevel.Recruitment
					->SetValue(t, i, stand.GetRecruitmentState(i));
				treeLevel.Mortality_C_ag
					->SetValue(t, i, stand.Mortality_C_ag(i));
				treeLevel.MortalityCode
					->SetValue(t, i, stand.GetMortalityType(i));
				treeLevel.Disturbance_C_ag
					->SetValue(t, i, stand.Disturbance_C_ag(i));
				treeLevel.DisturbanceType
					->SetValue(t, i, dist);
			}
		}
		standLevel.MeanAge->SetValue(s, t, stand.MeanAge());
		standLevel.MeanHeight->SetValue(s, t, stand.MeanHeight());
		standLevel.StandDensity->SetValue(s, t, stand.StandDensity());
		standLevel.TotalBiomassCarbon->SetValue(s, t, stand.Total_C_ag());
		standLevel.TotalBiomassCarbonGrowth
			->SetValue(s, t, stand.Total_C_ag_g());
		standLevel.MeanBiomassCarbon->SetValue(s, t, stand.Mean_C_ag());
		standLevel.RecruitmentRate->SetValue(s, t,stand.RecruitmentRate());
		standLevel.MortalityRate->SetValue(s, t, stand.MortalityRate());
		standLevel.DisturbanceMortalityRate->SetValue(s, t, stand.DisturbanceMortalityRate());
		standLevel.MortalityCarbon->SetValue(s, t,stand.TotalMortality_C_ag());
		standLevel.DisturbanceType->SetValue(s, t, dist);
		standLevel.DisturbanceMortalityCarbon
			->SetValue(s, t, stand.TotalDisturbance_C_ag());
	}
}