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
                       std::shared_ptr<ComponentBiomassCarbonCurve> growthCurve,
                       const moja::flint::IVariable* age,
                       const moja::flint::IPool* merch,
                       const moja::flint::IPool* foliage,
                       const moja::flint::IPool* other,
                       const moja::flint::IPool* fineRoots,
                       const moja::flint::IPool* coarseRoots)
            : _forestType(forestType), _growthCurve(growthCurve), _age(age), _merch(merch),
              _foliage(foliage), _other(other), _fineRoots(fineRoots), _coarseRoots(coarseRoots) {
            
            _rootBiomassEquation = rootBiomassEquation;
        }

        virtual ~StandComponent() = default;

        virtual const std::string& forestType() const { return _forestType; }

        virtual double calculateRootBiomass() const;
        virtual std::unordered_map<std::string, double> getIncrements(double standRootBiomass) const;
        virtual std::vector<double> getAboveGroundCarbonCurve() const;

    private:
        std::string _forestType;
        std::shared_ptr<RootBiomassEquation> _rootBiomassEquation;
        std::shared_ptr<ComponentBiomassCarbonCurve> _growthCurve;
        const moja::flint::IVariable* _age;
        const moja::flint::IPool* _merch;
        const moja::flint::IPool* _foliage;
        const moja::flint::IPool* _other;
        const moja::flint::IPool* _fineRoots;
        const moja::flint::IPool* _coarseRoots;
        
        std::unordered_map<std::string, double> getAGIncrements() const;
    };
    
}}}
#endif
