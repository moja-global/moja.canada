#ifndef stand_cbm_extension_h
#define stand_cbm_extension_h
#include "stand.h"
#include "speciesparameter.h"
#include "parameterset.h"
#include <vector>
namespace Sawtooth {
	namespace CBMExtension {

		enum C_AG_Source {
			//partition the live above ground carbon
			Live,
			//partition the above ground carbon lost to annual mortality
			AnnualMortality,
			//partition the aboveground carbon lost to a prescribed disturbance event
			DisturbanceMortality
		};

		struct CBMBiomassPools {
			CBMBiomassPools() {
				SWM = 0;
				SWF = 0;
				SWO = 0;
				SWCR = 0;
				SWFR = 0;
				HWM = 0;
				HWF = 0;
				HWO = 0;
				HWCR = 0;
				HWFR = 0;
			}
			double SWM;
			double SWF;
			double SWO;
			double SWCR;
			double SWFR;
			double HWM;
			double HWF;
			double HWO;
			double HWCR;
			double HWFR;
		};

		CBMBiomassPools operator+(const CBMBiomassPools& lh, const CBMBiomassPools& rh) {
			CBMBiomassPools result;
			result.SWM = lh.SWM + rh.SWM;
			result.SWF = lh.SWF + rh.SWF;
			result.SWO = lh.SWO + rh.SWO;
			result.SWCR = lh.SWCR + rh.SWCR;
			result.SWFR = lh.SWFR + rh.SWFR;
			result.HWM = lh.HWM + rh.HWM;
			result.HWF = lh.HWF + rh.HWF;
			result.HWO = lh.HWO + rh.HWO;
			result.HWCR = lh.HWCR + rh.HWCR;
			result.HWFR = lh.HWFR + rh.HWFR;
		}

		CBMBiomassPools operator-(const CBMBiomassPools& lh, const CBMBiomassPools& rh) {
			CBMBiomassPools result;
			result.SWM = lh.SWM - rh.SWM;
			result.SWF = lh.SWF - rh.SWF;
			result.SWO = lh.SWO - rh.SWO;
			result.SWCR = lh.SWCR - rh.SWCR;
			result.SWFR = lh.SWFR - rh.SWFR;
			result.HWM = lh.HWM - rh.HWM;
			result.HWF = lh.HWF - rh.HWF;
			result.HWO = lh.HWO - rh.HWO;
			result.HWCR = lh.HWCR - rh.HWCR;
			result.HWFR = lh.HWFR - rh.HWFR;
		}

		struct RootParameter {
			double rb_hw_a;
			double rb_sw_a;
			double rb_hw_b;
			double frp_a;
			double frp_b;
			double frp_c;
		};

		struct StumpParameter {
			double softwood_top_proportion;
			double softwood_stump_proportion;
			double hardwood_top_proportion;
			double hardwood_stump_proportion;
		};

		struct TurnoverParameter {
			double SoftwoodFoliageFallRate;
			double HardwoodFoliageFallRate;
			double StemAnnualTurnoverRate;
			double SoftwoodBranchTurnoverRate;
			double HardwoodBranchTurnoverRate;
			double CoarseRootAGSplit;
			double CoarseRootTurnProp;
			double FineRootAGSplit;
			double FineRootTurnProp;
			double OtherToBranchSnagSplit;
			double BranchSnagTurnoverRate;
			double StemSnagTurnoverRate;
		};

		class StandCBMExtension {
		private:
			Parameter::ParameterSet& Parameters;

		public:
			StandCBMExtension(Parameter::ParameterSet& parameters)
				: Parameters(parameters) {
			}

