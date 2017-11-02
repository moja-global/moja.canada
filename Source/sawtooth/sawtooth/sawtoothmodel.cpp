#include "sawtoothmodel.h"
#include "random.h"
#include "standcbmextension.h"

namespace Sawtooth {

	SawtoothModel::SawtoothModel(Sawtooth_ModelMeta meta, Parameter::ParameterSet& params,
		Rng::Random& r) : Parameters(params), random(r), Meta(meta){
		constants = params.GetConstants();
	}

	void SawtoothModel::InitializeStand(Stand& stand) {
		if (!stand.Initialized()) {
			int initialLive = constants.Seedling_n;
			double seed_mu = constants.Seedling_mu;
			double seed_sig = constants.Seedling_sig;
			double seed_min = constants.Seedling_min;
			std::vector<double> initialC_ag = random.randnorm(initialLive,
				seed_mu, seed_sig);
			for (int i = 0; i < initialLive; i++) {
				stand.EstablishTree(i, std::max(seed_min, initialC_ag[i]), 0.0);
			}
			stand.SetTreeHeight(ComputeHeight(stand));
		}
	}	
	
	void SawtoothModel::Step(Stand& stand, int t, int s,
		const Parameter::SpatialVariable& spatial,
		int disturbance, Sawtooth_StandLevelResult& standlevel,
		Sawtooth_CBMResult* cbmResult,
		Sawtooth_TreeLevelResult* treeLevel) {

		// Update tree age
		stand.IncrementAge();

		// Recruitment (% yr-1)

		std::vector<double> Pr;
		if (stand.NDead() > 0) {
			switch (Meta.recruitmentModel)
			{
			case Sawtooth_RecruitmentD1:
				Pr = ComputeRecruitmentD1(stand);
				break;
			case Sawtooth_RecruitmentD2:
				Pr = ComputeRecruitmentD2(stand);
				break;
			}
		}
		Recruitment(stand, Pr, 
			constants.RecruitmentC, 
			constants.RecruitmentH);

		//// Growth of aboveground carbon(kg C tree^-^ 1 yr^-^ 1)

		std::vector<double> C_ag_G;
		switch (Meta.growthModel) {
		case Sawtooth_GrowthD1:
			C_ag_G = ComputeGrowthD1(stand);
			break;
		case Sawtooth_GrowthD2:
			C_ag_G = ComputeGrowthD2(stand);
			break;
		case Sawtooth_GrowthES1:
			C_ag_G = ComputeGrowthES1(spatial, stand);
			break;
		case Sawtooth_GrowthES2:
			C_ag_G = ComputeGrowthES2(spatial, stand);
			break;
		case Sawtooth_GrowthES3:
			C_ag_G = ComputeGrowthES3(spatial, stand);
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
		case Sawtooth_MortalityNone:
			//do nothing probabilities are set to 0 already
			break;
		case Sawtooth_MortalityConstant:
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
		case Sawtooth_MortalityD1:
			ComputeMortalityD1(stand, Pm);
			break;
		case Sawtooth_MortalityD2:
			ComputeMortalityD2(stand, Pm);
			break;
		case Sawtooth_MortalityES1:
			ComputeMortalityES1(stand, spatial, Pm);
			break;
		case Sawtooth_MortalityES2:
			ComputeMortalityES2(stand, spatial, Pm);
			break;
		case Sawtooth_MortalityMLR35:
			ComputeMortalityMLR35(stand, spatial, Pm);
			break;
		}

		//Kill the trees due to regular mortality
		Mortality(stand, Pm);

		if (Meta.CBMEnabled) {

			CBMExtension::StandCBMExtension cbm_ext(Parameters);
			if (disturbance > 0) {
				cbm_ext.PerformDisturbance(stand, random, disturbance);
			}
			Sawtooth_CBMAnnualProcesses cbmstep = cbm_ext.Compute(stand);
			cbmResult->Processes[t] = cbmstep;
		}
		else if (disturbance > 0) {
			//perform the regular Sawtooth disturbance
			Disturbance(stand, disturbance);
		}

		ProcessResults(standlevel, treeLevel, stand, t, s,
			disturbance);
		stand.EndStep();
	}

	void SawtoothModel::ProcessResults(Sawtooth_StandLevelResult& standLevel,
		Sawtooth_TreeLevelResult* treeLevel, Stand& stand, int t, int s,
		int dist) {
		if (treeLevel != NULL) {
			for (int i = 0; i < stand.MaxDensity(); i++) {
				treeLevel->Age->SetValue(t, i, stand.Age(i));
				treeLevel->Height->SetValue(t, i, stand.Height(i));
				treeLevel->C_AG->SetValue(t, i, stand.C_ag(i));
				treeLevel->C_AG_G->SetValue(t, i, stand.C_ag_g(i));
				treeLevel->Live->SetValue(t, i, stand.IsLive(i));
				treeLevel->Recruitment
					->SetValue(t, i, stand.GetRecruitmentState(i));
				treeLevel->Mortality_C_ag
					->SetValue(t, i, stand.Mortality_C_ag(i));
				treeLevel->MortalityCode
					->SetValue(t, i, stand.GetMortalityType(i));
				treeLevel->Disturbance_C_ag
					->SetValue(t, i, stand.Disturbance_C_ag(i));
				treeLevel->DisturbanceType->SetValue(t, i, dist);
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