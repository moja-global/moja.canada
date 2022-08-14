/**
 * @file
 * Initialise and perform operations on the tree yield table
 * 
 * *************************/
#include "moja/flint/variable.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include <numeric>

namespace moja {
namespace modules {
namespace cbm {

    /**
     * Overloaded Constructor
     * 
     * Initialise TreeYieldTable._maxAge to 0, TreeYieldTable._ageInterval to 0
     * *********************/
    TreeYieldTable::TreeYieldTable() : _maxAge(0), _ageInterval(0) {}

    /**
     * Overloaded Constructor
     * 
     * Initialise TreeYieldTable._maxAge to 0, TreeYieldTable._speciesType to parameter speciesType \n,
     * If yieldTable is not empty, initialise TreeYieldTable._maxAge as the last value of "age" in parameter yieldTable, \n
     * TreeYieldTable._ageInterval as the difference between the second and first "age" values in parameter yieldTable \n
     * Invoke TreeYieldTable.Initialize() with paramter yieldTable
     * @param yieldTable const vector<DynamicObject>&
     * @param speciesType SpeciesType
     * *********************/
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

    /**
     * Return value of parameter age in TreeYieldTable._yieldsAtEachAge
     * 
     * @param age int
     * @return double
     * *******************/
    double TreeYieldTable::operator[](int age) const { 
        return _yieldsAtEachAge.at(age); 
    }

    /**
     * Initialise the yield table
     * 
     * Resize TreeYieldTable._yieldsAtEachAge to TreeYieldTable._maxAge + 1 and initialise it to 9 \n
     * Add the value of "merchantable_volume" attribute corresponding to each in parameter yieldTable to TreeYieldTable._yieldsAtEachAge \n
     * More than one value in the yield table for a given age means tha there are multiple hardwood or softwood species components - CBM3 Toolbox behaviour is to add the yields together \n
     * Invoke TreeYieldTable.preProcessYieldData() and TreeYieldTable.InterpolateVolumeAtEachAge()
     * Assign the sum of TreeYieldTable._yieldsAtEachAge to TreeYieldTable._totalVolume
     * 
     * @param yieldTable const vector<DynamicObject>&
     * @return void
     * ****************************/
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

    /**
     * For each value in the range 0 to TreeYieldTable._maxAge separated by TreeYieldTable._ageInterval, 
     * assign x0 as value, x1 as value + TreeYieldTable._ageInterval, y0 as value of x0 in TreeYieldTable._yieldsAtEachAge, y1 as value of x1 in TreeYieldTable._yieldsAtEachAge \n
     * For each age in the range x0 to x1, assign the result of the interpolation using TreeYieldTable.linear() 
     * with arguments age, x0, x1, y0, y1
     * 
     * @return void 
     * **************************/
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

    /**
     * If TreeYieldTable.yieldsAtEachAge is not empty, for ages in the range 0 and TreeYieldTable._maxAge, 
     * if the value of the current age in TreeYieldTable._yieldsAtEachAge is not equal to the value of age+1 in TreeYieldTable._yieldsAtEachAge and
     * value of age+1 in TreeYieldTable._yieldsAtEachAge is 0, set the value of age+1 equal to the value of age in TreeYieldTable._yieldsAtEachAge
     * 
     * @return void
     * ***************************/
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


@htmlonly
 <script type="text/javascript">
 function displayDate()
 {
 document.getElementById("demo").innerHTML=Date();
 }
 </script>
 <div id="demo">date</div>
 <script type="text/javascript">displayDate();</script>
 <br/>
 <button type="button" onclick="displayDate()">Refresh Date</button>
@endhtmlonly