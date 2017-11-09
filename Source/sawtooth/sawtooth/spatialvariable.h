#ifndef sawtooth_spatial_variable_h
#define sawtooth_spatial_variable_h
#include <vector>

namespace Sawtooth {
	namespace Parameter {
		struct SpatialVariable {
			double tmean_ann;
			double tmin_ann;
			double tmean_gs;
			double vpd;
			double etp_gs;
			double eeq;
			double ws_gs;
			double ca;
			double ndep;
			double ws_gs_z;
			double ws_gs_n;
			double etp_gs_z;
			double etp_gs_n;
			double slope;
			double twi;
			double aspect;
		};
	}
}
#endif
