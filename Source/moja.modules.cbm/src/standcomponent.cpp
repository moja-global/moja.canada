#include "moja/modules/cbm/standcomponent.h"

#include <moja/flint/ivariable.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    double StandComponent::calculateRootBiomass(flint::ILandUnitDataWrapper* landUnitData) const
    {
        const flint::IVariable* age = landUnitData->getVariable("age");

        const flint::IPool* merch = landUnitData->getPool((boost::format("%1%Merch") % _forestType).str());
        const flint::IPool* other = landUnitData->getPool((boost::format("%1%Other") % _forestType).str());
        const flint::IPool* foliage = landUnitData->getPool((boost::format("%1%Foliage") % _forestType).str());

        auto agIncrements = getAGIncrements(age, merch, other, foliage);
        double totalAGBiomass =
            merch->value() + agIncrements[merch->name()] +
            other->value() + agIncrements[other->name()] +
            foliage->value() + agIncrements[foliage->name()];

        return _rootBiomassEquation->calculateRootBiomass(totalAGBiomass);
    }

    std::unordered_map<std::string, double> StandComponent::getIncrements(
        flint::ILandUnitDataWrapper* landUnitData,
        double standRootBiomass) const
    {
        const flint::IVariable* age = landUnitData->getVariable("age");

        const flint::IPool* merch = landUnitData->getPool((boost::format("%1%Merch") % _forestType).str());
        const flint::IPool* other = landUnitData->getPool((boost::format("%1%Other") % _forestType).str());
        const flint::IPool* foliage = landUnitData->getPool((boost::format("%1%Foliage") % _forestType).str());
        const flint::IPool* coarseRoots = landUnitData->getPool((boost::format("%1%CoarseRoots") % _forestType).str());
        const flint::IPool* fineRoots = landUnitData->getPool((boost::format("%1%FineRoots") % _forestType).str());

        auto agIncrements = getAGIncrements(age, merch, other, foliage);
        double rootCarbon = _rootBiomassEquation->biomassToCarbon(calculateRootBiomass(landUnitData));
        auto rootProps = _rootBiomassEquation->calculateRootProportions(standRootBiomass);

        return std::unordered_map<std::string, double>{
            { merch->name(), agIncrements[merch->name()] },
            { other->name(), agIncrements[other->name()] },
            { foliage->name(), agIncrements[foliage->name()] },
            { fineRoots->name(), rootCarbon * rootProps.fine - fineRoots->value() },
            { coarseRoots->name(), rootCarbon * rootProps.coarse - coarseRoots->value() }
        };
    }

    std::unordered_map<std::string, double> StandComponent::getAGIncrements(
        const flint::IVariable* age, const flint::IPool* merch, const flint::IPool* other,
        const flint::IPool* foliage) const
    {
        int ageValue = age->value();
        double merchValue = merch->value();
        double otherValue = other->value();
        double foliageValue = foliage->value();

        // Return either the increment or the remainder of the pool value, if
        // the increment would result in a negative pool value.
        return std::unordered_map<std::string, double>{
            { merch->name(), std::max(_growthCurve->getMerchCarbonIncrement(ageValue), -merchValue) },
            { other->name(), std::max(_growthCurve->getOtherCarbonIncrement(ageValue), -otherValue) },
            { foliage->name(), std::max(_growthCurve->getFoliageCarbonIncrement(ageValue), -foliageValue) }
        };
    }

    std::vector<double> StandComponent::getAboveGroundCarbonCurve() const {
        return _growthCurve->getAboveGroundCarbonCurve();
    }

    const std::vector<double>& StandComponent::getMerchCarbonCurve() const {
        return _growthCurve->getMerchCarbonCurve();
    }

    const std::vector<double>& StandComponent::getFoliageCarbonCurve() const {
        return _growthCurve->getFoliageCarbonCurve();
    }

    const std::vector<double>& StandComponent::getOtherCarbonCurve() const {
        return _growthCurve->getOtherCarbonCurve();
    }

}}}
