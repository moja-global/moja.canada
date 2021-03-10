#ifndef MOJA_MODULES_CBM_ROOTBIOMASSEQUATION_H_
#define MOJA_MODULES_CBM_ROOTBIOMASSEQUATION_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"

#include <cmath>

namespace moja {
namespace modules {
namespace cbm {

    struct RootProportions {
        double fine;
        double coarse;
    };

    class CBM_API RootBiomassEquation {
    public:
        RootBiomassEquation(double biomassToCarbonRate = 0.5)
            : _biomassToCarbonRate(biomassToCarbonRate) {}

        RootBiomassEquation(const RootBiomassEquation& other)
            : _biomassToCarbonRate(other._biomassToCarbonRate) {}

        virtual ~RootBiomassEquation() = default;

        inline double carbonToBiomass(double carbon)  { return carbon  / _biomassToCarbonRate; }
        inline double biomassToCarbon(double biomass) { return biomass * _biomassToCarbonRate; }

        // Calculate the total root biomass from the total aboveground carbon
        // for the stand component.
        virtual double calculateRootBiomass(double componentAbovegroundCarbon) = 0;

        // Calculate the coarse and fine root proportions from the stand's total
        // root biomass for all components.
        virtual RootProportions calculateRootProportions(double standRootBiomass) = 0;

    protected:
        double _biomassToCarbonRate;
    };

    class CBM_API SoftwoodRootBiomassEquation : public RootBiomassEquation {
    public:
        SoftwoodRootBiomassEquation(double rootBioA, double frpA, double frpB, double frpC)
            : RootBiomassEquation(),
              _rootBioA(rootBioA), _frpA(frpA), _frpB(frpB), _frpC(frpC) {}

        virtual ~SoftwoodRootBiomassEquation() = default;

        inline double calculateRootBiomass(double componentAbovegroundCarbon) {
            // Li et al 2003 softwood root biomass.
            return _rootBioA * carbonToBiomass(componentAbovegroundCarbon);
        }

        inline RootProportions calculateRootProportions(double standRootBiomass) {
            // Li et al 2003 root proportions.
            double fineRootProp = _frpA + _frpB * std::exp(_frpC * standRootBiomass);
            double coarseRootProp = 1 - fineRootProp;

            return RootProportions{ fineRootProp, coarseRootProp };
        }

    private:
        double _rootBioA;
        double _frpA;
        double _frpB;
        double _frpC;
    };

    class CBM_API HardwoodRootBiomassEquation : public RootBiomassEquation {
    public:
        HardwoodRootBiomassEquation(double rootBioA, double rootBioB, double frpA, double frpB, double frpC)
            : RootBiomassEquation(),
              _rootBioA(rootBioA), _rootBioB(rootBioB), _frpA(frpA), _frpB(frpB), _frpC(frpC) {}

        virtual ~HardwoodRootBiomassEquation() = default;

        inline double calculateRootBiomass(double componentAbovegroundCarbon) {
            // Li et al 2003 hardwood root biomass.
            return _rootBioA * std::pow(carbonToBiomass(componentAbovegroundCarbon), _rootBioB);
        }

        inline RootProportions calculateRootProportions(double standRootBiomass) {
            // Li et al 2003 root proportions.
            double fineRootProp = _frpA + _frpB * std::exp(_frpC * standRootBiomass);
            double coarseRootProp = 1 - fineRootProp;

            return RootProportions{ fineRootProp, coarseRootProp };
        }

    private:
        double _rootBioA;
        double _rootBioB;
        double _frpA;
        double _frpB;
        double _frpC;
    };

}}}
#endif
