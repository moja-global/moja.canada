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
		_Q10wsb = data["Q10"];
		_Q10wf = data["Q10"];
		_Q10wr = data["Q10"];
		_Q10sf = data["Q10"];
		_Q10sr = data["Q10"];		
		_Q10fm = data["Q10"];		
		_Q10a = data["Q10"];		
		_Q10c = data["Q10"];

		_MAT = data["MAT"];
		_tref = data["tref"];
		_c = data["c"];
		_d = data["d"];
		_Pt = data["Pt"];

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
	}
	
	double PeatlandDecayParameters::computeAppliedDecayRate(double baseDecayRate, double meanAnnualTemperature, double tref, double q10){
		double value = 0;
		value = baseDecayRate * (exp((meanAnnualTemperature - tref) * (log(q10) * 0.1)));
		return value;
	}

	void PeatlandDecayParameters::setDefaultValue(const std::vector<double>& data){

	}

}}}