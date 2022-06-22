#include "moja/modules/cbm/standcomponent.h"

#include <moja/flint/ivariable.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    /**
     * Calculate the total above ground biomass from merchantable, foilage and other sources and return the root biomass
     * 
     * Assign variable age as value of variable "age" in parameter landUnitData, \n
     * merch as value of pool StandComponent._forestType + "Merch", other as value of pool StandComponent._forestType + "Other" and 
     * foilage as value of pool StandComponent._forestType + "Foilage" in parameter landUnitData \n
     * Assign the above ground biomass increments to variable agIncrements, the result of StandComponent.getAGIncrements() with arguments age, merch, other, foilage \n
     * The totalAGBiomass is given as the sum of values merch, other, foilage and the values of the name of merch, other and foilage in agIncrements
     * Return the result of StandComponent.calculateRootBiomass() with argument totalAGBiomass on StandComponent._rootBiomassEquation
     * 
     * @param landUnitData flint::ILandUnitDataWrapper*
     * @return double
     * ***********************************/
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

    /**
     * 
     * 
     * @param landUnitData flint::ILandUnitDataWrapper*
     * @param standRootBiomass double
     * @return unordered_map<std::string, double> 
     * ************************/
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

    /**
     * Return either the increment or the remainder of the pool values merchantable, foilage and other, if the increment results in a negative pool value
     * 
     * Assign variables ageValue, merchValue, otherValue, foilageValue the values of parameters age, merch, other and foilage \n
     * Return an unordered map with key as name of merch, value as the maximum of StandComponent.getMerchCarbonIncrement() on  StandComponent._growthCurve with parameter ageValue and the -1 * merchValue \n, 
     * name of other, value as the maximum of StandComponent.getMerchCarbonIncrement() on  StandComponent._growthCurve with parameter ageValue and the -1 * otherValue \n, 
     * name of foilage, value as the maximum of StandComponent.getMerchCarbonIncrement() on  StandComponent._growthCurve with parameter ageValue and the -1 * foilageValue  
     * 
     * @param age const flint::IVariable*
     * @param merch const flint::IPool*
     * @param other const flint::IPool*
     * @return unordered_map<std::string, double>
     * *****************************/
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

    /**
     * Return StandComponent.getAboveGroundCarbonCurve() on StandComponent._growthCurve
     * 
     * @return vector<double>&
     * ********************/
    std::vector<double> StandComponent::getAboveGroundCarbonCurve() const {
        return _growthCurve->getAboveGroundCarbonCurve();
    }

    /**
     * Return StandComponent.getMerchCarbonCurve() on StandComponent._growthCurve
     * 
     * @return vector<double>&
     * ********************/
    const std::vector<double>& StandComponent::getMerchCarbonCurve() const {
        return _growthCurve->getMerchCarbonCurve();
    }

    /**
     * Return StandComponent.getFoliageCarbonCurve() on StandComponent._growthCurve
     * 
     * @return vector<double>&
     * ********************/
    const std::vector<double>& StandComponent::getFoliageCarbonCurve() const {
        return _growthCurve->getFoliageCarbonCurve();
    }

    /**
     * Return StandComponent.getOtherCarbonCurve() on StandComponent._growthCurve
     * 
     * @return vector<double>&
     * ********************/
    const std::vector<double>& StandComponent::getOtherCarbonCurve() const {
        return _growthCurve->getOtherCarbonCurve();
    }

}}}
