#include "moja/flint/variable.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include <numeric>

namespace moja {
namespace modules {
namespace CBM {

	TreeYieldTable::TreeYieldTable() : _maxAge(0), _ageInterval(0) {}

	TreeYieldTable::TreeYieldTable(const std::vector<DynamicObject>& yieldTable,
								   SpeciesType speciesType) : _maxAge(0), _speciesType(speciesType) {

		auto rowCount = yieldTable.size();
		if (rowCount == 0)
			return;

		// get the last row of data
		const auto& lastRow = std::max_element(
				yieldTable.begin(), yieldTable.end(),
				[](const DynamicObject& r1, const DynamicObject& r2)->bool {
					return r1["age"] < r2["age"]; 
		});

		_maxAge = (*lastRow)["age"];

		// the first row has age = 0
		_ageInterval = _maxAge / int(rowCount - 1);

		// initialize age=volume data
		Initialize(yieldTable);
	}

	double TreeYieldTable::operator[](int age) const { 
		return _yieldsAtEachAge[age]; 
	}

	void TreeYieldTable::Initialize(const std::vector<DynamicObject>& yieldTable) {
		_yieldsAtEachAge.resize(_maxAge + 1);
		for (auto& row : yieldTable) {
			int age = row["age"];
			double volume = row["merchantable_volume"];
			_yieldsAtEachAge[age] = volume;
		}

		preProcessYieldData();

		InterpolateVolumeAtEachAge();

		_totalVolume = std::accumulate(_yieldsAtEachAge.begin(), _yieldsAtEachAge.end(), 0.0);		
	}

	void TreeYieldTable::InterpolateVolumeAtEachAge() {	
		int x0 = 0;
		int x1 = 0;
		double y0 = 0.0;
		double y1 = 0.0;

		for (int i = 0; i < _maxAge; i += _ageInterval) {
			x0 = i;
			x1 = i + _ageInterval;
			y0 = _yieldsAtEachAge[x0];
			y1 = _yieldsAtEachAge[x1];		

			for (int age = x0; age < x1; age++) {
				_yieldsAtEachAge[age] = linear(age, x0, x1, y0, y1);					
			}
		}
	}

	void TreeYieldTable::preProcessYieldData(){
		//for each yield table, it must have the valid data for each age up to the maximum age
		if (!_yieldsAtEachAge.empty()){				
			for (int age = 0; age < _maxAge; age++) {
				if (_yieldsAtEachAge[age] != _yieldsAtEachAge[age + 1]){
					if (_yieldsAtEachAge[age + 1] == 0)
						_yieldsAtEachAge[age + 1] = _yieldsAtEachAge[age];
				}
			}
		}		
	}
}}}