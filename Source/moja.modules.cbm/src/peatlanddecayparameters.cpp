#include "moja/modules/cbm/peatlanddecayparameters.h"

namespace moja {
namespace modules {
namespace cbm {

	PeatlandDecayParameters::PeatlandDecayParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType) :
		PeatlandParameters(_spuId, _peatlandType, _landCoverType) {		
	}
	
	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
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

	double PeatlandDecayParameters::computeAppliedDecayRate (double baseDecayRate, double q10) {
		double value = 0;
		value = baseDecayRate * (exp((_MAT - _tref) * (log(q10) * 0.1)));		
		return value;
	}		
}}}
