#ifndef stand_cbm_extension_h
#define stand_cbm_extension_h
#include "stand.h"
#include "speciesparameter.h"
#include <vector>
namespace Sawtooth {
	namespace CBMExtension {

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
		public:

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

			CBMBiomassPools PartitionAboveGroundC(
				const std::vector<double>& aboveGroundC,
				const Parameter::SpeciesParameter& sp,
				const StumpParameter& stump,
				const RootParameter& rootParam,
				double biomassToCarbonRate) {

				double SWFoliage = 0.0;
				double SWBark = 0.0;
				double SWBranch = 0.0;
				double SWStem = 0.0;
				double SWFineRoot = 0.0;
				double SWCoarseRoot = 0.0;
				double HWFoliage = 0.0;
				double HWBark = 0.0;
				double HWBranch = 0.0;
				double HWStem = 0.0;
				double HWFineRoot = 0.0;
				double HWCoarseRoot = 0.0;

				double C_ag_hw = 0;
				double C_ag_sw = 0;
				for (auto C_ag : aboveGroundC) {

					if (sp.DeciduousFlag) {
						C_ag_hw += C_ag;
						HWFoliage += C_ag * sp.Cag2Cf1 * std::pow(C_ag, sp.Cag2Cf2);
						HWBark += C_ag * sp.Cag2Cbk1 * std::pow(C_ag, sp.Cag2Cbk2);
						HWBranch += C_ag * sp.Cag2Cbr1 * std::pow(C_ag, sp.Cag2Cbr2);
					}
					else {
						C_ag_sw += C_ag;
						SWFoliage += C_ag * sp.Cag2Cf1 * std::pow(C_ag, sp.Cag2Cf2);
						SWBark += C_ag * sp.Cag2Cbk1 * std::pow(C_ag, sp.Cag2Cbk2);
						SWBranch += C_ag * sp.Cag2Cbr1 * std::pow(C_ag, sp.Cag2Cbr2);
					}

					HWStem = C_ag_hw - HWFoliage - HWBark - HWBranch;
					SWStem = C_ag_sw - SWFoliage - SWBark - SWBranch;

					double totalRootBioHW = rootParam.rb_hw_a *
						pow(C_ag_hw / biomassToCarbonRate, rootParam.rb_hw_b);
					double totalRootBioSW = rootParam.rb_sw_a * C_ag_sw / biomassToCarbonRate;
					double fineRootPortion = rootParam.frp_a + rootParam.frp_b *
						exp(rootParam.frp_c * (totalRootBioHW + totalRootBioSW));

					SWCoarseRoot = totalRootBioSW * (1 - fineRootPortion) * biomassToCarbonRate;
					SWFineRoot = totalRootBioSW * fineRootPortion * biomassToCarbonRate;
					HWCoarseRoot = totalRootBioHW * (1 - fineRootPortion) * biomassToCarbonRate;
					HWFineRoot = totalRootBioHW * fineRootPortion * biomassToCarbonRate;

					double swTopAndStump = SWStem * (stump.softwood_stump_proportion + stump.softwood_top_proportion);
					double hwTopAndStump = HWStem * (stump.hardwood_stump_proportion + stump.hardwood_top_proportion);

					CBMBiomassPools result;
					result.SWM = SWStem - swTopAndStump;
					result.SWO = SWBark + SWBranch + swTopAndStump;
					result.SWF = SWFoliage;
					result.SWFR = SWFineRoot;
					result.SWCR = SWCoarseRoot;

					result.HWM = HWStem - hwTopAndStump;
					result.HWO = HWBark + HWBranch + hwTopAndStump;
					result.HWF = HWFoliage;
					result.HWFR = HWFineRoot;
					result.HWCR = HWCoarseRoot;
					return result;
				}
			}
		};
	}
}
#endif
