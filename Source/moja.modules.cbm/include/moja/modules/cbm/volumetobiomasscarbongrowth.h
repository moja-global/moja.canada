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

#include <Poco/LRUCache.h>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API VolumeToBiomassCarbonGrowth {
    public:
        VolumeToBiomassCarbonGrowth(bool smootherEnabled = true) {
            getConverter().setSmoothing(smootherEnabled);
        };

        virtual ~VolumeToBiomassCarbonGrowth() {};	

        // Process a CBM stand growth curve to generate the biomass carbon curve.
        void generateBiomassCarbonCurve(StandGrowthCurve& standGrowthCurve);

        // Get the above ground biomass growth increment based on a yield growth curve ID and age.
        std::unordered_map<std::string, double> getBiomassCarbonIncrements(flint::ILandUnitDataWrapper* landUnitData, Int64 growthCurveID, Int64 spuID);

        // Get the total above ground biomass based on a yield growth curve ID and SPU.
        std::vector<double> getAboveGroundCarbonCurve(Int64 growthCurveID, Int64 spuID);

        // Get the total foliage curve based on a yield growth curve ID and SPU.
        std::vector<double> getFoliageCarbonCurve(Int64 growthCurveID, Int64 spuID);

        // Check if there is a biomass carbon growth curve for a stand yield growth curve.
        bool isBiomassCarbonCurveAvailable(Int64 growthCurveID, Int64 spuID);		

        void setSmoothing(bool enabled) { getConverter().setSmoothing(enabled); }

        Poco::SharedPtr<StandBiomassCarbonCurve> getBiomassCarbonCurve(Int64 growthCurveID, Int64 spuID);

    private:
        VolumeToBiomassConverter& getConverter() {
            thread_local VolumeToBiomassConverter converter;
            return converter;
        }

       Poco::LRUCache<std::tuple<Int64, Int64>, StandBiomassCarbonCurve>& getCache() {
           thread_local Poco::LRUCache<std::tuple<Int64, Int64>, StandBiomassCarbonCurve> cache;
           return cache;
        }

    };

}}}
#endif