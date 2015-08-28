#ifndef MOJA_MODULES_CBM_ROOTBIOMASSCARBONINCREMENT_H_
#define MOJA_MODULES_CBM_ROOTBIOMASSCARBONINCREMENT_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API RootBiomassCarbonIncrement {
    public:
        RootBiomassCarbonIncrement() : _softwoodCoarseRoots(0.0), _softwoodFineRoots(0.0),
                                       _hardwoodCoarseRoots(0.0), _hardwoodFineRoots(0.0) {}

        virtual ~RootBiomassCarbonIncrement() = default;			
    
        double softwoodCoarseRoots() const;
        double softwoodFineRoots() const;
        double hardwoodCoarseRoots() const;
        double hardwoodFineRoots() const;
        
        void setSoftwoodCoarseRoots(double value);
        void setSoftwoodFineRoots(double value); 
        void setHardwoodCoarseRoots(double value);
        void setHardwoodFineRoots(double value); 
                
    private:
        double _softwoodCoarseRoots;  
        double _softwoodFineRoots;    
        double _hardwoodCoarseRoots;  
        double _hardwoodFineRoots;  
    };

    inline double RootBiomassCarbonIncrement::softwoodCoarseRoots() const { return _softwoodCoarseRoots; }
    inline double RootBiomassCarbonIncrement::softwoodFineRoots() const { return _softwoodFineRoots; }
    inline double RootBiomassCarbonIncrement::hardwoodCoarseRoots() const { return _hardwoodCoarseRoots; }
    inline double RootBiomassCarbonIncrement::hardwoodFineRoots() const { return _hardwoodFineRoots; }

    inline void RootBiomassCarbonIncrement::setSoftwoodCoarseRoots(double value) { _softwoodCoarseRoots = value; }
    inline void RootBiomassCarbonIncrement::setSoftwoodFineRoots(double value) { _softwoodFineRoots = value; }
    inline void RootBiomassCarbonIncrement::setHardwoodCoarseRoots(double value) { _hardwoodCoarseRoots = value; }
    inline void RootBiomassCarbonIncrement::setHardwoodFineRoots(double value) { _hardwoodFineRoots = value; }

}}}
#endif