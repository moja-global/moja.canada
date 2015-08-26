#ifndef MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_
#define MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_

#include "moja/flint/modulebase.h"

namespace moja {
	namespace modules {
		namespace cbm {

			class CBMBuildLandUnitModule : public flint::ModuleBase {
			public:
				CBMBuildLandUnitModule() : ModuleBase() {}
				virtual ~CBMBuildLandUnitModule() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
				void onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n) override;

			private:
				const flint::IVariable* lossyear;
				flint::IVariable* buildWorked;
			};

		}
	}
} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMBUILDLANDUNITMODULE_H_