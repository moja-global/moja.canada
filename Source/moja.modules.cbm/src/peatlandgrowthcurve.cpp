#include "moja/modules/cbm/peatlandgrowthcurve.h"

namespace moja {
namespace modules {
namespace cbm {

	 /**
	 * For each row in parameter data, add "carbon" value into PeatlandGrowthcurve._woodyTotal.
	 * 
	 * @param data const vector<DynamicObject>&
	 * @return void
	 * *********************/
	void PeatlandGrowthcurve::setValue(const std::vector<DynamicObject>& data) {
		//assume it returns the table of [age, carbon] ordered by age 
		
		for (auto& row : data) {
			int age = row["age"];
			double value = row["carbon"];
			_woodyTotal.push_back(value);
		}			
	}

	 /**
     * Add the data parameter into PeatlandGrowthcurve._woodyTotal.
	 * 
	 * @return void
     * *******************/
	void PeatlandGrowthcurve::setValue(const DynamicObject& data){
		//assume it returns data from configuration file
		size_t size = data.size();

		std::string key;
		for (int i = 0; i < size; i++){
			key = "a" + std::to_string(i);
			double value = data[key];
			_woodyTotal.push_back(value);
		}
	}

	/**
	 * Assign size_t variable maxAge as PeatlandGrowthcurve_woodyTotal size - 1 \n
	 * if parameter age is greater than maxAge, assign size_t variable ageIndex as maxAge else ageIndex as age. \n
	 * If ageIndex is greater than 0, assign size_t variable ageIndexPre as ageIndex - 1 else ageIndexPre as  0.
	 * return the difference between ageIndex and ageIndexPre in PeatlandGrowthcurve._woodyTotal.
	 * @param age int
	 * @return double
     * *******************/
	double PeatlandGrowthcurve::getNetGrowthAtAge(int age){
		size_t maxAge = _woodyTotal.size() - 1;
		size_t ageIndex = age > maxAge ? maxAge : age;
		size_t ageIndexPre = ageIndex > 0 ? (ageIndex - 1) : 0;

		return _woodyTotal[ageIndex] - _woodyTotal[ageIndexPre];
	}
		
}}}