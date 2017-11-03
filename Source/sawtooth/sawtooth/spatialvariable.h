#ifndef sawtooth_spatial_variable_h
#define sawtooth_spatial_variable_h
#include <vector>

namespace Sawtooth {
	namespace Parameter {
		struct SpatialVariable {
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
			std::vector<double> SL;
			std::vector<double> TWI;
			std::vector<double> CASL;
		};
	}
}
#endif
