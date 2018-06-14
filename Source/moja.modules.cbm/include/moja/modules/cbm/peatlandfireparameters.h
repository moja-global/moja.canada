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
		double CCdwf() const  { return _CCdwf; }
		double CCdwr() const  { return _CCdwr; }
		double CCdsf() const  { return _CCdsf; }
		double CCdsr() const  { return _CCdsr; }
		double CCdfm() const  { return _CCdfm; }
		double Cca() const    { return _Cca; }
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
		PeatlandFireParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier);
		virtual ~PeatlandFireParameters() = default;

		void setValue(const DynamicObject& data) override;		

		inline double computeToCO2Rate(double baseRate) {return (_e* baseRate); }
		inline double computeToCORate(double baseRate) {return (_f * baseRate); }
		inline double computeToCH4Rate(double baseRate) {return (_g * baseRate); }

	private:

		double _CClwsb;
		double _CClwf;
		double _CClwr;
		double _CClsf;
		double _CClsr;
		double _CClsp;
		double _CClfm;
		double _CCdwsb;
		double _CCdwf;
		double _CCdwr;
		double _CCdsf;
		double _CCdsr;
		double _CCdfm;
		double _Cca;
		double _CTwr;
		double _CTsr;
		double _e;
		double _f;
		double _g;	

		std::vector<double> _baseRates;
	};
	
}}}
#endif