#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
namespace modules {
namespace cbm {	

     /**
	 * Constructor
	 * 
	 * Initialise PeatlandParameters with parameters _spuId,_peatlandType and _landCoverType.
	 * 
	 * @param _spuId int
	 * @param  _peatlandType PeatlandType
	 * @param  _landCoverType PeatlandLandCoverType
	 * **********************************/
	PeatlandGrowthParameters::PeatlandGrowthParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}

	/**
	 * Initialise member attributes
	 * 
	 * Assign values to PeatlandGrowthParameters._FAr, PeatlandGrowthParameters._NPPagls, PeatlandGrowthParameters._a, \n 
	 * PeatlandGrowthParameters._b, PeatlandGrowthParameters._AFfls, PeatlandGrowthParameters._Bags, PeatlandGrowthParameters._GCs, \n
	 * PeatlandGrowthParameters._AgBgS, PeatlandGrowthParameters._GCsp, PeatlandGrowthParameters._NPPsp,PeatlandGrowthParameters. _Rsp, PeatlandGrowthParameters._GCfm, \n 
	 * PeatlandGrowthParameters._NPPfm, PeatlandGrowthParameters._Rfm from parameter data
	 * 
	 * Invoke PeatlandGrowthParameters.applyGrowthParameterFunctions() 
	 * 
	 * @param  data DynamicObject&
	 * @return void
	 * ***************************/
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
	
	/**
	 * Initialise Growth parameters 
	 * 
	 * Assign PeatlandGrowthParameters._Magls as  PeatlandGrowthParameters._NPPagls / PeatlandGrowthParameters._Bagls, \n
	 * PeatlandGrowthParameters._SBags as PeatlandGrowthParameters._Bags * PeatlandGrowthParameters._GCs, \n
	 * PeatlandGrowthParameters._aNPPs as PeatlandGrowthParameters._AFfls * PeatlandGrowthParameters._SBags
	 * 
	 * @return void
	 * ************************/
	void PeatlandGrowthParameters::applyGrowthParameterFunctions() {
		_Magls = _NPPagls / _Bagls; //FP1
		_SBags = _Bags * _GCs;		//FP2
		_aNPPs = _AFfls * _SBags;	//FP3
	}	
}}}