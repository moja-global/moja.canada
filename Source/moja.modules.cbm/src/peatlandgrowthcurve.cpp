#include "moja/modules/cbm/peatlandgrowthcurve.h"


namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Static data member to store peatland woody layer shrub growth curves
			 **********************/
			static std::unordered_map<int, std::vector<double> > peatlandCurves;

			/**
			 * @brief setup shrub woody layer growth curve
			 * For each row in parameter data, add "carbon" value into PeatlandGrowthcurve._woodyTotal.
			 *
			 * @param data vector
			 * @return void
			 **********************/
			void PeatlandGrowthcurve::setValue(const std::vector<DynamicObject>& data) {

				//assume it returns the table of [id, age, carbon] ordered by age 
				for (auto& row : data) {
					_growthCurveId = row["id"];
					int age = row["age"];
					double value = row["carbon"];
					_woodyTotal.push_back(value);
				}

				//here to store the curve
				PeatlandGrowthcurve::storeCurve(_growthCurveId, _woodyTotal);
			}


			/**
			 * @brief
			 *
			 * Assign size_t variable maxAge as PeatlandGrowthcurve_woodyTotal size - 1 \n
			 * if parameter age is greater than maxAge, assign size_t variable ageIndex as maxAge else ageIndex as age. \n
			 * If ageIndex is greater than 0, assign size_t variable ageIndexPre as ageIndex - 1 else ageIndexPre as  0.
			 * return the difference between ageIndex and ageIndexPre in PeatlandGrowthcurve._woodyTotal.
			 *
			 * @param age int
			 * @return double
			 * *******************/
			double PeatlandGrowthcurve::getNetGrowthAtAge(int age) {
				size_t maxAge = _woodyTotal.size() - 1;
				size_t ageIndex = age > maxAge ? maxAge : age;
				size_t ageIndexPre = ageIndex > 0 ? (ageIndex - 1) : 0;

				return _woodyTotal[ageIndex] - _woodyTotal[ageIndexPre];
			}

			/**
			* @brief
			* For each peatland growth curve
			* store the curve vector data by peatlandId
			*
			* @param peatlandId int
			* @param data vector of double
			* @return void
			* *******************/
			void PeatlandGrowthcurve::storeCurve(int peatlandId, std::vector<double> data) {
				if (auto search = peatlandCurves.find(peatlandId); search != peatlandCurves.end()) {
					//curve already stored, just return
					return;
				}
				else {
					peatlandCurves[peatlandId] = data;
				}
			}

			/**
			* @brief
			* Find the minimum age to match a particular carbon value
			*
			* @param peatlandId int
			* @param woodyStemBranchCarbon double
			* @return minimumAge int
			* *******************/
			int PeatlandGrowthcurve::findTheMinumResetAge(int peatlandId, double woodyStemBranchCarbon) {
				int minimumAge = 0;
				double woodyGrowthCurveCarbonToStemBranchFactor = 0.734704;

				std::vector<double> curveData = peatlandCurves.at(peatlandId);

				std::vector<double> absDiff;
				for (auto& curveCarbonValue : curveData) {
					absDiff.push_back(abs(curveCarbonValue - woodyStemBranchCarbon / woodyGrowthCurveCarbonToStemBranchFactor));
				}

				minimumAge += std::distance(std::begin(absDiff), std::min_element(std::begin(absDiff), std::end(absDiff)));

				return minimumAge;
			}
		}
	}
}