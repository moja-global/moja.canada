#include "moja/modules/cbm/componentbiomasscarboncurve.h"

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * @brief Constructor
	 * 
	 * Assign ComponentBiomassCarbonCurve._maxAge as parameter maxAge , \n
	 * The sizes of vectors ComponentBiomassCarbonCurve._merchCarbonIncrements, \n
	 * ComponentBiomassCarbonCurve._foliageCarbonIncrements and ComponentBiomassCarbonCurve._otherCarbonIncrements \n
	 * as maxAge + 1
	 * 
	 * @param maxAge int
	 * **********************/
	ComponentBiomassCarbonCurve::ComponentBiomassCarbonCurve(int maxAge)
		: _maxAge(maxAge), _merchCarbonIncrements(maxAge + 1),
		  _foliageCarbonIncrements(maxAge + 1), _otherCarbonIncrements(maxAge + 1) {}	

}}}