#ifndef stand_cbm_extension_h
#define stand_cbm_extension_h
#include "stand.h"
#include "speciesparameter.h"
#include "parameterset.h"
#include "cbmparameter.h"
#include "results.h"
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
			return result;
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
			return result;
		}

		class StandCBMExtension {
		private:
			Parameter::ParameterSet& Parameters;

			CBMBiomassPools PartitionAboveGroundC(
				C_AG_Source source,
				const Stand& stand,
				double biomassC_utilizationLevel,
				const Parameter::CBM::StumpParameter& stump,
				const Parameter::CBM::RootParameter& rootParam,
				double biomassToCarbonRate);

			void Partition(CBMBiomassPools& result, int deciduous, double C_ag,
				double Cag2Cf1, double Cag2Cf2, double Cag2Cbk1,
				double Cag2Cbk2, double Cag2Cbr1, double Cag2Cbr2,
				double biomassC_utilizationLevel, const Parameter::CBM::StumpParameter& stump,
				const Parameter::CBM::RootParameter& rootParam, double biomassToCarbonRate);

			CBMBiomassPools ComputeLitterFalls(const Parameter::CBM::TurnoverParameter& t,
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
