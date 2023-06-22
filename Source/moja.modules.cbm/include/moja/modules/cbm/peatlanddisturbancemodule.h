#ifndef MOJA_MODULES_CBM_PEATLAND_DISTURBANCE_H_
#define MOJA_MODULES_CBM_PEATLAND_DISTURBANCE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/peatlandfireparameters.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/*
			Response to the historical and last disturbance events in CBM spinup
			*/
			class CBM_API PeatlandDisturbanceModule : public CBMModuleBase {
			public:
				PeatlandDisturbanceModule() {};
				virtual ~PeatlandDisturbanceModule() {};

				std::string fireEvent = "fire";

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				void doDisturbanceEvent(DynamicVar) override;
				void doLocalDomainInit() override;
				void doTimingInit() override;

				void fetchPeatlandDistMatrices();
				void fetchPeatlandDMAssociations();

			private:
				typedef std::vector<CBMDistEventTransfer> EventVector;
				typedef std::unordered_map<int, EventVector> EventMap;

				EventMap _matrices;
				std::unordered_map<std::pair<int, std::string>, std::pair<int, int>> _dmAssociations;

				flint::IVariable* _wtdModifier;
				flint::IVariable* _wtdModifierYear;

				int _peatlandId{ -1 };
				bool _runPeatland{ false };
			};
		}
	}
}
#endif