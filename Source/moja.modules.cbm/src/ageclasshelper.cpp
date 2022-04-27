/**
 * @file 
 * @brief The brief description goes here.
 * 
 * The detailed description if any, goes here 
 * ******/


#include "moja/modules/cbm/ageclasshelper.h"

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {
     
     /**
     * @brief Constructor.
     * 
     * This is the constructor for AgeClassHelper
     * The values are passed and assigned here
     * 
     * @param ageClassSize int
     * @param maximumAge int
     * ************************/
    AgeClassHelper::AgeClassHelper(int ageClassSize, int maximumAge) :
        _ageClassSize(ageClassSize),
        _maximumAge(maximumAge),
        _numAgeClasses(1 + ceil((float)maximumAge / (float)ageClassSize)) {

        generateAgeClasses(ageClassSize, maximumAge);
    }
    
    /**
     * @brief Generates the AgeClasses.
     * 
     * This is the function to generate age classes
     * The values are passed and assigned here
     * 
     * @param ageClassSize int
     * @param maximumAge int
     * @return void
     * ************************/

    void AgeClassHelper::generateAgeClasses(int ageClassSize, int maximumAge) {
        // Reserve age class 0 for non-forest 1 [-1, -1].
        _ageClasses[0] = std::make_tuple(-1, -1);

        for (int ageClassNumber = 1; ageClassNumber < _numAgeClasses; ageClassNumber++) {
            int startAge = (ageClassNumber - 1) * ageClassSize;
            
            int endAge = ageClassNumber * ageClassSize - 1;
            if (endAge >= maximumAge) {
                // The next-to-last age class ends at the maximum age.
                endAge = maximumAge - 1;
            }

            _ageClasses[ageClassNumber] = std::make_tuple(startAge, endAge);

            // Add each age in the age class to a lookup table for quick translation of
            // stand age to age class.
            for (int age = startAge; age <= endAge; age++) {
                _ageClassLookup[age] = ageClassNumber;
            }
        }

        // Final age class is maximum age and greater.
        _ageClasses[_numAgeClasses] = std::make_tuple(maximumAge, -1);
        _ageClassLookup[maximumAge] = _numAgeClasses;
    }

    /**
     * @brief getAgeClasses.
     * @param ageClass int
     * @return map<int, tuple<int, int>>
     * ************************/
    std::tuple<int, int> AgeClassHelper::getAgeClass(int ageClass) {
        return _ageClasses[ageClass];
    }

    /**
     * @brief calculate Age class String
     * @param ageClass int.
     * @return String
     * ************************/

    std::string AgeClassHelper::getAgeClassString(int ageClass) {
        auto ageClassRange = _ageClasses[ageClass];
        auto ageClassStart = std::get<0>(ageClassRange);
        auto ageClassEnd = std::get<1>(ageClassRange);
        auto ageClassString = ageClassStart == -1 ? "N/A"
            : ageClassEnd == -1 ? (boost::format("%1%+") % ageClassStart).str()
            : (boost::format("%1%-%2%") % ageClassStart % ageClassEnd).str();
                
        return ageClassString;
    }

    /**
     * @brief Overloaded getAgeClasses.
     * @return map<int, tuple<int, int>>
     * ************************/

    std::map<int, std::tuple<int, int>> AgeClassHelper::getAgeClasses() {
        return _ageClasses;
    }

    /**
     * @brief toAgeClass converts maps age to class.
     * 
     * @param standAge int
     * @return int
     * ************************/

    int AgeClassHelper::toAgeClass(int standAge) {
        if (standAge < 0) {
            return 0;
        }

        if (standAge > _maximumAge) {
            return _ageClassLookup[_maximumAge];
        }

        return _ageClassLookup[standAge];
    }

}}}