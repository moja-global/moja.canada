#ifndef MOJA_MODULES_CBM_PLDECAYPARAS_H_
#define MOJA_MODULES_CBM_PLDECAYPARAS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
namespace modules {
namespace cbm {
	
	class CBM_API PeatlandDecayParameters : public PeatlandParameters{
	public:
		double akwsb() const { return _akwsb; }
		double akwc() const { return _akwc; }
		double akwfe() const { return  _akwfe; }	
		double akwfne() const { return  _akwfne; }
		double akwr() const { return   _akwr; }
		double akfm() const { return _akfm; }
		double aksf() const { return _aksf; }
		double aksr() const { return _aksr; }
		double aka() const { return _aka; }
		double akc() const { return _akc; }
		double akaa() const { return _aka; }
		double akco() const { return _akc; }

		double MAT() const { return _MAT; }
		double tref() const { return  _tref; }
		double c() const { return _c; }
		double d() const { return _d; }

		double Q10wsb() const { return _Q10wsb; }
		double Q10wc() const { return _Q10wc; }
		double Q10wf() const { return _Q10wf; }
		double Q10wr() const { return _Q10wr; }
		double Q10sf() const { return _Q10sf; }
		double Q10sr() const { return _Q10sr; }		
		double Q10fm() const { return _Q10fm; }	
		double Q10a() const { return  _Q10a; }		
		double Q10c() const { return  _Q10c; }

		double Pt() const { return _Pt; }

		double kwsb() const { return _kwsb; }
		double kwc() const { return _kwc; }
		double kwfe() const { return  _kwfe; }
		double kwfne() const { return  _kwfne; }
		double kwr() const { return   _kwr; }
		double kfm() const { return _kfm; }
		double ksf() const { return _ksf; }
		double ksr() const { return _ksr; }
		double ka() const { return _ka; }
		double kc() const { return _kc; }
		double kaa() const { return _ka; }
		double kco() const { return _kc; }	
		

		/// <summary>
		/// Default constructor
		/// </summary>
		PeatlandDecayParameters(){};

		PeatlandDecayParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType);

		virtual ~PeatlandDecayParameters() = default;
		
		void setValue(const DynamicObject& data) override;		

		void updateAppliedDecayParameters(double meanAnnualTemperature);		


	private:
		double _akwsb {0};
		double _akwc {0};
		double _akwfe {0};	
		double _akwfne {0};
		double _akwr {0};
		double _aksf {0};
		double _aksr {0};
		double _akfm {0};
		double _aka {0};
		double _akc {0};

		double _MAT {0};
		double _tref {0};
		double _c {0};
		double _d {0};
		double _Pt {0};

		double _Q10wsb {0};
		double _Q10wc {0};
		double _Q10wf {0};
		double _Q10wr {0};
		double _Q10sf {0};
		double _Q10sr {0};		
		double _Q10fm {0};		
		double _Q10a {0};		
		double _Q10c {0};

		double _kwsb {0};
		double _kwc {0};
		double _kwfe {0};
		double _kwfne {0};
		double _kwr {0};
		double _ksf {0};
		double _ksr {0};
		double _kfm {0};
		double _ka {0};
		double _kc {0};

		double computeAppliedDecayRate(double baseDecayRate, double q10);
	};
	
}}}
#endif