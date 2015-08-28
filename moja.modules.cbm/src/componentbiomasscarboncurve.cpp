#include "moja/modules/cbm/componentbiomasscarboncurve.h"

namespace moja {
namespace modules {
namespace cbm {

	/*
	* Create a component biomass carbon curve 
	* set the maximum age
	* Request the vector size (maximum age + 1)
	*/
	ComponentBiomassCarbonCurve::ComponentBiomassCarbonCurve(int maxAge) : _maxAge(maxAge) {
		_merchCarbonIncrements.resize(_maxAge + 1);
		_foliageCarbonIncrements.resize(_maxAge + 1);
		_otherCarbonIncrements.resize(_maxAge + 1);
	}	

}}}