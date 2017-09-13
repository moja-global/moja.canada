#ifndef MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_
#define MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/datetime.h"
#include "moja/itiming.h"
#include "moja/flint/sequencermodulebase.h"
#include "moja/notificationcenter.h"
#include "moja/hash.h"
#include "moja/pocojsonutils.h"

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
		
        void configure(const DynamicObject& config) override {
            if (config.contains("ramp_start_date")) {
                _rampStartDate = moja::parseSimpleDate(
                    config["ramp_start_date"].extract<std::string>());
            }
        };

        void configure(ITiming& timing) override {
            timing.setStepLengthInYears(1);
        };

        bool Run(NotificationCenter& _notificationCenter, flint::ILandUnitController& luc) override;

    private:
        const flint::IPool* _aboveGroundSlowSoil;
        const flint::IPool* _belowGroundSlowSoil;				
		const flint::IPool* _featherMossSlow;
		const flint::IPool* _sphagnumMossSlow;		

        flint::IVariable* _age;
        flint::IVariable* _delay;
		flint::IVariable* _mat;
		flint::IVariable* _spu;
		flint::IVariable* _peatlandAge;

        int _maxRotationValue;		// maximum rotations to do the spinup, 30, each rotation is 125 years
        int _minimumRotation;		// minimum rotation to do the spinup, 3
        int _ageReturnInterval;		// age interval to fire a historic disturbance, 125 years      
        int _standAge;				// stand age to grow after the last disturbance
        int _standDelay;			// years to delay, during delay period, only turnover and decay processes
        int _spinupGrowthCurveID;	// spinup growth curve ID
        std::string _historicDistType;  // historic disturbance type happened at each age interval
        std::string _lastPassDistType;	// last disturance type happened when the slow pool is stable and minimum rotations are done
        
        // Optional ramp to use at the end of the spinup period; used when, for example, spinup uses a
        // value of 10 for a variable, and the rest of the simulation uses a value of 20, and the values
        // need to blend smoothly together, so the user prepares a 10-value ramp which is used for the last
        // 10 timesteps of the spinup period: 10, 11, 12, 13, ...
        Poco::Nullable<DateTime> _rampStartDate;

        // SPU, historic disturbance type, GC ID, return interval, mean annual temperature
        typedef std::tuple<int, std::string, int, int, double> CacheKey;
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
                                     int maximumSteps,
                                     bool incrementStep);

		// Fire historical and last disturbance
		void fireHistoricalLastDisturbanceEvent(NotificationCenter& notificationCenter,
                                                flint::ILandUnitController& luc,
                                                std::string disturbanceName);
    };

}}} // namespace moja::Modules::CBM

#endif // MOJA_MODULES_CBM_CBMSPINUPSEQUENCER_H_
