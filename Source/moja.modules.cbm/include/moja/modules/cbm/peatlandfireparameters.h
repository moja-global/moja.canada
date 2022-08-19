#ifndef MOJA_MODULES_CBM_PLFIREPARAS_H_
#define MOJA_MODULES_CBM_PLFIREPARAS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
namespace modules {
namespace cbm {
	
	class CBM_API PeatlandFireParameters : public PeatlandParameters {
	public:
		double CClwsb() const { return _CClwsb; }
		double CClwf() const  { return _CClwf; }
		double CClwr() const  { return _CClwr; }
		double CClsf() const  { return _CClsf; }
		double CClsr() const  { return _CClsr; }
		double CClsp() const  { return _CClsp; }
		double CClfm() const  { return _CClfm; }
		double CCdwsb() const { return _CCdwsb; }
		double CCdwc() const  { return _CCdwc; }
		double CCdwf() const  { return _CCdwf; }
		double CCdwr() const  { return _CCdwr; }
		double CCdsf() const  { return _CCdsf; }
		double CCdsr() const  { return _CCdsr; }
		double CCdfm() const  { return _CCdfm; }
		double CCa() const    { return _CCa; }
		double CCaa() const	  { return _CCaa; }
		double CCco() const   { return _CCco; }
		double CTwr() const   { return _CTwr; }
		double CTsr() const   { return _CTsr; }
		double e() const      { return _e; }
		double f() const      { return _f; }
		double g() const      { return _g; }

		std::vector<double> baseRates() const { return _baseRates; }

		/// <summary>
		/// Default constructor
		/// </summary>
		PeatlandFireParameters(){};
		PeatlandFireParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType);
		virtual ~PeatlandFireParameters() = default;

		void setValue(const DynamicObject& data) override;		

		inline double computeToCO2Rate(double baseRate) {return (_e* baseRate); }
		inline double computeToCORate(double baseRate) {return (_f * baseRate); }
		inline double computeToCH4Rate(double baseRate) {return (_g * baseRate); }

	private:

		double _CClwsb {0};
		double _CClwf {0};
		double _CClwr {0};
		double _CClsf {0};
		double _CClsr {0};
		double _CClsp {0};
		double _CClfm {0};
		double _CCdwsb {0};
		double _CCdwf {0};
		double _CCdwc{ 0 };
		double _CCdwr {0};
		double _CCdsf {0};
		double _CCdsr {0};
		double _CCdfm {0};
		double _CCa {0};
		double _CCaa{ 0 };
		double _CCco{ 0 };
		double _CTwr {0};
		double _CTsr {0};
		double _e {0};
		double _f {0};
		double _g {0};	

		std::vector<double> _baseRates;
	};
	
}}}
#endif