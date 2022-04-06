#ifndef MOJA_MODULES_CBM_MOSSTURNOVER_H_
#define MOJA_MODULES_CBM_MOSSTURNOVER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/// <summary>
			/// Parameters for moss related computing.
			/// </summary>	
			class CBM_API MossTurnoverModule : public CBMModuleBase {
			public:
				MossTurnoverModule();
				virtual ~MossTurnoverModule() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;

			private:
				flint::IVariable* _mossParameters = nullptr;;

				const flint::IPool* _featherMossLive = nullptr;;
				const flint::IPool* _sphagnumMossLive = nullptr;;
				const flint::IPool* _featherMossFast = nullptr;;
				const flint::IPool* _sphagnumMossFast = nullptr;;

				flint::IVariable* _regenDelay = nullptr;
				bool runMoss{ false };
				double fmlTurnoverRate{ 0 }; //Feather moss turnover rate                   
				double smlTurnoverRate{ 0 }; //Sphagnum moss turnover rate    

				void doLiveMossTurnover();
			};

		}
	}
}
#endif
