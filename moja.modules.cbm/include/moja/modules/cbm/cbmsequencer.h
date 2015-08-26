#ifndef MOJA_MODULES_CBM_CBMSEQUENCER_H_
#define MOJA_MODULES_CBM_CBMSEQUENCER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/datetime.h"
#include "moja/itiming.h"
#include "moja/flint/sequencermodulebase.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/notificationcenter.h"

#include <string>

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API CBMSequencer : public flint::SequencerModuleBase {
			public:
				CBMSequencer() {};
				virtual ~CBMSequencer() {};

				void configure(ITiming& timing) override {
					startDate = timing.startDate();
					endDate = timing.endDate();
				};

				bool Run(NotificationCenter& _notificationCenter, flint::ILandUnitController& luc) override;

			private:
				DateTime startDate;
				DateTime endDate;
			};

		}
	}
} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMSEQUENCER_H_