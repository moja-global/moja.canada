#ifndef MOJA_MODULES_CBM_ICONVERTER_H_
#define MOJA_MODULES_CBM_ICONVERTER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/perdfactor.h"
#include "moja/modules/cbm/componentbiomasscarboncurve.h"
#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/smoother.h"

namespace moja {
namespace modules {
namespace cbm {			

    // ADT - to convert the yield growth curve to biomass carbon curve.
    class CBM_API VolumeToBiomassConverter {

    public:
        VolumeToBiomassConverter(bool smootherEnabled = true) : _smootherEnabled(smootherEnabled) { }
        virtual ~VolumeToBiomassConverter() { }

        /*
        * Generate stand component biomass carbon curve. The component is either of softwood or hardwood.
        * This version assums that the stand growth curve have all of the required data including:
        * Leading species for each component, PERD factor for each leading species.
        */
        std::shared_ptr<ComponentBiomassCarbonCurve> generateComponentBiomassCarbonCurve(
            StandGrowthCurve& standGrowthCurve, SpeciesType speciesType);

        void setSmoothing(bool enabled) { _smootherEnabled = enabled; }

        // Apply smoother on a carbon curve based on the stand growth yield.
        void doSmoothing(const StandGrowthCurve& standGrowthCurve,
                         ComponentBiomassCarbonCurve* carbonCurve,
                         SpeciesType speciesType);

    private:
        bool _smootherEnabled;
        Smoother _smoother;
    };

}}}
#endif