#include "moja/modules/cbm/peatlandturnoverparameters.h"

namespace moja {
namespace modules {
namespace cbm {	
	
	/**
	 * Constructor
	 * 
	 * Inherit parent constructor PeatlandParameters with parameters _spuId,_peatlandType and _landCoverType
	 * 
	 * @param int _spuId
	 * @param PeatlandType _peatlandType
	 * @param PeatlandLandCoverType _landCoverType
	 * **********************************/
	PeatlandTurnoverParameters::PeatlandTurnoverParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}

	/**
	 * Initialise member attributesf84f6 (review source documentation)
	 * 
	 * Assign values to PeatlandTurnoverParameters._Pfe, PeatlandTurnoverParameters._Pfn, PeatlandTurnoverParameters._Pel, \n
	 * PeatlandTurnoverParameters._Pnl, PeatlandTurnoverParameters._Mbgls, PeatlandTurnoverParameters._Mags, PeatlandTurnoverParameters._Mbgs, \n
	 * PeatlandTurnoverParameters._Pt, PeatlandTurnoverParameters._Ptacro, PeatlandTurnoverParameters._a \n,
	 * PeatlandTurnoverParameters._b, PeatlandTurnoverParameters._c, PeatlandTurnoverParameters._d \n
	 * PeatlandTurnoverParameters._Msts, PeatlandTurnoverParameters._Msto, PeatlandTurnoverParameters._Mstf, \n 
	 * PeatlandTurnoverParameters._Mstfr, PeatlandTurnoverParameters._Mstcr from parameter data
	 * 
	 * @param DynamicObject& data
	 * @return void
 	 * *************************/
	void PeatlandTurnoverParameters::setValue(const DynamicObject& data) {
		_Pfe = data["Pfe"];
		_Pfn = data["Pfn"];
		_Pel = data["Pel"];
		_Pnl = data["Pnl"];
		_Mbgls = data["Mbgls"];
		_Mags = data["Mags"];
		_Mbgs = data["Mbgs"];
		_Pt = data["Pt"];
		_Ptacro = data["Ptacro"];
		_a = data["a"];		
		_b = data["b"];		
		_c = data["c"];		
		_d = data["d"];		
		_Msts = data["Msts"];
		_Msto = data["Msto"];
		_Mstf = data["Mstf"];
		_Mstfr = data["Mstfr"];
		_Mstcr = data["Mstcr"];
	}
}}}