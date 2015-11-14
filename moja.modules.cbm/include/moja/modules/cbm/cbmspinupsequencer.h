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
        const std::string historicDistTypeID = "historic_disturbance_type_id";
        const std::string lastDistTypeID = "last_disturbance_type_id";
		const std::string delay = "delay";

        void configure(ITiming& timing) override {
            startDate = timing.startDate();
            endDate = timing.endDate();
        };

        bool Run(NotificationCenter& _notificationCenter, flint::ILandUnitController& luc) override;

    private:
        DateTime startDate;
        DateTime endDate;

        flint::IPool::ConstPtr _aboveGroundSlowSoil;
        flint::IPool::ConstPtr _belowGroundSlowSoil;				
        flint::IVariable* _age;
		flint::IVariable* _delay;

        /* Get spinup parameters for this land unit */
        bool getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData);

        /* Check if the slow pool is stable */
        bool isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue);	

        /* Fire timing events */
        void fireSpinupSequenceEvent(NotificationCenter& notificationCenter, flint::ILandUnitController& luc, int maximumSteps);

        int _maxRotationValue;		// maximum rotations to do the spinup, 30, each rotation is 125 years
        int _miniumRotation;		// minimum rotation to do the spinup, 3
        int _ageReturnInterval;		// age interval to fire a historic disturbance, 125 years				
        int _historicDistTypeID;	// historic disturbance type happened at each age interval
        int _lastDistTypeID;		// last disturance type happened when the slow pool is stable and minimum rotations are done
        int _standAge;				// stand age to grow after the last disturbance
		int _standDelay;			// years to delay, during delay period, only turnover and decay processes
        typedef std::tuple<int /* spu */, int /* histdisttypeid */, int /* lastdisttypeid */, int /* gcid */> CacheKey;
        std::unordered_map<CacheKey, std::vector<double>, moja::Hash> _cache;
    };

}}} // namespace moja::Modules::CBM

#endif // MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_
