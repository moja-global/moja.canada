#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/treespecies.h"

namespace moja {
namespace modules {
namespace CBM {	
	TreeSpecies::TreeSpecies(int speciesID, SpeciesType speciesType){				
		_speciesID = speciesID;
		_speciesType = speciesType;
	}

	 int TreeSpecies::speciesID() const {
		return _speciesID;
	}

	 SpeciesType TreeSpecies::speciesType() const {
		 return _speciesType;
	 }
}}}