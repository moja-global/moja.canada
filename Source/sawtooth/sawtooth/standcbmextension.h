#ifndef sawtooth_stand_cbm_extension_h
#define sawtooth_stand_cbm_extension_h
#include "stand.h"
#include "parameter_core.h"
#include "parameter_cbm.h"
#include "parameterset.h"
#include "results.h"
#include <vector>
namespace Sawtooth {
	namespace CBMExtension {

		enum C_AG_Source {
			//partition the live above ground carbon
			Live,
			//partition value of the live trees less the live tree’s growth increment
			NetGrowth,
			//partition the above ground carbon lost to annual mortality
			AnnualMortality,
			//partition the aboveground carbon lost to a prescribed disturbance event
			DisturbanceMortality
		};

		class StandCBMExtension {
		private:
			Parameter::ParameterSet& Parameters;

			Sawtooth_CBMBiomassPools PartitionAboveGroundC(
				C_AG_Source source,
				const Stand& stand,
				const Parameter::CBM::StumpParameter& stump,
				const Parameter::CBM::RootParameter& rootParam);

			void Partition(Sawtooth_CBMBiomassPools& result, int deciduous,
				double C_ag, double Cag2Cf1, double Cag2Cf2, double Cag2Cbk1,
				double Cag2Cbk2, double Cag2Cbr1, double Cag2Cbr2,
				double biomassC_utilizationLevel,
				const Parameter::CBM::StumpParameter& stump);

			void PartitionNetGrowth(
				Sawtooth_CBMBiomassPools& netGrowth, const Stand& stand,
				double scaleFactor, int deciduous, double Cag2Cf1,
				double Cag2Cf2, double Cag2Cbk1, double Cag2Cbk2,
				double Cag2Cbr1, double Cag2Cbr2,
				double biomassC_utilizationLevel,
				const Parameter::CBM::StumpParameter& stump);

			void PartitionRoots(Sawtooth_CBMBiomassPools& pools,
				const Parameter::CBM::RootParameter& rootParam);

			Sawtooth_CBMBiomassPools ComputeLitterFalls(const Parameter::CBM::TurnoverParameter& t,
				const Sawtooth_CBMBiomassPools& biomass);

		public:
			StandCBMExtension(Parameter::ParameterSet& parameters)
				: Parameters(parameters) {
			}

			void PerformDisturbance(Stand& stand, Rng::Random& r,
				int disturbanceType);

			void PartialDisturbance1(
				const Sawtooth_CBMBiomassPools& disturbanceLosses,
				Sawtooth::Stand & stand, Sawtooth::Rng::Random & r);
			
			void PartialDisturbance2(
				const Sawtooth_CBMBiomassPools& disturbanceLosses,
				Sawtooth::Stand & stand, Sawtooth::Rng::Random & r);

			Sawtooth_CBMAnnualProcesses Compute(const Stand& stand);
		};
	}
}
#endif
