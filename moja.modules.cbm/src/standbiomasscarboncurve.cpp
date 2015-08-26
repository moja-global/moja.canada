#include "moja/modules/cbm/standbiomasscarboncurve.h"

namespace moja {
namespace modules {
namespace cbm {
	
	std::shared_ptr<ComponentBiomassCarbonCurve> StandBiomassCarbonCurve::softwoodCarbonCurve() const { return _softwoodComponent; }

	std::shared_ptr<ComponentBiomassCarbonCurve> StandBiomassCarbonCurve::hardwoodCarbonCurve() const { return _hardwoodComponent; }

	void StandBiomassCarbonCurve::setSoftwoodComponent(std::shared_ptr<ComponentBiomassCarbonCurve> carbonCurve) { _softwoodComponent = carbonCurve; }

	void StandBiomassCarbonCurve::setHardwoodComponent(std::shared_ptr<ComponentBiomassCarbonCurve> carbonCurve) { _hardwoodComponent = carbonCurve; }
}}}