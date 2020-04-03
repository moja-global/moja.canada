#ifndef MOJA_MODULES_CBM_VOLUMETOBIOMASSCARBONGROWTH_H_
#define MOJA_MODULES_CBM_VOLUMETOBIOMASSCARBONGROWTH_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/hash.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/abovegroundbiomasscarbonincrement.h"
#include "moja/modules/cbm/standbiomasscarboncurve.h"
#include "moja/modules/cbm/volumetobiomassconverter.h"
#include "moja/modules/cbm/rootbiomasscarbonincrement.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API VolumeToBiomassCarbonGrowth {
    public:
        VolumeToBiomassCarbonGrowth(bool smootherEnabled = true) : _converter(smootherEnabled) {};
        virtual ~VolumeToBiomassCarbonGrowth() {};	

        // Process a CBM stand growth curve to generate the biomass carbon curve.
        void generateBiomassCarbonCurve(std::shared_ptr<StandGrowthCurve> standGrowthCurve);

        // Get the above ground biomass growth increment based on a yield growth curve ID and age.
        std::unordered_map<std::string, double> getBiomassCarbonIncrements(flint::ILandUnitDataWrapper* landUnitData, Int64 growthCurveID, Int64 spuID);

        // Get the turnover rates for a curve in a specific SPU.
        std::shared_ptr<TurnoverRates> getTurnoverRates(Int64 growthCurveID, Int64 spuID);

        // Get the total above ground biomass curve based on a yield growth curve ID and SPU.
        std::vector<double> getAboveGroundCarbonCurve(Int64 growthCurveID, Int64 spuID);

        // Get the total foliage curve based on a yield growth curve ID and SPU.
        std::vector<double> getFoliageCarbonCurve(Int64 growthCurveID, Int64 spuID);

        // Check if there is a biomass carbon growth curve for a stand yield growth curve.
        bool isBiomassCarbonCurveAvailable(Int64 growthCurveID, Int64 spuID);		

        void setSmoothing(bool enabled) { _converter.setSmoothing(enabled); }

    private:
        std::shared_ptr<StandBiomassCarbonCurve> getBiomassCarbonCurve(Int64 growthCurveID, Int64 spuID);
        std::unordered_map<std::tuple<Int64, Int64>, std::shared_ptr<StandBiomassCarbonCurve>> _standBioCarbonGrowthCurves;
        VolumeToBiomassConverter _converter;
        Poco::Mutex _mutex;
    };

}}}
#endif