			CBMBiomassPools ComputeLitterFalls(const TurnoverParameter& t,
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

			CBMBiomassPools ComputeGrossGrowthIncrement(
				CBMBiomassPools netgrowth, const TurnoverParameter& t) {
				return netgrowth + ComputeLitterFalls(t, netgrowth);
			}

			CBMBiomassPools ComputeNetGrowthIncrement(
				const CBMBiomassPools& t0,
				const Stand& stand, double biomassC_utilizationLevel,
				const StumpParameter& stump, const RootParameter& rootParam,
				double biomassToCarbonRate) {

				CBMBiomassPools t1 = PartitionAboveGroundC(Live, stand,
					biomassC_utilizationLevel, stump, rootParam,
					biomassToCarbonRate);
				return t1 - t0;
			}

			CBMBiomassPools ComputeMortality(const Stand& stand,
				double biomassC_utilizationLevel, const StumpParameter& stump,
				const RootParameter& rootParam, double biomassToCarbonRate) {
				return PartitionAboveGroundC(AnnualMortality, stand,
					biomassC_utilizationLevel, stump, rootParam,
					biomassToCarbonRate);
			}

			CBMBiomassPools ComputeDisturbanceMortality(const Stand& stand,
				double biomassC_utilizationLevel, const StumpParameter& stump,
				const RootParameter& rootParam, double biomassToCarbonRate){
				return PartitionAboveGroundC(DisturbanceMortality, stand,
					biomassC_utilizationLevel, stump, rootParam,
					biomassToCarbonRate);
			}


			CBMBiomassPools PartitionAboveGroundC(
				C_AG_Source source,
				const Stand& stand,
				double biomassC_utilizationLevel,
				const StumpParameter& stump,
				const RootParameter& rootParam,
				double biomassToCarbonRate) {

				CBMBiomassPools pools;
				for (auto species : stand.UniqueSpecies()) {
					const auto sp = Parameters.GetSpeciesParameter(species);
					bool deciduous = sp->DeciduousFlag;
					
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
						break;
					case Sawtooth::CBMExtension::DisturbanceMortality:
						for (auto iDead : stand.iDead()) {
							double C_ag = stand.Disturbance_C_ag(iDead);
							Partition(pools, deciduous, C_ag, sp->Cag2Cf1,
								sp->Cag2Cf2, sp->Cag2Cbk1, sp->Cag2Cbk2,
								sp->Cag2Cbr1, sp->Cag2Cbr2,
								biomassC_utilizationLevel, stump, rootParam,
								biomassToCarbonRate);
						break;
					default:
						throw std::invalid_argument("specified source not valid");
					}
				}
				return pools;
			}

			void Partition(CBMBiomassPools& result, bool deciduous, double C_ag,
				double Cag2Cf1, double Cag2Cf2, double Cag2Cbk1,
				double Cag2Cbk2, double Cag2Cbr1, double Cag2Cbr2,
				double biomassC_utilizationLevel, const StumpParameter& stump,
				const RootParameter& rootParam, double biomassToCarbonRate) {

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

				if (C_ag >= biomassC_utilizationLevel) { //group 
					HWStemMerchC = C_ag_hw - HWFoliageC - HWBarkC - HWBranchC;
					SWStemMerchC = C_ag_sw - SWFoliageC - SWBarkC - SWBranchC;
				}
				else {
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

				double swTopAndStump = SWStemMerchC * (stump.softwood_stump_proportion + stump.softwood_top_proportion);
				double hwTopAndStump = HWStemMerchC * (stump.hardwood_stump_proportion + stump.hardwood_top_proportion);

				result.SWM += SWStemMerchC - swTopAndStump;
				result.SWO += SWBarkC + SWBranchC + swTopAndStump + SWStemNonMerchC;
				result.SWF += SWFoliageC;
				result.SWFR += SWFineRootC;
				result.SWCR += SWCoarseRootC;

				result.HWM += HWStemMerchC - hwTopAndStump;
				result.HWO += HWBarkC + HWBranchC + hwTopAndStump + HWStemNonMerchC;
				result.HWF += HWFoliageC;
				result.HWFR += HWFineRootC;
				result.HWCR += HWCoarseRootC;
			}
		};
	}
}
#endif
