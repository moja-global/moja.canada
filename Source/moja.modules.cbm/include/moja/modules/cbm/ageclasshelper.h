/** \file file.h
 * A brief file description.
 * A more elaborated file description.
 */
 


#ifndef MOJA_MODULES_CBM_AGECLASSHELPER_H_
#define MOJA_MODULES_CBM_AGECLASSHELPER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API AgeClassHelper {
        /*! \class Test class.h "inc/class.h"
        *  \brief This is a test class.
        *
        * Some details about the Test class.
        */
	public:
        AgeClassHelper(int ageClassSize = 20, int maximumAge = 300);

        int toAgeClass(int standAge);
        std::tuple<int, int> getAgeClass(int ageClass);
        std::string getAgeClassString(int ageClass);
        std::map<int, std::tuple<int, int>> getAgeClasses();

	private:
        int _ageClassSize;
        int _maximumAge;
        int _numAgeClasses;

        std::map<int, std::tuple<int, int>> _ageClasses;
        std::map<int, int> _ageClassLookup;
    
        void generateAgeClasses(int ageClassSize, int maximumAge);
    };
	
}}}
#endif