#include "moja/flint/variable.h"

#include "moja/modules/cbm/treespecies.h"

namespace moja {
namespace modules {
namespace cbm {

    TreeSpecies::TreeSpecies(int speciesID, SpeciesType speciesType) {
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