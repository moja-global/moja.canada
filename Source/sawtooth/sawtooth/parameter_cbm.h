#ifndef sawtooth_cbm_parameter_h
#define sawtooth_cbm_parameter_h

#include "dblayer.h"
namespace Sawtooth {
namespace Parameter {
namespace CBM {
	struct RootParameter {
		int id;
		double biomass_to_carbon;
		double hw_a;
		double sw_a;
		double hw_b;
		double frp_a;
		double frp_b;
		double frp_c;
		RootParameter() {}
		RootParameter(Cursor& c) {
			id = c.GetValueInt32("id");
			biomass_to_carbon = c.GetValueDouble("biomass_to_carbon");
			hw_a = c.GetValueDouble("hw_a");
			sw_a = c.GetValueDouble("sw_a");
			hw_b = c.GetValueDouble("hw_b");
			frp_a = c.GetValueDouble("frp_a");
			frp_b = c.GetValueDouble("frp_b");
			frp_c = c.GetValueDouble("frp_c");
		}
	};

	struct StumpParameter {
		int id;
		double sw_top_proportion;
		double sw_stump_proportion;
		double hw_top_proportion;
		double hw_stump_proportion;
		StumpParameter() {}
		StumpParameter(Cursor& c) {
			id = c.GetValueInt32("id");
			sw_top_proportion = c.GetValueDouble("sw_top_proportion");
			sw_stump_proportion = c.GetValueDouble("sw_stump_proportion");
			hw_top_proportion = c.GetValueDouble("hw_top_proportion");
			hw_stump_proportion = c.GetValueDouble("hw_stump_proportion");
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
		TurnoverParameter(Cursor& c) {
			id = c.GetValueInt32("id");
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
