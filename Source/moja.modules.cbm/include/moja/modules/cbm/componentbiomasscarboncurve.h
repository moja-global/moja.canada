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
	class CBM_API ComponentBiomassCarbonCurve {
	public:
		ComponentBiomassCarbonCurve() {};
		virtual ~ComponentBiomassCarbonCurve() = default;	

		ComponentBiomassCarbonCurve(int maxAge);
	
		double getMerchCarbonIncrement(int age) const;
		double getFoliageCarbonIncrement(int age) const;
		double getOtherCarbonIncrement(int age) const;

		double getMerchCarbonAtAge(int age) const;
		double getFoliageCarbonAtAge(int age) const;
		double getOtherCarbonAtAge(int age) const;

        std::vector<double> getAboveGroundCarbonCurve() const;

        const std::vector<double>& getMerchCarbonCurve() const;
        const std::vector<double>& getFoliageCarbonCurve() const;
        const std::vector<double>& getOtherCarbonCurve() const;

		void setMerchCarbonAtAge(int age, double value);
		void setFoliageCarbonAtAge(int age, double value);
		void setOtherCarbonAtAge(int age, double value);

	private:	
		int _maxAge = 1;
		std::vector<double> _merchCarbonIncrements;		
		std::vector<double> _foliageCarbonIncrements;
		std::vector<double> _otherCarbonIncrements;
	};

	inline double ComponentBiomassCarbonCurve::getMerchCarbonIncrement(int age) const {
		if (age >= _merchCarbonIncrements.size() - 1) {
			return 0;
		}

		int nextAge = age + 1;
		return _merchCarbonIncrements[nextAge] - _merchCarbonIncrements[age];
	}

	inline double ComponentBiomassCarbonCurve::getFoliageCarbonIncrement(int age) const {
		if (age >= _foliageCarbonIncrements.size() - 1) {
			return 0;
		}

		int nextAge = age + 1;
        return _foliageCarbonIncrements[nextAge] - _foliageCarbonIncrements[age];
	}

	inline double ComponentBiomassCarbonCurve::getOtherCarbonIncrement(int age) const {
		if (age >= _otherCarbonIncrements.size() - 1) {
			return 0;
		}

		int nextAge = age + 1;
        return _otherCarbonIncrements[nextAge] - _otherCarbonIncrements[age];
	}

	inline void ComponentBiomassCarbonCurve::setMerchCarbonAtAge(int age, double value) {
		if (_merchCarbonIncrements.size() < age + 1) {
			_merchCarbonIncrements.resize(age + 1, 0);
		}

		_merchCarbonIncrements[age] = value;
	};

	inline void ComponentBiomassCarbonCurve::setFoliageCarbonAtAge(int age, double value) {
		if (_foliageCarbonIncrements.size() < age + 1) {
			_foliageCarbonIncrements.resize(age + 1, 0);
		}

		_foliageCarbonIncrements[age] = value;
	};

	inline void ComponentBiomassCarbonCurve::setOtherCarbonAtAge(int age, double value) {
		if (_otherCarbonIncrements.size() < age + 1) {
			_otherCarbonIncrements.resize(age + 1, 0);
		}

		_otherCarbonIncrements[age] = value;
	};

	inline double ComponentBiomassCarbonCurve::getMerchCarbonAtAge(int age) const {
        return age >= _merchCarbonIncrements.size() ? _merchCarbonIncrements[_maxAge]
            : _merchCarbonIncrements[age];
	};

	inline double ComponentBiomassCarbonCurve::getFoliageCarbonAtAge(int age) const {
        return age >= _foliageCarbonIncrements.size() ? _foliageCarbonIncrements[_maxAge]
            : _foliageCarbonIncrements[age];
	};

	inline double ComponentBiomassCarbonCurve::getOtherCarbonAtAge(int age) const {
        return age >= _otherCarbonIncrements.size() ? _otherCarbonIncrements[_maxAge]
            : _otherCarbonIncrements[age];
	};

    inline std::vector<double> ComponentBiomassCarbonCurve::getAboveGroundCarbonCurve() const {
        std::vector<double> agCarbon;
        for (int i = 0; i < _maxAge; i++) {
            agCarbon.push_back(
                  _merchCarbonIncrements[i]
                + _foliageCarbonIncrements[i]
                + _otherCarbonIncrements[i]);
        }

        return agCarbon;
    }

    inline const std::vector<double>& ComponentBiomassCarbonCurve::getMerchCarbonCurve() const {
        return _merchCarbonIncrements;
    }

    inline const std::vector<double>& ComponentBiomassCarbonCurve::getFoliageCarbonCurve() const {
        return _foliageCarbonIncrements;
    }

    inline const std::vector<double>& ComponentBiomassCarbonCurve::getOtherCarbonCurve() const {
        return _otherCarbonIncrements;
    }

}}}
#endif