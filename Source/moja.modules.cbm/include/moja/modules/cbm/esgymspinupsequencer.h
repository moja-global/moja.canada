#ifndef MOJA_MODULES_CBM_ESGYMSPINUPSEQUENCER_H_
#define MOJA_MODULES_CBM_ESGYMSPINUPSEQUENCER_H_

#include <unordered_map>

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/datetime.h"
#include "moja/flint/itiming.h"
#include "moja/flint/sequencermodulebase.h"
#include "moja/notificationcenter.h"
#include "moja/pocojsonutils.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API ESGYMSpinupSequencer : public flint::SequencerModuleBase {
	public:
		ESGYMSpinupSequencer() : _standAge(0) {};
		virtual ~ESGYMSpinupSequencer() {};

		const std::string returnInverval = "return_interval";
		const std::string maxRotation = "max_rotations";
		const std::string historicDistType = "historic_disturbance_type";
		const std::string lastDistType = "last_pass_disturbance_type";
		const std::string delay = "delay";
		const std::string inventoryDelay = "inventory_delay";

        void configure(const DynamicObject& config) override {
            if (config.contains("ramp_start_date")) {
                _rampStartDate = moja::parseSimpleDate(
                    config["ramp_start_date"].extract<std::string>());
            }
        };

        void configure(flint::ITiming& timing) override {
			startDate = timing.startDate();
			endDate = timing.endDate();
		};

		bool Run(NotificationCenter& _notificationCenter, flint::ILandUnitController& luc) override;

	private:
		DateTime startDate;
		DateTime endDate;

		const flint::IPool* _aboveGroundSlowSoil;
		const flint::IPool* _belowGroundSlowSoil;
		flint::IVariable* _age;
		flint::IVariable* _delay;

		/* Get spinup parameters for this land unit */
		bool getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData);

		/* Check if the slow pool is stable */
		bool isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue);

		/* Fire timing events */
		void fireSpinupSequenceEvent(NotificationCenter& notificationCenter,
                                     flint::ILandUnitController& luc,
                                     int maximumSteps,
                                     bool incrementStep);

		/* Fire historical and last disturbance */
		void fireHistoricalLastDisturbanceEvent(NotificationCenter& notificationCenter,
                                                flint::ILandUnitController& luc,
                                                std::string disturbanceName);

        void fetchDistTypeCodes();
        
        int _maxRotationValue;		// maximum rotations to do the spinup, 30, each rotation is 125 years
		int _miniumRotation;		// minimum rotation to do the spinup, 3
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

        std::unordered_map<std::string, int> _distTypeCodes;
    };
}}}
#endif
