#ifndef MOJA_MODULES_CBM_VOLUMETOBIOMASSCARBONGROWTH_H_
#define MOJA_MODULES_CBM_VOLUMETOBIOMASSCARBONGROWTH_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/abovegroundbiomasscarbonincrement.h"
#include "moja/modules/cbm/standbiomasscarboncurve.h"
#include "moja/modules/cbm/volumetobiomassconverter.h"
#include "moja/modules/cbm/rootbiomasscarbonincrement.h"
#include "moja/modules/cbm/overmaturedeclinelosses.h"


namespace moja {
namespace modules {
namespace cbm {

    class CBM_API VolumeToBiomassCarbonGrowth {
    public:
        VolumeToBiomassCarbonGrowth();
        virtual ~VolumeToBiomassCarbonGrowth() {};	

        const double otherToBranchSnagSplit = 0.25;
        const double coarseRootAGSplit = 0.5;
        const double fineRootAGSplit = 0.5;
        const double softwoodRootParameterA = 0.222;
        const double hardwoodRootParameterA = 1.576;
        const double hardwoodRootParameterB = 0.615;
        const double fineRootProportionParameterA = 0.072;
        const double fineRootProportionParameterB = 0.354;
        const double fineRootProportionParameterC = -0.060212;
        const double biomassToCarbonRation = 0.5;
        
        /*
        * Process a CBM stand growth curve to generate the biomass carbon curve	
        */
        void generateBiomassCarbonCurve(std::shared_ptr<StandGrowthCurve> standGrowthCurve);

        /*
        * Get the above ground biomass growth increment based on a yield growth curve ID and age
        */
        std::shared_ptr<AboveGroundBiomassCarbonIncrement> getAGBiomassCarbonIncrements(
            Int64 GrowthCurveID, int Age);

        /*
        * Get the below ground biomass growth increment based on a yield growth curve ID and age
        */
        std::shared_ptr<RootBiomassCarbonIncrement> getBGBiomassCarbonIncrements(
            double totalSWAgCarbon, double standSWCoarseRootsCarbon, double standSWFineRootsCarbon,
            double totalHWAgCarbon, double standHWCoarseRootsCarbon, double standHWFineRootsCarbon);

        /*
        * Check if there is a biomass carbon growth curve for a stand yield growth curve
        */
        bool isBiomassCarbonCurveAvailable(Int64 growthCurveID);		

    private:	
        std::shared_ptr<StandBiomassCarbonCurve> getBiomassCarbonCurve(Int64 growthCurveID);
        std::map<Int64, std::shared_ptr<StandBiomassCarbonCurve>> _standBioCarbonGrowthCurves;
        std::unique_ptr<VolumeToBiomassConverter> _converter;		
    };

}}}
#endif