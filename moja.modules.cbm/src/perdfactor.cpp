#include "moja/modules/cbm/perdfactor.h"

namespace moja {
namespace modules {
namespace cbm {

	void PERDFactor::setTreeSpeciesID(int id) {
		_treeSpeciesId = id;
	}

	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
	void PERDFactor::setValue(const DynamicObject& data) {
		_a = data["a"];
		_b = data["b"];
		_a_nonmerch = data["a_non_merch"];
		_b_nonmerch = data["b_non_merch"];
		_k_nonmerch = data["k_non_merch"];
		_cap_nonmerch = data["cap_non_merch"];
		_a_sap = data["a_sap"];
		_b_sap = data["b_sap"];
		_k_sap = data["k_sap"];
		_cap_sap = data["cap_sap"];
		_a1 = data["a1"];
		_a2 = data["a2"];
		_a3 = data["a3"];
		_b1 = data["b1"];
		_b2 = data["b2"];
		_b3 = data["b3"];
		_c1 = data["c1"];
		_c2 = data["c2"];
		_c3 = data["c3"];
		_min_volume = data["min_volume"];
		_max_volume = data["max_volume"];
		_low_stemwood_prop   = data["low_stemwood_prop"];
		_high_stemwood_prop  = data["high_stemwood_prop"];
		_low_stembark_prop   = data["low_stembark_prop"];
		_high_stembark_prop  = data["high_stembark_prop"];
		_low_branches_prop   = data["low_branches_prop"];
		_high_branches_prop  = data["high_branches_prop"];
		_low_foliage_prop    = data["low_foliage_prop"];
		_high_foliage_prop   = data["high_foliage_prop"];
		_softwood_top_prop   = data["softwood_top_prop"];
		_softwood_stump_prop = data["softwood_stump_prop"];
		_hardwood_top_prop   = data["hardwood_top_prop"];
		_hardwood_stump_prop = data["hardwood_stump_prop"];
	}

	void PERDFactor::setDefaultValue(const std::vector<double>& data) {
		int idx = 1;
		_a = data[idx++];
		_b = data[idx++];
		_a_nonmerch = data[idx++];
		_b_nonmerch = data[idx++];
		_k_nonmerch = data[idx++];
		_cap_nonmerch = data[idx++];
		_a_sap = data[idx++];
		_b_sap = data[idx++];
		_k_sap = data[idx++];
		_cap_sap = data[idx++];
		_a1 = data[idx++];
		_a2 = data[idx++];
		_a3 = data[idx++];
		_b1 = data[idx++];
		_b2 = data[idx++];
		_b3 = data[idx++];
		_c1 = data[idx++];
		_c2 = data[idx++];
		_c3 = data[idx++];
		_min_volume = data[idx++];
		_max_volume = data[idx++];
		_low_stemwood_prop = data[idx++];
		_high_stemwood_prop = data[idx++];
		_low_stembark_prop = data[idx++];
		_high_stembark_prop = data[idx++];
		_low_branches_prop = data[idx++];
		_high_branches_prop = data[idx++];
		_low_foliage_prop = data[idx++];
		_high_foliage_prop = data[idx++];
		_softwood_top_prop = data[idx++];
		_softwood_stump_prop = data[idx++];
		_hardwood_top_prop = data[idx++];
		_hardwood_stump_prop = data[idx++];
	}

}}}