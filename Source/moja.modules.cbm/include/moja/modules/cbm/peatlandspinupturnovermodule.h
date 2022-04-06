#ifndef MOJA_MODULES_CBM_SPINUP_PEATLAND_PREPARE_H_
#define MOJA_MODULES_CBM_SPINUP_PEATLAND_PREPARE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/peatlandturnovermodulebase.h"

#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/*
			Prepare initial variables to simulate a peatland landunit (pixel)
			*/
			class CBM_API PeatlandSpinupTurnOverModule : public PeatlandTurnoverModuleBase {
			public:
				PeatlandSpinupTurnOverModule() {};
				virtual ~PeatlandSpinupTurnOverModule() {};

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;


			private:
				flint::IVariable* _spinupMossOnly = nullptr;

				double _spinup_longterm_wtd{ 0 };
				double _spinup_previous_annual_wtd{ 0 };
				double _spinup_current_annual_wtd{ 0 };

				void doWaterTableFlux();

				bool _isInitialPoolLoaded{ false };
				void loadPeatlandInitialPoolValues(const DynamicObject& data);
			};
		}
	}
}
#endif