#include "moja/modules/cbm/peatlandfireparameters.h"

namespace moja {
namespace modules {
namespace cbm {	

	PeatlandFireParameters::PeatlandFireParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}


	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
	void PeatlandFireParameters::setValue(const DynamicObject& data) {
		_baseRates.clear();

		_CClwsb = data["CClwsb"];
		_baseRates.push_back(_CClwsb);

		_CClwf = data["CClwf"];
		_baseRates.push_back(_CClwf);

		_CClwr = data["CClwr"];
		_baseRates.push_back(_CClwr);

		_CClsf = data["CClsf"];
		_baseRates.push_back(_CClsf);

		_CClsr = data["CClsr"];
		_baseRates.push_back(_CClsr);

		_CClsp = data["CClsp"];
		_baseRates.push_back(_CClsp);

		_CClfm = data["CClfm"];
		_baseRates.push_back(_CClfm);

		_CCdwsb = data["CCdwsb"];
		_baseRates.push_back(_CCdwsb);

		_CCdwf = data["CCdwf"];
		_baseRates.push_back(_CCdwf);
		
		_CCdwc = data["CCdwc"];
		_baseRates.push_back(_CCdwc);
		
		_CCdwr = data["CCdwr"];
		_baseRates.push_back(_CCdwr);

		_CCdsf = data["CCdsf"];
		_baseRates.push_back(_CCdsf);

		_CCdsr = data["CCdsr"];
		_baseRates.push_back(_CCdsr);

		_CCdfm = data["CCdfm"];
		_baseRates.push_back(_CCdfm);

		_CCa = data["CCa"];
		_baseRates.push_back(_CCa); // for acrotelm_o

		_CCaa = data["CCaa"];
		_baseRates.push_back(_CCaa); // for acrotelm_a

		_CCco = data["CCco"];
		_baseRates.push_back(_CCco); // for acrotelm_a		

		_CTwr = data["CTwr"];
		_CTsr = data["CTsr"];		

		_e = data["e"];
		_f = data["f"];
		_g = data["g"];		
	}	
}}}