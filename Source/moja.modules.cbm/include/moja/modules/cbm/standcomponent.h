#ifndef MOJA_MODULES_CBM_STANDCOMPONENT_H_
#define MOJA_MODULES_CBM_STANDCOMPONENT_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/componentbiomasscarboncurve.h"

#include "moja/flint/ipool.h"
#include "moja/flint/modulebase.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {
    
    class CBM_API StandComponent {
    public:
        StandComponent(std::string forestType,
                       std::shared_ptr<RootBiomassEquation> rootBiomassEquation,
                       std::shared_ptr<ComponentBiomassCarbonCurve> growthCurve)
            : _forestType(forestType), _growthCurve(growthCurve) {
            
            _rootBiomassEquation = rootBiomassEquation;
        }

        virtual ~StandComponent() = default;

        virtual const std::string& forestType() const { return _forestType; }

        virtual double calculateRootBiomass(flint::ILandUnitDataWrapper* landUnitData) const;
        virtual std::unordered_map<std::string, double> getIncrements(flint::ILandUnitDataWrapper* landUnitData, double standRootBiomass) const;
        virtual std::vector<double> getAboveGroundCarbonCurve() const;

        virtual const std::vector<double>& getMerchCarbonCurve() const;
        virtual const std::vector<double>& getFoliageCarbonCurve() const;
        virtual const std::vector<double>& getOtherCarbonCurve() const;

    private:
        std::string _forestType;
        std::shared_ptr<RootBiomassEquation> _rootBiomassEquation;
        std::shared_ptr<ComponentBiomassCarbonCurve> _growthCurve;
        
        std::unordered_map<std::string, double> getAGIncrements(
            const flint::IVariable* age, const flint::IPool* merch, const flint::IPool* other,
            const flint::IPool* foliage) const;
    };
    
}}}
#endif
