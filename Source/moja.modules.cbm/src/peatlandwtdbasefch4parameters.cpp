#include "moja/modules/cbm/peatlandwtdbasefch4parameters.h"

namespace moja {
namespace modules {
namespace cbm {	

    /**
	 * Constructor
	 * 
	 * Initialise PeatlandParameters with parameters _spuId, _peatlandType and _landCoverType.
	 * 
	 * @param int _spuId
	 * @param PeatlandType _peatlandType
	 * @param PeatlandLandCoverType _landCoverType
	 * **********************************/
	PeatlandWTDBaseFCH4Parameters::PeatlandWTDBaseFCH4Parameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType){}

	/**
	 * Assign PeatlandWTDBaseFCH4Parameters._OptCH4WTD, PeatlandWTDBaseFCH4Parameters._F10r \n
	 * and PeatlandWTDBaseFCH4Parameters._F10d values of variables "OptCH4WTD", "F10r", "F10d" in paramater data
	 * 
	 * @param DynamicObject& data
	 * @return void
	 * *********************/
	void PeatlandWTDBaseFCH4Parameters::setValue(const DynamicObject& data) {
		_OptCH4WTD = data["OptCH4WTD"];
		_F10r = data["F10r"];
		_F10d = data["F10d"];		
	}
	
	/**
	 * Assign PeatlandWTDBaseFCH4Parameters._FCH4_max value of "FCH4_max" in parameter data
	 * 
	 * @param DynamicObject& data
	 * @return void
	 * *********************/
	void PeatlandWTDBaseFCH4Parameters::setFCH4Value(const DynamicObject& data){
		_FCH4_max = data["FCH4_max"];
	}
}}}