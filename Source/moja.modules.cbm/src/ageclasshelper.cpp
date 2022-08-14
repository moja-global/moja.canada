/**
 * @file 
 * @brief Utility functions for age class calculations
 * 
 * ******/


#include "moja/modules/cbm/ageclasshelper.h"

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {
     
     /**
     * @brief Constructor
     * 
     * Initialise AgeClassHelper._ageClassSize as ageClassSize, \n
     * AgeClassHelper._maximumAge as maximumAge, AgeClassHelper._numAgeClasses as 1 + ceil((float)maximumAge / (float)ageClassSize), \n
     * and invoke AgeClassHelper.generateAgeClasses()
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
     * @brief Initialise AgeClassHelper._ageClasses and AgeClassHelper._ageClassLookup
     * 
     * _ageClasses is a map, where the keys range from 0 to AgeClassHelper._numAgeClasses \n
     * _ageClasses[0] is reserved for non-forest 1, assigned a value [-1, -1] \n
     * _ageClasses[AgeClassHelper._numAgeClasses] is assigned [maximumAge, -1] \n
     * For each key in the range, 1 to AgeClassHelper._numAgeClasses, _ageClasses[key] is assigned [startAge, endAge] where \n
     * startAge is given as (key - 1) * ageClassSize and endAge is given as key * ageClassSize - 1 \n
     * endAge is bounded by the value maximumAge - 1 
     * 
     * For every key in _ageClasses, every age in the range startAge to endAge, \n
     * serves as a key. The corresponding value is the key in _ageClasses from which the range was generated  \n
     * _ageClassLookup[maximumAge] is assigned AgeClassHelper._numAgeClasses
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

            // Add each age in the age class to a lookup table for quick translation of stand age to age class.
            for (int age = startAge; age <= endAge; age++) {
                _ageClassLookup[age] = ageClassNumber;
            }
        }
        // Final age class is maximum age and greater.
        _ageClasses[_numAgeClasses] = std::make_tuple(maximumAge, -1);
        _ageClassLookup[maximumAge] = _numAgeClasses;
    }

    /**
     * @brief Return value of paramter ageClass in AgeClassHelper._ageClasses
     * 
     * @param ageClass int
     * @return map<int, tuple<int, int>>
     * ************************/
    std::tuple<int, int> AgeClassHelper::getAgeClass(int ageClass) {
        return _ageClasses[ageClass];
    }

    /**
     * @brief Compute and return the ageClass string
     * 
     * ageClassRange (<startAge, endAge>) of parameter ageClass is obtained from AgeClassHelper._ageClasses
     * If startAge is -1, return "N/A" \n, 
     * If endAge is -1, return "startAge+"  \n,
     * Else "startAge-endAge" \n
     * Integral values of startAge and endAge are used in the ageClass string
     * 
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
     * @brief Return AgeClassHelper._ageClasses
     * 
     * @return map<int, tuple<int, int>>
     * ************************/

    std::map<int, std::tuple<int, int>> AgeClassHelper::getAgeClasses() {
        return _ageClasses;
    }

    /**
     * @brief Return value of parameter standAge in AgeClassHelper._ageClassLookup
     * 
     * The minimum value of standAge is 0, maximum value is AgeClassHelper._maximumAge
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