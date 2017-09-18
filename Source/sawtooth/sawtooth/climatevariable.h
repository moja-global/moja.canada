#ifndef climate_param_h
#define climate_param_h
#include <vector>

namespace Sawtooth {
	namespace Parameter {
		struct ClimateVariable {
			double tmin;
			double tmean;
			double vpd;
			double etr;
			double eeq;
			double ws;
			double ca;
			double ndep;
			double ws_mjjas_z;
			double ws_mjjas_n;
			double etr_mjjas_z;
			double etr_mjjas_n;
		};
	}
}
#endif // !climate_param_h
