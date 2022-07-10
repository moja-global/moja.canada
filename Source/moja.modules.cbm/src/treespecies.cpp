#include "moja/flint/variable.h"

#include "moja/modules/cbm/treespecies.h"

namespace moja {
namespace modules {
namespace cbm {

    /**
     * Constructor
     * 
     * Initialise TreeSpecies._speciesID as  speciesID, \n
     * TreeSpecies._speciesType as speciesType
     *  
     * @param int speciesID
     * @param SpeciesType speciesType
     * **********************/
    TreeSpecies::TreeSpecies(int speciesID, SpeciesType speciesType) {
        _speciesID = speciesID;
        _speciesType = speciesType;
    }

    /**
     * Return TreeSpecies._speciesID
     * 
     * @return int
     * ***************/
    int TreeSpecies::speciesID() const {
        return _speciesID;
    }

    /**
     * Return TreeSpecies._speciesType
     * 
     * @return SpeciesType
     * ******************/
    SpeciesType TreeSpecies::speciesType() const {
        return _speciesType;
    }

}}}