#ifndef MOJA_MODULES_CBM_COMPONENTBIOMASSCARBONCURVE_H_
#define MOJA_MODULES_CBM_COMPONENTBIOMASSCARBONCURVE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {

	/*
	* ADT - component carbon curve (softwood and hardwood components)
	*/
	class CBM_API ComponentBiomassCarbonCurve{
	public:
		ComponentBiomassCarbonCurve() {};
		virtual ~ComponentBiomassCarbonCurve() {};	

		ComponentBiomassCarbonCurve(int maxAge);
	
		double getMerchCarbonIncrement(int age) const;
		double getFoliageCarbonIncrement(int age) const;
		double getOtherCarbonIncrement(int age) const;

		double getMerchCarbonAtAge(int age) const;
		double getFoliageCarbonAtAge(int age) const;
		double getOtherCarbonAtAge(int age) const;

		void setMerchCarbonAtAge(int age, double value);
		void setFoliageCarbonAtAge(int age, double value);
		void setOtherCarbonAtAge(int age, double value);
	private:	
		int _maxAge;
		std::vector<double> _merchCarbonIncrements;		
		std::vector<double> _foliageCarbonIncrements;
		std::vector<double> _otherCarbonIncrements;
	};

	inline double ComponentBiomassCarbonCurve::getMerchCarbonIncrement(int age) const {
		return (age >= _maxAge ? 0 : (_merchCarbonIncrements[age + 1] - _merchCarbonIncrements[age]));
		//return _merchCarbonIncrements[age + 1] - _merchCarbonIncrements[age];
	}

	inline double ComponentBiomassCarbonCurve::getFoliageCarbonIncrement(int age) const {
		return (age >= _maxAge ? 0 : (_foliageCarbonIncrements[age + 1] - _foliageCarbonIncrements[age]));
	}

	inline double ComponentBiomassCarbonCurve::getOtherCarbonIncrement(int age) const {
		return (age >= _maxAge ? 0 : (_otherCarbonIncrements[age + 1] - _otherCarbonIncrements[age]));
	}

	inline void ComponentBiomassCarbonCurve::setMerchCarbonAtAge(int age, double value) {
		_merchCarbonIncrements[age] = value;
	};

	inline void ComponentBiomassCarbonCurve::setFoliageCarbonAtAge(int age, double value) {
		_foliageCarbonIncrements[age] = value;
	};

	inline void ComponentBiomassCarbonCurve::setOtherCarbonAtAge(int age, double value) {
		_otherCarbonIncrements[age] = value;
	};

	inline double ComponentBiomassCarbonCurve::getMerchCarbonAtAge(int age) const {
		return  (age > _maxAge ? _merchCarbonIncrements[_maxAge] : _merchCarbonIncrements[age]);
	};

	inline double ComponentBiomassCarbonCurve::getFoliageCarbonAtAge(int age) const {
		return (age > _maxAge ? _foliageCarbonIncrements[_maxAge] : _foliageCarbonIncrements[age]);
	};

	inline double ComponentBiomassCarbonCurve::getOtherCarbonAtAge(int age) const {
		return  (age > _maxAge ? _otherCarbonIncrements[_maxAge] : _otherCarbonIncrements[age]);
	};
}}}
#endif