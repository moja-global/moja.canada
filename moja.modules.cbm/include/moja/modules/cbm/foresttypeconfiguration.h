#ifndef MOJA_MODULES_CBM_FORESTTYPECONFIGURATION_H_
#define MOJA_MODULES_CBM_FORESTTYPECONFIGURATION_H_

#include "moja/flint/ipool.h"
#include "moja/flint/ivariable.h"
#include "moja/modules/cbm/rootbiomassequation.h"

namespace cbm = moja::modules::cbm;
namespace flint = moja::flint;

struct CBM_API ForestTypeConfiguration {
    std::string forestType;
    flint::IVariable* age;
    std::shared_ptr<cbm::RootBiomassEquation> rootBiomassEquation;
    flint::IPool::ConstPtr merch;
    flint::IPool::ConstPtr other;
    flint::IPool::ConstPtr foliage;
    flint::IPool::ConstPtr coarseRoots;
    flint::IPool::ConstPtr fineRoots;
};

#endif