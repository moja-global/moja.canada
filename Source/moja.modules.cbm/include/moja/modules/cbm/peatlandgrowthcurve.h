#ifndef MOJA_MODULES_CBM_PLGROWTHCURVE_H_
#define MOJA_MODULES_CBM_PLGROWTHCURVE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandparameters.h"


namespace moja {
namespace modules {
namespace cbm {

	class CBM_API PeatlandGrowthcurve {
	public:
		int growthCurveId() const { return _growthCurveId; }		

		/// <summary>
		/// Default constructor
		/// </summary>
		PeatlandGrowthcurve(){}		

		void setValue(const std::vector<DynamicObject>& data);
		void setValue(const DynamicObject& data);

		double getNetGrowthAtAge(int age);

	private:
		int _growthCurveId{ -1 }; // the growth curve ID
			
		//woody layer yield curve, net growth value in terms of carbon (Mg C ha-1)
		std::vector<double> _woodyTotal;  				
	};
	
}}}
#endif