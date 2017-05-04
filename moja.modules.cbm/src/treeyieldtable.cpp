#include "moja/flint/variable.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include <numeric>

namespace moja {
namespace modules {
namespace cbm {

    TreeYieldTable::TreeYieldTable() : _maxAge(0), _ageInterval(0) {}

    TreeYieldTable::TreeYieldTable(const std::vector<DynamicObject>& yieldTable,
                                   SpeciesType speciesType) : _maxAge(0), _speciesType(speciesType) {

        auto rowCount = yieldTable.size();
        if (rowCount == 0) {
            return;
        }

		std::set<int> ages;
		for (auto& row : yieldTable) {
			ages.insert(row["age"].convert<int>());
		}

		_maxAge = *(--ages.end());
		auto agesIter = ages.begin();
		int firstAge = *agesIter;
		int nextAge = *(++agesIter);
		_ageInterval = nextAge - firstAge;

        // Initialize age=volume data.
        Initialize(yieldTable);
    }

    double TreeYieldTable::operator[](int age) const { 
        return _yieldsAtEachAge.at(age); 
    }

    void TreeYieldTable::Initialize(const std::vector<DynamicObject>& yieldTable) {
        _yieldsAtEachAge.resize(_maxAge + 1);
		for (int i = 0; i < _yieldsAtEachAge.size(); i++) {
			_yieldsAtEachAge[i] = 0;
		}

        for (auto& row : yieldTable) {
            int age = row["age"];
            double volume = row["merchantable_volume"];
			// More than one value in the yield table for a given age means that
			// there are multiple hardwood or softwood species components - CBM3
			// Toolbox behaviour is to add the yields together, so we do the same
			// here.
            _yieldsAtEachAge.at(age) += volume;
        }

		preProcessYieldData();
        InterpolateVolumeAtEachAge();
        _totalVolume = std::accumulate(_yieldsAtEachAge.begin(),
                                       _yieldsAtEachAge.end(),
                                       0.0);
    }

    void TreeYieldTable::InterpolateVolumeAtEachAge() {	
        int x0 = 0;
        int x1 = 0;
        double y0 = 0.0;
        double y1 = 0.0;

		for (int i = 0; i < _maxAge; i += _ageInterval) {
            x0 = i;
            x1 = i + _ageInterval;
            if (x1 > _maxAge) {
                x1 = _maxAge;
            }

            y0 = _yieldsAtEachAge.at(x0);
            y1 = _yieldsAtEachAge.at(x1);		

            for (int age = x0; age < x1; age++) {
                _yieldsAtEachAge.at(age) = linear(age, x0, x1, y0, y1);					
            }
        }
    }

    void TreeYieldTable::preProcessYieldData() {
        // For each yield table, it must have the valid data for each age up to
        // the maximum age.
        if (_yieldsAtEachAge.empty()) {
            return;
        }

        for (int age = 0; age < _maxAge; age++) {
            if (_yieldsAtEachAge.at(age) != _yieldsAtEachAge.at(age + 1)) {
                if (_yieldsAtEachAge.at(age + 1) == 0) {
                    _yieldsAtEachAge.at(age + 1) = _yieldsAtEachAge.at(age);
                }
            }
        }
    }

}}}