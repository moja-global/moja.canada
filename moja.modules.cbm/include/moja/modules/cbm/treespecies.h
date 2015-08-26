#ifndef MOJA_MODULES_CBM_TREESPECIES_H_
#define MOJA_MODULES_CBM_TREESPECIES_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// Enumration of tree speciese type, softwoor or hardwood
	/// </summary>
	enum class CBM_API SpeciesType { Softwood = 1, Hardwood };

	class CBM_API TreeSpecies{

	public:
		TreeSpecies(){}
		virtual ~TreeSpecies(){ }		

		TreeSpecies(int speciesID, SpeciesType speciesType);

		int speciesID() const;
		SpeciesType speciesType() const;	

	private:
		int _speciesID;
		SpeciesType _speciesType;
	};
}}}
#endif