#include "moja/modules/cbm/peatlandwtdbasefch4parameters.h"

namespace moja {
namespace modules {
namespace cbm {	

	PeatlandWTDBaseFCH4Parameters::PeatlandWTDBaseFCH4Parameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}


	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
	void PeatlandWTDBaseFCH4Parameters::setValue(const DynamicObject& data) {
		_OptCH4WTD = data["OptCH4WTD"];
		_F10r = data["F10r"];
		_F10d = data["F10d"];		
	}
	
	void PeatlandWTDBaseFCH4Parameters::setFCH4Value(const DynamicObject& data){
		_FCH4_max = data["FCH4_max"];
	}
}}}