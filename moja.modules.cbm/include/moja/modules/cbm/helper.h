#ifndef CBM_Helper_H_
#define CBM_Helper_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"

namespace moja {
namespace modules {
namespace CBM {

    class CBM_API Helper {
    public:
        Helper() = delete;
        ~Helper() = delete;

        static double calculateMerchFactor(double volume, double a, double b);
        
        static double calculateNonMerchFactor(
            double merchStemwood, double a_nonmerch,
            double b_nonmerch, double k_nonmerch);

        static double calculateSaplingFactor(double stemwood, double k_sap,
                                             double a_sap, double b_sap);

        static double modelTerm(double vol, double a1, double a2, double a3);
    };

    /// <summary>
    /// see equation 1 on page 7 of Boudewyn et al 2007
    /// </summary>
    inline double Helper::calculateMerchFactor(double volume, double a, double b) {
        return a * pow(volume, b);
    }

    /// <summary>
    /// see equation 2 on page 7 of Boudewyn et al 2007
    /// </summary>
    inline double Helper::calculateNonMerchFactor(
        double merchStemwood, double a_nonmerch, double b_nonmerch, double k_nonmerch) {

        return k_nonmerch + a_nonmerch * pow(merchStemwood, b_nonmerch);
    }

    /// <summary>
    /// page 8 Boudewyn et al equation 3
    /// </summary>
    inline double Helper::calculateSaplingFactor(
        double stemwood, double k_sap, double a_sap, double b_sap) {

        return k_sap + a_sap * pow(stemwood, b_sap);
    }

    /// <summary>
    /// see equations 4,5,6,7 on page 8 of Boudewyn et al 2007
    /// </summary>
    inline double Helper::modelTerm(double vol, double a1, double a2, double a3) {
        return exp(a1 + a2 * vol + a3 * log(vol + 5.0));
    }

}}}
#endif