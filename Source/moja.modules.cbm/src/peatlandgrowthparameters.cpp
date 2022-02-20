#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
namespace modules {
namespace cbm {	

	PeatlandGrowthParameters::PeatlandGrowthParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}

	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
	void PeatlandGrowthParameters::setValue(const DynamicObject& data) {
		_FAr = data["FAR"];
		_NPPagls = data["NPPagls"];
		_Bagls = data["Bagls"];
		_a = data["a"];
		_b = data["b"];
		_AFfls = data["AFfls"];
		_Bags = data["Bags"];
		_GCs = data["GCs"];
		_AgBgS = data["AgBgS"];
		_GCsp = data["GCsp"];
		_NPPsp = data["NPPsp"];
		_Rsp = data["Rsp"];
		_GCfm = data["GCfm"];
		_NPPfm = data["NPPfm"];
		_Rfm = data["Rfm"];

		//apply parameter functions
		applyGrowthParameterFunctions();
	}
	
	void PeatlandGrowthParameters::applyGrowthParameterFunctions() {
		_Magls = _NPPagls / _Bagls; //FP1
		_SBags = _Bags * _GCs;		//FP2
		_aNPPs = _AFfls * _SBags;	//FP3
	}	
}}}