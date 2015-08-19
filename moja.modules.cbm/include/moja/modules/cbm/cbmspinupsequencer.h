#ifndef CBMSpinupSequencer_H_
#define CBMSpinupSequencer_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/datetime.h"
#include "moja/itiming.h"
#include "moja/flint/sequencermodulebase.h"
#include "moja/notificationcenter.h"

#include <string>

namespace moja {
	namespace modules {
		namespace CBM {

			class CBM_API CBMSpinupSequencer : public flint::SequencerModuleBase {
			public:
				CBMSpinupSequencer(): _standAge(0) {};
				virtual ~CBMSpinupSequencer() {};

				const std::string returnInverval = "ReturnInterval";
				const std::string maxRotation = "MaxRotations";
				const std::string histroricDistTypeID = "HistoricDisturbanceTypeId";
				const std::string lastDistTypeID = "LastDisturbanceTypeId";

				void configure(ITiming& timing) override {
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

				/*Get spinup parameters for this land unit*/
				bool getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData);

				/*Check if the slow pool is stable*/
				bool isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue);	

				/*Fire timing events*/
				void fireSpinupSequenceEvent(NotificationCenter& notificationCenter, int maximumSteps);

				int _maxRotationValue;		// maximum rotations to do the spinup, 30, each rotation is 125 years
				int _miniumRotation;		// minimum rotation to do the spinup, 3
				int _ageReturnInterval;		// age interval to fire a historic disturbance, 125 years				
				int _histroricDistTypeID;	// historic disturbance type happened at each age interval
				int _lastDistTypeID;		// last disturance type happened when the slow pool is stable and minimum rotations are done
				int _standAge;				// stand age to grow after the last disturbance
			};

		}
	}
} // namespace moja::Modules::CBM
#endif // CBMSpinupSequencer_H_