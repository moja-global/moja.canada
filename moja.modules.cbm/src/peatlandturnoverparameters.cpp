#include "moja/modules/cbm/peatlandturnoverparameters.h"

namespace moja {
namespace modules {
namespace cbm {	

	PeatlandTurnoverParameters::PeatlandTurnoverParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier) :
		PeatlandParameters(_spuId, _peatlandType, _peatlandTreeClassifier){}


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
	}

	void PeatlandTurnoverParameters::setDefaultValue(const std::vector<double>& data) {
		
	}

}}}