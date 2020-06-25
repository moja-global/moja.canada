#ifndef MOJA_MODULES_CBM_PLWTDBASEFCH4_H_
#define MOJA_MODULES_CBM_PLWTDBASEFCH4_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
namespace modules {
namespace cbm {
	
	class CBM_API PeatlandWTDBaseFCH4Parameters : public PeatlandParameters {
	public:
		double OptCH4WTD() 	const { return _OptCH4WTD; }	//OptCH4WTD   
		double F10r()		const { return  _F10r; }		//F10r            
		double F10d()		const { return _F10d; }			//F10d            
		double FCH4_max() 	const { return _FCH4_max; }		//FCH4_max	  
		

		/// <summary>
		/// Default constructor
		/// </summary>
		PeatlandWTDBaseFCH4Parameters(){};
		PeatlandWTDBaseFCH4Parameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType);
		virtual ~PeatlandWTDBaseFCH4Parameters() = default;	

		void setValue(const DynamicObject& data) override;
		void setFCH4Value(const DynamicObject& data);
		
	private:
		double _OptCH4WTD{ 0.0 };	//OptCH4WTD
		double _F10r{ 0.0 };	//F10r
		double _F10d{ 0.0 };	//F10d
		double _FCH4_max{ 0.0 };	//FCH4_max	
	};
	
}}}
#endif