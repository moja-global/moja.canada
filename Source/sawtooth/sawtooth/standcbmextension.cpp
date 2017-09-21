#include "standcbmextension.h"
namespace Sawtooth {
	namespace CBMExtension {

		CBMBiomassPools StandCBMExtension::ComputeLitterFalls(
			const Parameter::CBM::TurnoverParameter& t, 
			const CBMBiomassPools& biomass) {
			CBMBiomassPools result;

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

		CBMBiomassPools StandCBMExtension::PartitionAboveGroundC(
			C_AG_Source source,
			const Stand& stand,
			double biomassC_utilizationLevel,
			const Parameter::CBM::StumpParameter& stump,
			const Parameter::CBM::RootParameter& rootParam,
			double biomassToCarbonRate) {

			CBMBiomassPools pools;
			for (auto species : stand.UniqueSpecies()) {
				const auto sp = Parameters.GetSpeciesParameter(species);
				auto deciduous = sp->DeciduousFlag;

				switch (source)
				{
				case Sawtooth::CBMExtension::Live:
					for (auto ilive : stand.iLive()) {
						double C_ag = stand.C_ag(ilive);
						Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
							sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
							sp->Cag2Cbr1, sp->Cag2Cbr2,
							biomassC_utilizationLevel, stump, rootParam,
							biomassToCarbonRate);
					}
					break;
				case Sawtooth::CBMExtension::AnnualMortality:
					for (auto iDead : stand.iDead()) {
						double C_ag = stand.Mortality_C_ag(iDead);
						Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
							sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
							sp->Cag2Cbr1, sp->Cag2Cbr2,
							biomassC_utilizationLevel, stump, rootParam,
							biomassToCarbonRate);
					}
					break;
				case Sawtooth::CBMExtension::DisturbanceMortality:
					for (auto iDead : stand.iDead()) {
						double C_ag = stand.Disturbance_C_ag(iDead);
						Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
							sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
							sp->Cag2Cbr1, sp->Cag2Cbr2,
							biomassC_utilizationLevel, stump, rootParam,
							biomassToCarbonRate);
					}
					break;
				default:
					throw std::invalid_argument("specified source not valid");
				}
			}
			return pools;
		}

		void StandCBMExtension::Partition(CBMBiomassPools& result,
			int deciduous, double C_ag, double Cag2Cf1, double Cag2Cf2,
			double Cag2Cbk1, double Cag2Cbk2, double Cag2Cbr1, double Cag2Cbr2,
			double biomassC_utilizationLevel,
			const Parameter::CBM::StumpParameter& stump,
			const Parameter::CBM::RootParameter& rootParam,
			double biomassToCarbonRate) {

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

			double totalRootBioHW = rootParam.rb_hw_a *
				pow(C_ag_hw / biomassToCarbonRate, rootParam.rb_hw_b);
			double totalRootBioSW = rootParam.rb_sw_a * C_ag_sw / biomassToCarbonRate;
			double fineRootPortion = rootParam.frp_a + rootParam.frp_b *
				exp(rootParam.frp_c * (totalRootBioHW + totalRootBioSW));

			SWCoarseRootC = totalRootBioSW * (1 - fineRootPortion) * biomassToCarbonRate;
			SWFineRootC = totalRootBioSW * fineRootPortion * biomassToCarbonRate;
			HWCoarseRootC = totalRootBioHW * (1 - fineRootPortion) * biomassToCarbonRate;
			HWFineRootC = totalRootBioHW * fineRootPortion * biomassToCarbonRate;

			//find the top and stump using the stump/proportions versus the merchantable stem
			double swTopAndStump = SWStemMerchC * (stump.softwood_stump_proportion + stump.softwood_top_proportion);
			double hwTopAndStump = HWStemMerchC * (stump.hardwood_stump_proportion + stump.hardwood_top_proportion);

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

		CBMAnnualProcesses StandCBMExtension::Compute(
			const CBMBiomassPools& bio_t0, const Stand& stand) {
			CBMAnnualProcesses result;

			double biomassC_utilizationLevel;
			Parameter::CBM::StumpParameter stump;
			Parameter::CBM::RootParameter rootParam;
			Parameter::CBM::TurnoverParameter turnover;
			double biomassToCarbonRate;

			result.NetGrowth = PartitionAboveGroundC(Live, stand,
				biomassC_utilizationLevel, stump, rootParam,
				biomassToCarbonRate) - bio_t0;
			result.Litterfall = ComputeLitterFalls(turnover, 
				bio_t0 + result.NetGrowth);
			result.GrossGrowth = result.NetGrowth + result.Litterfall;
			result.Mortality = PartitionAboveGroundC(AnnualMortality, stand,
				biomassC_utilizationLevel, stump, rootParam,
				biomassToCarbonRate);
			result.Disturbance = PartitionAboveGroundC(DisturbanceMortality,
				stand, biomassC_utilizationLevel, stump, rootParam,
				biomassToCarbonRate);

			return result;
		}

	}
}