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
		double Q10wf() const { return _Q10wf; }
		double Q10wr() const { return _Q10wr; }
		double Q10sf() const { return _Q10sf; }
		double Q10sr() const { return _Q10sr; }		
		double Q10fm() const { return _Q10fm; }	
		double Q10a() const { return  _Q10a; }		
		double Q10c() const { return  _Q10c; }

		double Pt() const { return _Pt; }

		double kwsb() const { return _kwsb; }
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

		PeatlandDecayParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier);

		virtual ~PeatlandDecayParameters() = default;
		
		void setValue(const DynamicObject& data) override;		

		void updateMeanAnnualTemperature(const DynamicObject& data, double meanAnnualTemperature);

		double computeAppliedDecayRate(double baseDecayRate, double meanAnnualTemperature, double tref, double q10);

	private:
		double _akwsb;
		double _akwfe;	
		double _akwfne;
		double _akwr;
		double _aksf;
		double _aksr;
		double _akfm;
		double _aka;
		double _akc;

		double _MAT;
		double _tref;
		double _c;
		double _d;
		double _Pt;

		double _Q10wsb;
		double _Q10wf;
		double _Q10wr;
		double _Q10sf;
		double _Q10sr;		
		double _Q10fm;		
		double _Q10a;		
		double _Q10c;

		double _kwsb;
		double _kwfe;
		double _kwfne;
		double _kwr;
		double _ksf;
		double _ksr;
		double _kfm;
		double _ka;
		double _kc;

		std::vector<double> defaultDecayParamer;
	};
	
}}}
#endif