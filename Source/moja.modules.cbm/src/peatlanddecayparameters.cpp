#include "moja/modules/cbm/peatlanddecayparameters.h"

namespace moja {
namespace modules {
namespace cbm {

	PeatlandDecayParameters::PeatlandDecayParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier) :
		PeatlandParameters(_spuId, _peatlandType, _peatlandTreeClassifier) {		
	}
	
	/// <summary>
	/// Set the data from the transform result data row
	/// </summary>
	/// <param name="data"></param>
	void PeatlandDecayParameters::setValue(const DynamicObject& data) {
		_kwsb = data["kwsb"];
		_kwfe = data["kwfe"];
		_kwfne = data["kwfne"];
		_kwr = data["kwr"];
		_ksf = data["ksf"];
		_ksr = data["ksr"];
		_kfm = data["kfm"];
		_ka = data["ka"];
		_kc = data["kc"];

		_Q10wsb = data["Q10wsb"];
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

		/*
		double _kwsb = data["kwsb"];
		double _kwfe = data["kwfe"];	
		double _kwfne = data["kwfne"];
		double _kwr = data["kwr"];
		double _ksf = data["ksf"];
		double _ksr = data["ksr"];
		double _kfm = data["kfm"];
		double _ka = data["ka"];
		double _kc = data["kc"];

		_akwsb = computeAppliedDecayRate(_kwsb, _MAT, _tref, _Q10wsb);
		_akwfe = computeAppliedDecayRate(_kwfe, _MAT, _tref, _Q10wf);
		_akwfne = computeAppliedDecayRate(_kwfne, _MAT, _tref, _Q10wf);
		_akwr = computeAppliedDecayRate(_kwr, _MAT, _tref, _Q10wr);
		_aksf = computeAppliedDecayRate(_ksf, _MAT, _tref, _Q10sf);
		_aksr = computeAppliedDecayRate(_ksr, _MAT, _tref, _Q10sr);
		_akfm = computeAppliedDecayRate(_kfm, _MAT, _tref, _Q10fm);
		_aka = computeAppliedDecayRate(_ka, _MAT, _tref, _Q10a);
		_akc = computeAppliedDecayRate(_kc, _MAT, _tref, _Q10c);

		*/
	}
	
	double PeatlandDecayParameters::computeAppliedDecayRate(double baseDecayRate, double meanAnnualTemperature, double tref, double q10){
		double value = 0;
		value = baseDecayRate * (exp((meanAnnualTemperature - tref) * (log(q10) * 0.1)));
		return value;
	}	

	void PeatlandDecayParameters::updateMeanAnnualTemperature(const DynamicObject& data, double meanAnnualTemperature) {	
		_MAT = meanAnnualTemperature;

		_akwsb = computeAppliedDecayRate(_kwsb, _MAT, _tref, _Q10wsb);
		_akwfe = computeAppliedDecayRate(_kwfe, _MAT, _tref, _Q10wf);
		_akwfne = computeAppliedDecayRate(_kwfne, _MAT, _tref, _Q10wf);
		_akwr = computeAppliedDecayRate(_kwr, _MAT, _tref, _Q10wr);
		_aksf = computeAppliedDecayRate(_ksf, _MAT, _tref, _Q10sf);
		_aksr = computeAppliedDecayRate(_ksr, _MAT, _tref, _Q10sr);
		_akfm = computeAppliedDecayRate(_kfm, _MAT, _tref, _Q10fm);
		_aka = computeAppliedDecayRate(_ka, _MAT, _tref, _Q10a);
		_akc = computeAppliedDecayRate(_kc, _MAT, _tref, _Q10c);
	}
}}}
