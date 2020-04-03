#ifndef MOJA_MODULES_CBM_STANDBIOMASSCARBONCURVE_H_
#define MOJA_MODULES_CBM_STANDBIOMASSCARBONCURVE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/modules/cbm/standcomponent.h"
#include "moja/modules/cbm/turnoverrates.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API StandBiomassCarbonCurve {
    public:
        StandBiomassCarbonCurve(std::shared_ptr<TurnoverRates> turnoverRates) : _turnoverRates(turnoverRates) {};
        virtual ~StandBiomassCarbonCurve() {};

        inline void addComponent(const StandComponent& component) {
            _components.push_back(component);
        }

        std::unordered_map<std::string, double> getIncrements(flint::ILandUnitDataWrapper* landUnitData);

        // Gets the hardwood and softwood turnover rates for the growth curve.
        std::shared_ptr<TurnoverRates> getTurnoverRates();

        // Gets the absolute total aboveground carbon at each age, where index = age.
        std::vector<double> getAboveGroundCarbonCurve();

        // Gets the absolute total foliage carbon at each age, where index = age.
        std::vector<double> getFoliageCarbonCurve();

    private:
        std::vector<StandComponent> _components;
        std::shared_ptr<TurnoverRates> _turnoverRates;
    };

}}}
#endif