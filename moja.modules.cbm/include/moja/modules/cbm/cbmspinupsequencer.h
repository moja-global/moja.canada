#ifndef MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_
#define MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/datetime.h"
#include "moja/itiming.h"
#include "moja/flint/sequencermodulebase.h"
#include "moja/notificationcenter.h"
#include "moja/hash.h"

#include <string>
#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API CBMSpinupSequencer : public flint::SequencerModuleBase {
    public:
        CBMSpinupSequencer(): _standAge(0) {};
        virtual ~CBMSpinupSequencer() {};

        const std::string returnInverval = "return_interval";
        const std::string maxRotation = "max_rotations";
        const std::string historicDistType = "historic_disturbance_type";
        const std::string lastDistType = "last_pass_disturbance_type";
		const std::string delay = "delay";
		
        void configure(ITiming& timing) override {
            timing.setStepLengthInYears(1);
        };

        bool Run(NotificationCenter& _notificationCenter, flint::ILandUnitController& luc) override;

    private:
        flint::IPool::ConstPtr _aboveGroundSlowSoil;
        flint::IPool::ConstPtr _belowGroundSlowSoil;				
		flint::IPool::ConstPtr _featherMossSlow;
		flint::IPool::ConstPtr _sphagnumMossSlow;		

        flint::IVariable* _age;
		flint::IVariable* _delay;		

        int _maxRotationValue;		// maximum rotations to do the spinup, 30, each rotation is 125 years
        int _minimumRotation;		// minimum rotation to do the spinup, 3
        int _ageReturnInterval;		// age interval to fire a historic disturbance, 125 years      
        int _standAge;				// stand age to grow after the last disturbance
        int _standDelay;			// years to delay, during delay period, only turnover and decay processes
        int _spinupGrowthCurveID;	// spinup growth curve ID
        std::string _historicDistType;  // historic disturbance type happened at each age interval
        std::string _lastPassDistType;	// last disturance type happened when the slow pool is stable and minimum rotations are done

        // SPU, historic disturbance type, GC ID, return interval
        typedef std::tuple<int, std::string, int, int> CacheKey;
        std::unordered_map<CacheKey, std::vector<double>, moja::Hash> _cache;

        // Get spinup parameters for this land unit
        bool getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData);

        // Check if the slow pool is stable
        bool isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue);	

		// Check moss
		bool isMossApplicable();

		// Check peatland
		bool isPeatlandApplicable();

        // Fire timing events
        void fireSpinupSequenceEvent(NotificationCenter& notificationCenter,
                                     flint::ILandUnitController& luc,
                                     int maximumSteps);

		// Fire historical and last disturbance
		void fireHistoricalLastDisturbanceEvent(NotificationCenter& notificationCenter,
                                                flint::ILandUnitController& luc,
                                                std::string disturbanceName);
    };

}}} // namespace moja::Modules::CBM

#endif // MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_
