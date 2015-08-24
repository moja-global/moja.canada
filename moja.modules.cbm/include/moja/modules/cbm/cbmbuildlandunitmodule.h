#ifndef CBMBuildLandUnitModule_H_
#define CBMBuildLandUnitModule_H_

#include "moja/flint/modulebase.h"

namespace moja {
	namespace modules {
		namespace CBM {

			class CBMBuildLandUnitModule : public flint::ModuleBase {
			public:
				CBMBuildLandUnitModule() : ModuleBase() {}
				virtual ~CBMBuildLandUnitModule() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
				void onPreTimingSequence(const flint::PreTimingSequenceNotification::Ptr& n) override;

			private:
				flint::IVariable* _buildWorked;
                flint::IVariable* _initialAge;
			};

		}
	}
} // namespace moja::Modules::CBM
#endif // CBMBuildLandUnitModule_H_