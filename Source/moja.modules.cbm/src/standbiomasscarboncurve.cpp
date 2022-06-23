#include "moja/modules/cbm/standbiomasscarboncurve.h"

#include <moja/logging.h>
#include <algorithm>
#include <fstream>

namespace moja {
namespace modules {
namespace cbm {
    
    /**
     * Create a variable standRootBiomass with initial value 0.0 \n
     * Add to standRootBiomass the result of StandComponent.calculateRootBiomass() on each component in StandBiomassCarbonCurve._components \n
     * Create an unoredered_map<string::double> variable increments \n
     * For each component in StandBiomassCarbonCurve._components, add the result of StandComponent.getIncrements() with arguments as parameter landUnitData and variable standRootBiomass to variable increments \n
     * Return increments
     * 
     * @param landUnitData flint::ILandUnitDataWrapper*
     * @return nordered_map<string, double>
     * ***********************/
    std::unordered_map<std::string, double> StandBiomassCarbonCurve::getIncrements(flint::ILandUnitDataWrapper* landUnitData) {
        double standRootBiomass = 0.0;
        for (const auto& component : _components) {
            standRootBiomass += component.calculateRootBiomass(landUnitData);
        }

        std::unordered_map<std::string, double> increments;
        for (const auto& component : _components) {
            const auto componentIncrements = component.getIncrements(landUnitData, standRootBiomass);
            increments.insert(componentIncrements.begin(), componentIncrements.end());
        }

        return increments;
    }

    /**
     * Get the absolute total aboveground carbon at each age, where index = age
     * 
     * Create a vector variable, curve, to store the absolute total aboveground carbon at each age \n
     * For each component in StandBiomassCarbonCurve._components, invoke StandComponent.getAboveGroundCarbonCurve() and assign it to variable componentCurve \n 
     * If the size of componentCurve > size of curve, resize variable curve to the size of componentCurve, initialise the new indices in curve to 0.0 \n
     * For each index in range 0 to the size of componentCurve, add the value of componentCurve at that index to the value at the same index in variable curve \n
     * Return variable curve
     * 
     * @return vector<double>
     * ***********************/
    std::vector<double> StandBiomassCarbonCurve::getAboveGroundCarbonCurve() {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getAboveGroundCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    /**
     * Get the absolute total merchantable carbon at each age, where index = age
     * 
     * Create a vector variable, curve, to store the absolute total merchantable carbon at each age \n
     * For each component in StandBiomassCarbonCurve._components, invoke StandComponent.getMerchCarbonCurve() and assign it to variable componentCurve \n 
     * If the size of componentCurve > size of curve, resize variable curve to the size of componentCurve, initialise the new indices in curve to 0.0 \n
     * For each index in range 0 to the size of componentCurve,  add the value of componentCurve at that index to the value at the same index in variable curve \n
     * Return variable curve
     * 
     * @return vector<double>
     * ***********************/
    std::vector<double> StandBiomassCarbonCurve::getMerchCarbonCurve() {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getMerchCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    /**
     * Get the absolute total foliage carbon at each age, where index = age
     * 
     * Create a vector variable, curve, to store the absolute total foliage carbon at each age \n
     * For each component in StandBiomassCarbonCurve._components, invoke StandComponent.getFoliageCarbonCurve() and assign it to variable componentCurve \n 
     * If the size of componentCurve > size of curve, resize variable curve to the size of componentCurve, initialise the new indices in curve to 0.0 \n
     * For each index in range 0 to the size of componentCurve,  add the value of componentCurve at that index to the value at the same index in variable curve \n
     * Return variable curve
     * 
     * @return vector<double>
     * ***********************/
    std::vector<double> StandBiomassCarbonCurve::getFoliageCarbonCurve()
    {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getFoliageCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    /**
     * Get the absolute total other carbon at each age, where index = age
     * 
     * Create a vector variable, curve, to store the absolute total other carbon at each age \n
     * For each component in StandBiomassCarbonCurve._components, invoke StandComponent.getOtherCarbonCurve() and assign it to variable componentCurve \n 
     * If the size of componentCurve > size of curve, resize variable curve to the size of componentCurve, initialise the new indices in curve to 0.0 \n
     * For each index in range 0 to the size of componentCurve,  add the value of componentCurve at that index to the value at the same index in variable curve \n
     * Return variable curve
     * 
     * @return vector<double>
     * ***********************/
    std::vector<double> StandBiomassCarbonCurve::getOtherCarbonCurve() {
        std::vector<double> curve;
        for (const auto& component : _components) {
            const auto& componentCurve = component.getOtherCarbonCurve();
            auto maxComponentAge = componentCurve.size();

            if (curve.size() < maxComponentAge) {
                curve.resize(maxComponentAge, 0.0);
            }

            for (int i = 0; i < maxComponentAge; i++) {
                curve[i] += componentCurve[i];
            }
        }

        return curve;
    }

    /**
     * Write the results of StandBiomassCarbonCurve.getMerchCarbonCurve(), StandBiomassCarbonCurve.getFoliageCarbonCurve(), 
     * and StandBiomassCarbonCurve.getOtherCarbonCurve() to a file, parameter path is the output file path
     *       
     * @param path string&
     * @return void
     * ************************/
    void StandBiomassCarbonCurve::writeDebuggingInfo(const std::string& path) {
        std::ofstream file(path);

        file << "component,";
        for (int i = 0; i < getMerchCarbonCurve().size(); i++) {
            file << i << ",";
        }

        file << "\nMerch,";
        for (auto& value : getMerchCarbonCurve()) {
            file << value << ",";
        }

        file << "\nFoliage,";
        for (auto& value : getFoliageCarbonCurve()) {
            file << value << ",";
        }

        file << "\nOther,";
        for (auto& value : getOtherCarbonCurve()) {
            file << value << ",";
        }

        file.close();
    }

}}}