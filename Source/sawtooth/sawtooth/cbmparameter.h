#ifndef cbm_parameter_h
#define cbm_parameter_h

#include "dblayer.h"
namespace Sawtooth {
namespace Parameter {
namespace CBM {
	struct RootParameter {
		int id;
		double biomass_to_carbon;
		double rb_hw_a;
		double rb_sw_a;
		double rb_hw_b;
		double frp_a;
		double frp_b;
		double frp_c;
		RootParameter() {}
		RootParameter(Cursor c) {
			id = c.GetValueInt32("id");
			biomass_to_carbon = c.GetValueDouble("biomass_to_carbon");
			rb_hw_a = c.GetValueDouble("rb_hw_a");
			rb_sw_a = c.GetValueDouble("rb_sw_a");
			rb_hw_b = c.GetValueDouble("rb_hw_b");
			frp_a = c.GetValueDouble("frp_a");
			frp_b = c.GetValueDouble("frp_b");
			frp_c = c.GetValueDouble("frp_c");
		}
	};

	struct StumpParameter {
		int id;
		double softwood_top_proportion;
		double softwood_stump_proportion;
		double hardwood_top_proportion;
		double hardwood_stump_proportion;
		StumpParameter() {}
		StumpParameter(Cursor c) {
			id = c.GetValueInt32("id");;
			softwood_top_proportion = c.GetValueDouble("softwood_top_proportion");
			softwood_stump_proportion = c.GetValueDouble("softwood_stump_proportion");
			hardwood_top_proportion = c.GetValueDouble("hardwood_top_proportion");
			hardwood_stump_proportion = c.GetValueDouble("hardwood_stump_proportion");
		}
	};

	struct TurnoverParameter {
		int id;
		double SoftwoodFoliageFallRate;
		double HardwoodFoliageFallRate;
		double StemAnnualTurnoverRate;
		double SoftwoodBranchTurnoverRate;
		double HardwoodBranchTurnoverRate;
		double CoarseRootTurnProp;
		double FineRootTurnProp;
		TurnoverParameter() {}
		TurnoverParameter(Cursor c) {
			id = c.GetValueInt32("id");;
			SoftwoodFoliageFallRate = c.GetValueDouble("SoftwoodFoliageFallRate");
			HardwoodFoliageFallRate = c.GetValueDouble("HardwoodFoliageFallRate");
			StemAnnualTurnoverRate = c.GetValueDouble("StemAnnualTurnoverRate");
			SoftwoodBranchTurnoverRate = c.GetValueDouble("SoftwoodBranchTurnoverRate");
			HardwoodBranchTurnoverRate = c.GetValueDouble("HardwoodBranchTurnoverRate");
			CoarseRootTurnProp = c.GetValueDouble("CoarseRootTurnProp");
			FineRootTurnProp = c.GetValueDouble("FineRootTurnProp");
		}
	};

}}}
#endif
