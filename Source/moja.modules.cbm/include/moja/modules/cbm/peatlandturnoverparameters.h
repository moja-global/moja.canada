#ifndef MOJA_MODULES_CBM_PLTURNOVERPARAS_H_
#define MOJA_MODULES_CBM_PLTURNOVERPARAS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
namespace modules {
namespace cbm {
	
	class CBM_API PeatlandTurnoverParameters : public PeatlandParameters {
	public:
		double Pfe() const { return	_Pfe; }
		double Pfn() const { return  _Pfn; }
		double Pel() const { return  _Pel; }
		double Pnl() const { return  _Pnl; }
		double Mbgls() const { return _Mbgls; }
		double Mags() const { return _Mags; }
		double Mbgs() const { return _Mbgs; }
		double Pt() const { return _Pt; }
		double Ptacro() const { return _Ptacro; }
		double a() const { return _a; }
		double b() const { return _b; }
		double c() const { return _c; }
		double d() const { return _d; }
		double Msts()  const { return _Msts; }
		double Msto()  const { return _Msto; }
		double Mstf()  const { return _Mstf; }
		double Mstfr() const { return _Mstfr; }
		double Mstcr() const { return _Mstcr; }

		

		/// <summary>
		/// Default constructor
		/// </summary>
		PeatlandTurnoverParameters(){};
		PeatlandTurnoverParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier);
		virtual ~PeatlandTurnoverParameters() = default;	

		void setValue(const DynamicObject& data) override;

	private:
		double _Pfe;	//Proportion woody Foliage that is Evergreem
		double _Pfn;	//Proportion Foliagethat is non-Evergreen/deciduous
		double _Pel;	//Proportion of Evergreen leaves lost annually
		double _Pnl;	//Proportion of non-Evergreen leaves lost annually
		double _Mbgls;	//Mortality rate for belowground low shrubs
		double _Mags;	//Mortality for aboveground sedges
		double _Mbgs;	//Mortality for bellowground sedges
		double _Pt;		//Proportion of decayed/hummified C transferred to next  pool
		double _Ptacro;	//Proportion of decayed/hummified C transferred to next  pool
		double _a;		//EQ2)   CUMULATIVE C DENSITY	C = a*(z)^b
		double _b;		//EQ2)   CUMULATIVE C DENSITY	C = a*(z)^b 
		double _c;		//EQ1)   INSTANTANEOUS C DENSITY	C = c*ln(z) - d	
		double _d;		//EQ1)   INSTANTANEOUS C DENSITY	C = c*ln(z) - d
		double _Msts;
		double _Msto;
		double _Mstf;
		double _Mstfr;
		double _Mstcr;

	};
	
}}}
#endif