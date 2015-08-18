#ifndef CBM_PERDFactor_H_
#define CBM_PERDFactor_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace CBM {
	
	class CBM_API PERDFactor {
	public:
		int treeSpeciesTypeId() const { return _treeSpeciesId; }
		int merchEquationNumber() const { return _merchEquationNumber; }
		int nonmerchEquationNumber() const { return _nonmerchEquationNumber; }
		int saplingEquationNumber() const { return _saplingEquationNumber; }
		int otherEquationNumber() const { return _otherEquationNumber; }
		double a() const { return _a; }
		double b() const { return _b; }
		double a_nonmerch() const { return _a_nonmerch; }
		double b_nonmerch() const { return _b_nonmerch; }
		double k_nonmerch() const { return _k_nonmerch; }
		double cap_nonmerch() const { return _cap_nonmerch; }
		double a_sap() const { return _a_sap; }
		double b_sap() const { return _b_sap; }
		double k_sap() const { return _k_sap; }
		double cap_sap() const { return _cap_sap; }
		double a1() const { return _a1; }
		double a2() const { return _a2; }
		double a3() const { return _a3; }
		double b1() const { return _b1; }
		double b2() const { return _b2; }
		double b3() const { return _b3; }
		double c1() const { return _c1; }
		double c2() const { return _c2; }
		double c3() const { return _c3; }
		double min_volume() const { return _min_volume; }
		double max_volume() const { return _max_volume; }
		double low_stemwood_prop() const { return _low_stemwood_prop; }
		double high_stemwood_prop() const { return _high_stemwood_prop; }
		double low_stembark_prop() const { return _low_stembark_prop; }
		double high_stembark_prop() const { return _high_stembark_prop; }
		double low_branches_prop() const { return _low_branches_prop; }
		double high_branches_prop() const { return _high_branches_prop; }
		double low_foliage_prop() const { return _low_foliage_prop; }
		double high_foliage_prop() const { return _high_foliage_prop; }
		double softwood_top_prop() const { return _softwood_top_prop; }
		double softwood_stump_prop() const { return _softwood_stump_prop; }
		double hardwood_top_prop() const { return _hardwood_top_prop; }
		double hardwood_stump_prop() const { return _hardwood_stump_prop; }

			/// <summary>
			/// Default constructor
			/// </summary>
		PERDFactor() :
			_merchEquationNumber(1),
			_nonmerchEquationNumber(2),
			_saplingEquationNumber(4),
			_otherEquationNumber(7) {}

		void setValue(const DynamicObject& data);
		void setDefaultValue(const std::vector<double>& data);
		void setTreeSpeciesID(int id);

		private:
			int _treeSpeciesId;
			int _merchEquationNumber;
			int _nonmerchEquationNumber;
			int _saplingEquationNumber;
			int _otherEquationNumber;
			double _a;
			double _b;
			double _a_nonmerch;
			double _b_nonmerch;
			double _k_nonmerch;
			double _cap_nonmerch;
			double _a_sap;
			double _b_sap;
			double _k_sap;
			double _cap_sap;
			double _a1;
			double _a2;
			double _a3;
			double _b1;
			double _b2;
			double _b3;
			double _c1;
			double _c2;
			double _c3;
			double _min_volume;
			double _max_volume;
			double _low_stemwood_prop;
			double _high_stemwood_prop;
			double _low_stembark_prop;
			double _high_stembark_prop;
			double _low_branches_prop;
			double _high_branches_prop;
			double _low_foliage_prop;
			double _high_foliage_prop;
			double _softwood_top_prop;
			double _softwood_stump_prop;
			double _hardwood_top_prop;
			double _hardwood_stump_prop;
	};
	
}}}
#endif