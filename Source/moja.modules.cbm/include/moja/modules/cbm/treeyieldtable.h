#ifndef MOJA_MODULES_CBM_YIELDTABLE_H_
#define MOJA_MODULES_CBM_YIELDTABLE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/treespecies.h"

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// ADT - Tree yield table
	/// </summary>
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

		/*
		Indexer to get the volume at each age
		*/
		double operator[](int age) const;
		
		/*
		*Get the species type for this yield table
		*/
		SpeciesType speciesType() const { return _speciesType; }

		/// <summary>
		/// Create a yield table for a specified type with the transform result yield data
		/// </summary>
		/// <param name="yieldTable">It has [Age, Volume]</param>      
		/// <param name="specieseType">species type, either softwood or hardwood</param>   
		TreeYieldTable(const std::vector<DynamicObject>& yieldTable,
					   SpeciesType speciesType);
	private:
		/// <summary>
		/// Load original yield table data[age, volume] 
		/// Interpolate to get volume at each age
		/// Calculate the maximum volume
		/// </summary>
		/// <param name="rows"></param>
		void Initialize(const std::vector<DynamicObject>& yieldTable);

		/// <summary>
		/// Lineraly Interpolate the tree yield table to assign the volume at each age year
		/// </summary>             
		void InterpolateVolumeAtEachAge();

		/// <summary>
		/// There should be a volume data at each age. 
		/// The yield table is not valid if there are ZERO values after the last valid non-zero volume. 
		/// For example, the volume data like[0,0,0,0,10,20,30,50,60,70,0,0,0,0]. 
		/// In current CBM, all the trialling ZEROs should be replaced by the last non-zero data - 70.
		/// The volume data like[0,0,0,0,10,20,30,50,60,70,70,70,70,70].
		/// </summary>             
		void preProcessYieldData();

		/// <summary>
		/// http://stackoverflow.com/questions/12838007/c-sharp-linear-interpolation
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