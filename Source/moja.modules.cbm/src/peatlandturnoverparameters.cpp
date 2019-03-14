#include "moja/modules/cbm/peatlandturnoverparameters.h"

namespace moja {
namespace modules {
namespace cbm {	

	PeatlandTurnoverParameters::PeatlandTurnoverParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}


	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
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