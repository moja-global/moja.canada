#ifndef MOJA_MODULES_CBM_PLTURNOVER_H_
#define MOJA_MODULES_CBM_PLTURNOVER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/peatlandturnovermodulebase.h"

#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandgrowthparameters.h"

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API PeatlandTurnoverModule : public PeatlandTurnoverModuleBase {
			public:
				PeatlandTurnoverModule() : PeatlandTurnoverModuleBase() { }
				virtual ~PeatlandTurnoverModule() = default;

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;

			private:
				flint::IVariable* _waterTableDepthModifier{ nullptr };
				flint::IVariable* _spinupMossOnly{ nullptr };

				double _forward_longterm_wtd{ 0 };
				double _forward_previous_annual_wtd{ 0 };
				double _forward_current_annual_wtd{ 0 };

				std::string _forward_wtd_modifier{ "" };
				bool _modifiersFullyAppplied{ false };

				void doWaterTableFlux();
				void updateWaterTable();
				double getModifiedAnnualWTD(std::string modifierStr);
			};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLTURNOVER_H_
