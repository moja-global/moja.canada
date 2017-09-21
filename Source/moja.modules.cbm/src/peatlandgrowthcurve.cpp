#include "moja/modules/cbm/peatlandgrowthcurve.h"

namespace moja {
namespace modules {
namespace cbm {
	
	void PeatlandGrowthcurve::setValue(const std::vector<DynamicObject>& data) {
		//assume it returns the table of [age, carbon] ordered by age 
		
		for (auto& row : data) {
			int age = row["age"];
			double value = row["carbon"];
			_woodyTotal.push_back(value);
		}			
	}


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


	double PeatlandGrowthcurve::getNetGrowthAtAge(int age){
		size_t maxAge = _woodyTotal.size() - 1;
		size_t ageIndex = age > maxAge ? maxAge : age;
		size_t ageIndexPre = ageIndex > 0 ? (ageIndex - 1) : 0;

		return _woodyTotal[ageIndex] - _woodyTotal[ageIndexPre];
	}
		
}}}