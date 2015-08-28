#ifndef MOJA_MODULES_CBM_BIOMASSCARBONINCREMENT_H_
#define MOJA_MODULES_CBM_BIOMASSCARBONINCREMENT_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"

namespace moja {
namespace modules {
namespace cbm {

    /*
    * ADT - Stand growth curve biomass carbon increment
    * This is the tree growth output at a specified age.
    */
    class CBM_API AboveGroundBiomassCarbonIncrement {
    public:
        AboveGroundBiomassCarbonIncrement() = delete;
        virtual ~AboveGroundBiomassCarbonIncrement() {};

        AboveGroundBiomassCarbonIncrement(
            double softwoodMerch, double softwoodFoliage, double softwoodOther,
            double hardwoodMerch, double hardwoodFoliage, double hardwoodOther)
            : _softwoodMerch(softwoodMerch), _softwoodOther(softwoodOther), _softwoodFoliage(softwoodFoliage),
              _hardwoodMerch(hardwoodMerch), _hardwoodOther(hardwoodOther), _hardwoodFoliage(hardwoodFoliage) {}

        double softwoodMerch() const { return _softwoodMerch; }
        double softwoodOther() const { return _softwoodOther; }
        double softwoodFoliage() const { return _softwoodFoliage; }
        double hardwoodMerch() const { return _hardwoodMerch; }
        double hardwoodOther() const { return _hardwoodOther; }
        double hardwoodFoliage()const { return _hardwoodFoliage; }

        double getTotalAGBiomassIncrements();

    private:
        double _softwoodMerch;
        double _softwoodOther;
        double _softwoodFoliage;
        double _hardwoodMerch;
        double _hardwoodOther;
        double _hardwoodFoliage;
    };

    inline double AboveGroundBiomassCarbonIncrement::getTotalAGBiomassIncrements() {
        return _softwoodMerch + _softwoodOther + _softwoodFoliage
            + _hardwoodMerch + _hardwoodOther + _hardwoodFoliage;
    }

}}}
#endif