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
		PeatlandTurnoverParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType);
		virtual ~PeatlandTurnoverParameters() = default;	

		void setValue(const DynamicObject& data) override;

	private:
		double _Pfe{ 0 };	//Proportion woody Foliage that is Evergreem
		double _Pfn{ 0 };	//Proportion Foliagethat is non-Evergreen/deciduous
		double _Pel{ 0 };	//Proportion of Evergreen leaves lost annually
		double _Pnl{ 0 };	//Proportion of non-Evergreen leaves lost annually
		double _Mbgls{ 0 };	//Mortality rate for belowground low shrubs
		double _Mags{ 0 };	//Mortality for aboveground sedges
		double _Mbgs{ 0 };	//Mortality for bellowground sedges
		double _Pt{ 0 };		//Proportion of decayed/hummified C transferred to next  pool
		double _Ptacro{ 0 };	//Proportion of decayed/hummified C transferred to next  pool
		double _a{ 0 };		//EQ2)   CUMULATIVE C DENSITY	C = a*(z)^b
		double _b{ 0 };		//EQ2)   CUMULATIVE C DENSITY	C = a*(z)^b 
		double _c{ 0 };		//EQ1)   INSTANTANEOUS C DENSITY	C = c*ln(z) - d	
		double _d{ 0 };		//EQ1)   INSTANTANEOUS C DENSITY	C = c*ln(z) - d
		double _Msts{ 0 };
		double _Msto{ 0 };
		double _Mstf{ 0 };
		double _Mstfr{ 0 };
		double _Mstcr{ 0 };
	};
	
}}}
#endif