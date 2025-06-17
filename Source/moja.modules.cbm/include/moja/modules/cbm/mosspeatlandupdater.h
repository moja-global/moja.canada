#ifndef MOJA_MODULES_CBM_MOSSPEATLANDUPDATER_H_
#define MOJA_MODULES_CBM_MOSSPEATLANDUPDATER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/turnoverrates.h"
#include "moja/modules/cbm/peatlands.h"
#include "moja/modules/cbm/helper.h"

#include <boost/algorithm/string.hpp> 
#include <fstream>

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API MossPeatlandUpdater : public CBMModuleBase {
			public:
				MossPeatlandUpdater() {};
				virtual ~MossPeatlandUpdater() {};

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;
				void doPreTimingSequence() override;
				void doPrePostDisturbanceEvent() override;

			private:
				const flint::IPool* _atmosphere;

				const flint::IPool* _aboveGroundVeryFastSoil;
				const flint::IPool* _belowGroundVeryFastSoil;
				const flint::IPool* _aboveGroundFastSoil;
				const flint::IPool* _belowGroundFastSoil;
				const flint::IPool* _aboveGroundSlowSoil;
				const flint::IPool* _belowGroundSlowSoil;

				const flint::IPool* _featherMossFast = nullptr;
				const flint::IPool* _sphagnumMossFast = nullptr;
				const flint::IPool* _featherMossSlow = nullptr;
				const flint::IPool* _sphagnumMossSlow = nullptr;

				const flint::IPool* _woodyFoliageDead = nullptr;
				const flint::IPool* _woodyFineDead = nullptr;
				const flint::IPool* _woodyCoarseDead = nullptr;
				const flint::IPool* _woodyRootsDead = nullptr;
				const flint::IPool* _sedgeFoliageDead{ nullptr };
				const flint::IPool* _sedgeRootsDead{ nullptr };

				const flint::IPool* _featherMossDead{ nullptr };
				const flint::IPool* _acrotelm_o{ nullptr };
				const flint::IPool* _catotelm_a{ nullptr };
				const flint::IPool* _acrotelm_a{ nullptr };
				const flint::IPool* _catotelm_o{ nullptr };

				flint::IVariable* _runPeatland = nullptr;
				flint::IVariable* _runMoss = nullptr;

				flint::IVariable* _age;
				flint::IVariable* _shrubAge;
				flint::IVariable* _mossAge;
				flint::IVariable* _smallTreeAge;

				flint::IVariable* _cset = nullptr;
				flint::IVariable* _mossCaMPIndictor = nullptr;
				flint::IVariable* _runtimePeatlandId = nullptr;
				flint::IVariable* _loadMossInitial = nullptr;

				// flag to indicate this instance is in spin-up phase or forward
				bool _inSpinup{ false };

				// Check if to run peatland module
				bool isPeatlandApplicable(int peatlandId, Int64 standForestGrowthCurveId);

				// Check if to run moss module
				bool isMossApplicable(bool runPeatland, Int64 standForestGrowthCurveId);

				double getPeatMoss();

				double getPeatCaMP();

				void resetMossInitialValue();
				void loadMossInitialValue();

				void transferMoss2CaMP();

				void transferCaMP2Moss();
			};
		}
	}
}
#endif