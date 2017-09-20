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

		//summary of partitioned flows
		struct CBMAnnualProcesses {
			//total annual growth including litter fall (NPP)
			CBMBiomassPools GrossGrowth;
			//the annual net change in stand biomass (equal to 
			//GrossGrowth - Litterfall)
			CBMBiomassPools NetGrowth;
			//losses due to litterfalls
			CBMBiomassPools Litterfall;
			//losses due to annual mortality
			CBMBiomassPools Mortality;
			//losses due to a prescribed disturbance event
			CBMBiomassPools Disturbance;
		};

		CBMBiomassPools operator+(const CBMBiomassPools& lh,
			const CBMBiomassPools& rh) {
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

		CBMBiomassPools operator-(const CBMBiomassPools& lh,
			const CBMBiomassPools& rh) {
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
			int id;
			double rb_hw_a;
			double rb_sw_a;
			double rb_hw_b;
			double frp_a;
			double frp_b;
			double frp_c;
		};

		struct StumpParameter {
			int id;
			double softwood_top_proportion;
			double softwood_stump_proportion;
			double hardwood_top_proportion;
			double hardwood_stump_proportion;
		};

		struct TurnoverParameter {
			int id;
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

			CBMBiomassPools PartitionAboveGroundC(
				C_AG_Source source,
				const Stand& stand,
				double biomassC_utilizationLevel,
				const StumpParameter& stump,
				const RootParameter& rootParam,
				double biomassToCarbonRate);

			void Partition(CBMBiomassPools& result, bool deciduous, double C_ag,
				double Cag2Cf1, double Cag2Cf2, double Cag2Cbk1,
				double Cag2Cbk2, double Cag2Cbr1, double Cag2Cbr2,
				double biomassC_utilizationLevel, const StumpParameter& stump,
				const RootParameter& rootParam, double biomassToCarbonRate);

			CBMBiomassPools ComputeLitterFalls(const TurnoverParameter& t,
				const CBMBiomassPools& biomass);

		public:
			StandCBMExtension(Parameter::ParameterSet& parameters)
				: Parameters(parameters) {
			}

			CBMAnnualProcesses Compute(const CBMBiomassPools& bio_t0, const Stand& stand);
		};
	}
}
#endif
