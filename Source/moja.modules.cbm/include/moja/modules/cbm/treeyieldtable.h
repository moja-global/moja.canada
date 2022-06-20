#ifndef MOJA_MODULES_CBM_YIELDTABLE_H_
#define MOJA_MODULES_CBM_YIELDTABLE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/treespecies.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API TreeYieldTable {
	public:
		TreeYieldTable();
		virtual ~TreeYieldTable() = default;

		/// <summary>
		/// Maximum age of the yield table
		/// </summary>
		int maxAge() const { return _maxAge; };

		/// <summary>
		/// Total volume of the speciese growth curve
		/// </summary>
		double totalVolume() const { return _totalVolume; };

		/// <summary>
		/// Vector of volume (m^3/ha) at each age stop, 1 ha = 10,000 m^2
		/// </summary>
		std::vector<double>& yieldsAtEachAge() { return _yieldsAtEachAge; };


		double operator[](int age) const;
		
		/*
		*Get the species type for this yield table
		*/
		SpeciesType speciesType() const { return _speciesType; }

	
		TreeYieldTable(const std::vector<DynamicObject>& yieldTable,
					   SpeciesType speciesType);
	private:

		void Initialize(const std::vector<DynamicObject>& yieldTable);
           
		void InterpolateVolumeAtEachAge();
             
		void preProcessYieldData();

		/// <summary>
		/// Resource for interpolation : http://stackoverflow.com/questions/12838007/c-sharp-linear-interpolation
		/// </summary>
		double linear(double x, double x0, double x1, double y0, double y1);
		
		int _maxAge;
		int _ageInterval;
		double _totalVolume;
		SpeciesType _speciesType;
		std::vector<double> _yieldsAtEachAge;
	};

	inline double TreeYieldTable::linear(double x, double x0, double x1, double y0, double y1) {
		if ((x1 - x0) == 0) {
			return (y0 + y1) / 2;
		} else {
			return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
		}
	}

}}}
#endif