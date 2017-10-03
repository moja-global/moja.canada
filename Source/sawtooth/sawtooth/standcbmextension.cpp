#include "standcbmextension.h"
namespace Sawtooth {
	namespace CBMExtension {

		Sawtooth_CBMBiomassPools StandCBMExtension::ComputeLitterFalls(
			const Parameter::CBM::TurnoverParameter& t, 
			const Sawtooth_CBMBiomassPools& biomass) {
			Sawtooth_CBMBiomassPools result;

			result.SWM = biomass.SWM * t.StemAnnualTurnoverRate;
			result.SWO = biomass.SWO * t.SoftwoodBranchTurnoverRate;
			result.SWF = biomass.SWF * t.SoftwoodFoliageFallRate;
			result.SWFR = biomass.SWFR * t.FineRootTurnProp;
			result.SWCR = biomass.SWCR * t.CoarseRootTurnProp;

			result.HWM = biomass.HWM * t.StemAnnualTurnoverRate;
			result.HWO = biomass.HWO * t.HardwoodBranchTurnoverRate;
			result.HWF = biomass.HWF * t.HardwoodFoliageFallRate;
			result.HWFR = biomass.HWFR * t.FineRootTurnProp;
			result.HWCR = biomass.HWCR * t.CoarseRootTurnProp;

			return result;
		}

