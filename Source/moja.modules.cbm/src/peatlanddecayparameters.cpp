/**
 * @file 
 *  
 * ******/
#include "moja/modules/cbm/peatlanddecayparameters.h"

namespace moja {
namespace modules {
namespace cbm {
    /**
    * Constructor.
	* 
	* 
    * @param _spuId int
	* @param _peatlandType PeatlandType
	* @param _landCoverType PeatlandLandCoverType
	* @return void
    * ************************/
	PeatlandDecayParameters::PeatlandDecayParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType) {		
	}
	
	/**
    * Set the data from the transform result data row.
	* 
	* Assign PeatlandDecayParameters._kwsb,PeatlandDecayParameters._kwc,PeatlandDecayParameters._kwfe, \n
	* PeatlandDecayParameters._kwfne,PeatlandDecayParameters._kwr,PeatlandDecayParameters._ksf, \n
	* PeatlandDecayParameters._ksr,PeatlandDecayParameters._kfm,PeatlandDecayParameters._ka,PeatlandDecayParameters._kc \n
	* PeatlandDecayParameters._Q10wsb,PeatlandDecayParameters._Q10wc,PeatlandDecayParameters._Q10wf,PeatlandDecayParameters._Q10wr, \n
	* PeatlandDecayParameters._Q10sf,PeatlandDecayParameters._Q10sr,PeatlandDecayParameters._Q10fm,PeatlandDecayParameters._Q10a, \n
	* PeatlandDecayParameters._Q10c,PeatlandDecayParameters._tref,PeatlandDecayParameters._c,PeatlandDecayParameters._d and PeatlandDecayParameters._Pt.
	* 
	* @param data DynamicObject&
    * @return void
    * ************************/
	void PeatlandDecayParameters::setValue(const DynamicObject& data) {
		_kwsb = data["kwsb"];
		_kwc = data["kwc"];
		_kwfe = data["kwfe"];
		_kwfne = data["kwfne"];
		_kwr = data["kwr"];
		_ksf = data["ksf"];
		_ksr = data["ksr"];
		_kfm = data["kfm"];
		_ka = data["ka"];
		_kc = data["kc"];

		_Q10wsb = data["Q10wsb"];
		_Q10wc = data["Q10wc"];
		_Q10wf = data["Q10wf"];
		_Q10wr = data["Q10wr"];
		_Q10sf = data["Q10sf"];
		_Q10sr = data["Q10sr"];		
		_Q10fm = data["Q10fm"];		
		_Q10a = data["Q10a"];		
		_Q10c = data["Q10c"];

		_tref = data["tref"];
		_c = data["c"];
		_d = data["d"];
		_Pt = data["Pt"];		
	}
	
	/**
    * .
	* 
	* Assign PeatlandDecayParameters._MAT as meanAnnualTemperature parameter. \n
	* Assign PeatlandDecayParameters._akwsb as computeAppliedDecayRate(PeatlandDecayParameters._kwsb,PeatlandDecayParameters._Q10wsb), \n
	* PeatlandDecayParameters._akwc as computeAppliedDecayRate(PeatlandDecayParameters._kwc, PeatlandDecayParameters._Q10wc), \n
	* PeatlandDecayParameters._akwfe as computeAppliedDecayRate(PeatlandDecayParameters._kwfe, PeatlandDecayParameters._Q10wf), \n
	* PeatlandDecayParameters._akwfne as computeAppliedDecayRate(PeatlandDecayParameters._kwfne, PeatlandDecayParameters._Q10wf), \n
	* PeatlandDecayParameters._akwr as computeAppliedDecayRate(PeatlandDecayParameters._kwr, PeatlandDecayParameters._Q10wr), \n
	* PeatlandDecayParameters._aksf as computeAppliedDecayRate(PeatlandDecayParameters._ksf, PeatlandDecayParameters._Q10sf), \n
	* PeatlandDecayParameters._aksr as computeAppliedDecayRate(PeatlandDecayParameters._ksr, PeatlandDecayParameters._Q10sr), \n
	* PeatlandDecayParameters._akfm as computeAppliedDecayRate(PeatlandDecayParameters._kfm, PeatlandDecayParameters._Q10fm), \n
	* PeatlandDecayParameters._aka as computeAppliedDecayRate(PeatlandDecayParameters._ka, PeatlandDecayParameters._Q10a), \n
	* PeatlandDecayParameters._akc as computeAppliedDecayRate(PeatlandDecayParameters._kc, PeatlandDecayParameters._Q10c)
	* 
	* @param meanAnnualTemperature double
    * @return void
    * ************************/
	void PeatlandDecayParameters::updateAppliedDecayParameters(double meanAnnualTemperature) {
		_MAT = meanAnnualTemperature;

		_akwsb = computeAppliedDecayRate(_kwsb, _Q10wsb);
		_akwc = computeAppliedDecayRate(_kwc, _Q10wc);
		_akwfe = computeAppliedDecayRate(_kwfe, _Q10wf);
		_akwfne = computeAppliedDecayRate(_kwfne, _Q10wf);
		_akwr = computeAppliedDecayRate(_kwr, _Q10wr);
		_aksf = computeAppliedDecayRate(_ksf, _Q10sf);
		_aksr = computeAppliedDecayRate(_ksr, _Q10sr);
		_akfm = computeAppliedDecayRate(_kfm, _Q10fm);
		_aka = computeAppliedDecayRate(_ka, _Q10a);
		_akc = computeAppliedDecayRate(_kc, _Q10c);
	}

	/**
    * Compute applied decay rate.
	* 
	* Compute the log of q10 paramater, mutiply it by 0.1 \n
	* Compute the exponential of (PeatlandDecayParameters._MAT - PeatlandDecayParameters._tref). \n
	* Multipy the log value,exponential value and baseDecayRate parameter. \n
	* return the value.
	* 
	* @param baseDecayRate double
	* @param q10 double
    * @return double
    * ************************/
	double PeatlandDecayParameters::computeAppliedDecayRate (double baseDecayRate, double q10) {
		double value = 0;
		value = baseDecayRate * (exp((_MAT - _tref) * (log(q10) * 0.1)));		
		return value;
	}		
}}}
