#include "moja/modules/cbm/standcomponent.h"

#include <moja/flint/ivariable.h>

namespace moja {
namespace modules {
namespace cbm {

    double StandComponent::calculateRootBiomass() const {
        auto agIncrements = getAGIncrements();
        double totalAGBiomass =
            _merch->value() + agIncrements[_merch->name()] +
            _other->value() + agIncrements[_other->name()] +
            _foliage->value() + agIncrements[_foliage->name()];

        return _rootBiomassEquation->calculateRootBiomass(totalAGBiomass);
    }

    std::unordered_map<std::string, double> StandComponent::getIncrements(double standRootBiomass) const {
        auto agIncrements = getAGIncrements();
        double rootCarbon = _rootBiomassEquation->biomassToCarbon(calculateRootBiomass());
        auto rootProps = _rootBiomassEquation->calculateRootProportions(standRootBiomass);

        return std::unordered_map<std::string, double>{
            { _merch->name(), agIncrements[_merch->name()] },
            { _other->name(), agIncrements[_other->name()] },
            { _foliage->name(), agIncrements[_foliage->name()] },
            { _fineRoots->name(), rootCarbon * rootProps.fine - _fineRoots->value() },
            { _coarseRoots->name(), rootCarbon * rootProps.coarse - _coarseRoots->value() }
        };
    }

    std::unordered_map<std::string, double> StandComponent::getAGIncrements() const {
        int age = _age->value();
        double merch = _merch->value();
        double other = _other->value();
        double foliage = _foliage->value();

        // Return either the increment or the remainder of the pool value, if
        // the increment would result in a negative pool value.
        return std::unordered_map<std::string, double>{
            { _merch->name(), std::max(_growthCurve->getMerchCarbonIncrement(age), -merch) },
            { _other->name(), std::max(_growthCurve->getOtherCarbonIncrement(age), -other) },
            { _foliage->name(), std::max(_growthCurve->getFoliageCarbonIncrement(age), -foliage) }
        };
    }

    std::vector<double> StandComponent::getAboveGroundCarbonCurve() const {
        return _growthCurve->getAboveGroundCarbonCurve();
    }

}}}