		Sawtooth_CBMBiomassPools StandCBMExtension::PartitionAboveGroundC(
			C_AG_Source source,
			const Stand& stand,
			const Parameter::CBM::StumpParameter& stump,
			const Parameter::CBM::RootParameter& rootParam) {

			Sawtooth_CBMBiomassPools pools;
			for (auto species : stand.UniqueSpecies()) {
				const auto sp = Parameters.GetSpeciesParameter(species);
				auto deciduous = sp->DeciduousFlag;
				double biomassC_utilizationLevel = Parameters
					.GetBiomassCUtilizationLevel(stand.GetRegionId(), species) / 1000.0;
				switch (source)
				{
				case Sawtooth::CBMExtension::Live:
					for (auto ilive : stand.iLive(species)) {
						double C_ag = stand.C_ag(ilive) / stand.Area() / 1000.0;
						Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
							sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
							sp->Cag2Cbr1, sp->Cag2Cbr2,
							biomassC_utilizationLevel, stump, rootParam);
					}
					break;
				case Sawtooth::CBMExtension::AnnualMortality:
					for (auto iDead : stand.iDead(species)) {
						double C_ag = stand.Mortality_C_ag(iDead) / stand.Area() / 1000.0;
						Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
							sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
							sp->Cag2Cbr1, sp->Cag2Cbr2,
							biomassC_utilizationLevel, stump, rootParam);
					}
					break;
				case Sawtooth::CBMExtension::DisturbanceMortality:
					for (auto iDead : stand.iDead(species)) {
						double C_ag = stand.Disturbance_C_ag(iDead) / stand.Area() / 1000.0;
						Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
							sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
							sp->Cag2Cbr1, sp->Cag2Cbr2,
							biomassC_utilizationLevel, stump, rootParam);
					}
					break;
				default:
					throw std::invalid_argument("specified source not valid");
				}
			}
			return pools;
		}

		void StandCBMExtension::Partition(Sawtooth_CBMBiomassPools& result,
			int deciduous, double C_ag, double Cag2Cf1, double Cag2Cf2,
			double Cag2Cbk1, double Cag2Cbk2, double Cag2Cbr1, double Cag2Cbr2,
			double biomassC_utilizationLevel,
			const Parameter::CBM::StumpParameter& stump,
			const Parameter::CBM::RootParameter& rootParam) {

			if (C_ag == 0.0) { return; }

			double SWFoliageC = 0.0;
			double SWBarkC = 0.0;
			double SWBranchC = 0.0;
			double SWStemMerchC = 0.0;
			double SWStemNonMerchC = 0.0;
			double SWFineRootC = 0.0;
			double SWCoarseRootC = 0.0;
			double HWFoliageC = 0.0;
			double HWBarkC = 0.0;
			double HWBranchC = 0.0;
			double HWStemMerchC = 0.0;
			double HWStemNonMerchC = 0.0;
			double HWFineRootC = 0.0;
			double HWCoarseRootC = 0.0;

			double C_ag_hw = 0;
			double C_ag_sw = 0;

			if (deciduous) { //group C_ag according to species flag
				C_ag_hw = C_ag;
				HWFoliageC = C_ag * Cag2Cf1 * std::pow(C_ag, Cag2Cf2);
				HWBarkC = C_ag * Cag2Cbk1 * std::pow(C_ag, Cag2Cbk2);
				HWBranchC = C_ag * Cag2Cbr1 * std::pow(C_ag, Cag2Cbr2);
			}
			else {
				C_ag_sw = C_ag;
				SWFoliageC = C_ag * Cag2Cf1 * std::pow(C_ag, Cag2Cf2);
				SWBarkC = C_ag * Cag2Cbk1 * std::pow(C_ag, Cag2Cbk2);
				SWBranchC = C_ag * Cag2Cbr1 * std::pow(C_ag, Cag2Cbr2);
			}

			//group by merch/non-merch
			if (C_ag >= biomassC_utilizationLevel) { 
				//merch stems (less the top and stump) go into the merch C pool
				HWStemMerchC = C_ag_hw - HWFoliageC - HWBarkC - HWBranchC;
				SWStemMerchC = C_ag_sw - SWFoliageC - SWBarkC - SWBranchC;
			}
			else {
				//non-merch stems go into the other pool
				HWStemNonMerchC = C_ag_hw - HWFoliageC - HWBarkC - HWBranchC;
				SWStemNonMerchC = C_ag_sw - SWFoliageC - SWBarkC - SWBranchC;
			}

			double totalRootBioHW = rootParam.hw_a *
				std::pow(C_ag_hw / rootParam.biomass_to_carbon, rootParam.hw_b);
			double totalRootBioSW = rootParam.sw_a * C_ag_sw / rootParam.biomass_to_carbon;
			double fineRootPortion = rootParam.frp_a + rootParam.frp_b *
				std::exp(rootParam.frp_c * (totalRootBioHW + totalRootBioSW));

			SWCoarseRootC = totalRootBioSW * (1 - fineRootPortion) *  rootParam.biomass_to_carbon;
			SWFineRootC = totalRootBioSW * fineRootPortion *  rootParam.biomass_to_carbon;
			HWCoarseRootC = totalRootBioHW * (1 - fineRootPortion) *  rootParam.biomass_to_carbon;
			HWFineRootC = totalRootBioHW * fineRootPortion *  rootParam.biomass_to_carbon;

			//find the top and stump using the stump/proportions versus the merchantable stem
			double swTopAndStump = SWStemMerchC * (stump.sw_stump_proportion + stump.sw_top_proportion) * 0.01;
			double hwTopAndStump = HWStemMerchC * (stump.hw_stump_proportion + stump.hw_top_proportion) * 0.01;

			//accumulate the resulting components
			result.SWM += SWStemMerchC - swTopAndStump; //deduct top and stump from Merch
			result.SWO += SWBarkC + SWBranchC + SWStemNonMerchC + swTopAndStump; //and move the top and stump to other
			result.SWF += SWFoliageC;
			result.SWFR += SWFineRootC;
			result.SWCR += SWCoarseRootC;

			result.HWM += HWStemMerchC - hwTopAndStump;
			result.HWO += HWBarkC + HWBranchC + hwTopAndStump + HWStemNonMerchC;
			result.HWF += HWFoliageC;
			result.HWFR += HWFineRootC;
			result.HWCR += HWCoarseRootC;
		}

		Sawtooth_CBMAnnualProcesses StandCBMExtension::Compute(
			const Sawtooth_CBMBiomassPools& bio_t0, const Stand& stand) {
			Sawtooth_CBMAnnualProcesses result;

			const auto stump = Parameters.GetStumpParameter(stand.GetStumpParameterId());
			const auto rootParam = Parameters.GetRootParameter(stand.GetRootParameterId());
			const auto turnover = Parameters.GetTurnoverParameter(stand.GetTurnoverParameterId());

			result.Mortality = PartitionAboveGroundC(AnnualMortality, stand,
				*stump, *rootParam);
			Sawtooth_CBMBiomassPools liveBiomass = PartitionAboveGroundC(Live, stand,
				*stump, *rootParam);
			result.NetGrowth = liveBiomass - bio_t0;
			result.Litterfall = ComputeLitterFalls(*turnover, liveBiomass);
			result.GrossGrowth = result.NetGrowth + result.Litterfall + result.Mortality;
			result.Disturbance = PartitionAboveGroundC(DisturbanceMortality,
				stand, *stump, *rootParam);

			return result;
		}
	}
}