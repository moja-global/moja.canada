#ifndef MOJA_MODULES_CBM_MOSS_DISTURBANCE_H_
#define MOJA_MODULES_CBM_MOSS_DISTURBANCE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/*
			Moss module to response to the fire disturbance events in CBM
			*/
			class CBM_API MossDisturbanceModule : public CBMModuleBase {
			public:
				MossDisturbanceModule() {
					_runMoss = false;
				};

				virtual ~MossDisturbanceModule() {};

				const std::string fireEvent = "fire";

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				void doDisturbanceEvent(DynamicVar) override;
				void doLocalDomainInit() override;
				void doTimingInit() override;

			private:
				bool _runMoss;

				typedef std::vector<CBMDistEventTransfer> EventVector;
				typedef std::unordered_map<int, EventVector> EventMap;

				EventMap _matrices;
				std::unordered_map<std::string, int> _dmAssociations;

				void fetchMossDistMatrices();
				void fetchMossDMAssociations();
			};
		}
	}
}
#endif