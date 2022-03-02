#ifndef MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_
#define MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_

#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/cbmdisturbancelistener.h"
#include "moja/hash.h"

#include <unordered_map>
#include <unordered_set>

namespace moja {
	namespace modules {
		namespace cbm {

			class CBMDisturbanceEventModule : public CBMModuleBase {
			public:
				CBMDisturbanceEventModule() : CBMModuleBase() {}
				virtual ~CBMDisturbanceEventModule() = default;

				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

				virtual void doDisturbanceEvent(DynamicVar) override;
				virtual void doLocalDomainInit() override;

			private:
				flint::IVariable* _age;

				const flint::IPool* _softwoodMerch;
				const flint::IPool* _softwoodOther;
				const flint::IPool* _softwoodFoliage;
				const flint::IPool* _softwoodCoarseRoots;
				const flint::IPool* _softwoodFineRoots;

				const flint::IPool* _hardwoodMerch;
				const flint::IPool* _hardwoodOther;
				const flint::IPool* _hardwoodFoliage;
				const flint::IPool* _hardwoodCoarseRoots;
				const flint::IPool* _hardwoodFineRoots;

				const flint::IPool* _woodyFoliageLive;
				const flint::IPool* _woodyStemsBranchesLive;
				const flint::IPool* _woodyRootsLive;

				const flint::IPool* _softwoodStem;
				const flint::IPool* _hardwoodStem;

				flint::IVariable* _smalltreeAge;
				flint::IVariable* _shrubAge;
			};

		}
	}
} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMDISTURBANCEEVENTMODULE_H_
