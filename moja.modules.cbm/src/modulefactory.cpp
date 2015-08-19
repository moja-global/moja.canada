#include "moja/modules/cbm/modulefactory.h"
#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/modules/cbm/yieldtablegrowthmodule.h"
#include "moja/modules/cbm/cbmturnovermodule.h"
#include "moja/modules/cbm/cbmsequencer.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/cbmaggregatorfluxsqlite.h"
#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"
#include "moja/modules/cbm/outputerstreampostnotify.h"
#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/modules/cbm/cbmspinupdisturbancemodule.h"

#include "moja/modules/cbm/cbmlandunitdatatransform.h"

using moja::flint::IModule;
using moja::flint::ITransform;
using moja::flint::ModuleRegistration;
using moja::flint::TransformRegistration;

namespace moja {
	namespace modules {

		extern "C" {

			MOJA_LIB_API moja::flint::IModule* CreateCBMAggregatorFluxSQLite		() { return new moja::modules::CBM::CBMAggregatorFluxSQLite		(); }
			MOJA_LIB_API moja::flint::IModule* CreateCBMDecayModule					() { return new moja::modules::CBM::CBMDecayModule				();	}
			MOJA_LIB_API moja::flint::IModule* CreateCBMDisturbanceEventModule		() { return new moja::modules::CBM::CBMDisturbanceEventModule	(); }
			MOJA_LIB_API moja::flint::IModule* CreateCBMGrowthModule				() { return new moja::modules::CBM::YieldTableGrowthModule		();	}
			MOJA_LIB_API moja::flint::IModule* CreateCBMTurnoverModule				() { return new moja::modules::CBM::CBMTurnoverModule			();	}
			MOJA_LIB_API moja::flint::IModule* CreateCBMSequencer					() { return new moja::modules::CBM::CBMSequencer				();	}
			MOJA_LIB_API moja::flint::IModule* CreateOutputerStreamPostNotify		() { return new moja::modules::CBM::OutputerStreamPostNotify	(); }
			MOJA_LIB_API moja::flint::IModule* CreateOutputerStreamFluxPostNotify	() { return new moja::modules::CBM::OutputerStreamFluxPostNotify(); }
			MOJA_LIB_API moja::flint::IModule* CreateCBMSpinupSequencer				() { return new moja::modules::CBM::CBMSpinupSequencer			();	}
			MOJA_LIB_API moja::flint::IModule* CreateCBMBuildLandUnitModule			() { return new moja::modules::CBM::CBMBuildLandUnitModule		(); }
			MOJA_LIB_API moja::flint::IModule* CreateCBMSpinupDisturbanceModule		() { return new moja::modules::CBM::CBMSpinupDisturbanceModule  (); }

			MOJA_LIB_API ITransform* CreateCBMLandUnitDataTransform					() { return new moja::modules::CBM::CBMLandUnitDataTransform	(); }

			MOJA_LIB_API int getModuleRegistrations(moja::flint::ModuleRegistration* outModuleRegistrations) {
				int index = 0;
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMAggregatorFluxSQLite",		&CreateCBMAggregatorFluxSQLite };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMDecayModule",				&CreateCBMDecayModule };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMDisturbanceEventModule",	&CreateCBMDisturbanceEventModule };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMGrowthModule",				&CreateCBMGrowthModule };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMTurnoverModule",			&CreateCBMTurnoverModule };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMSequencer",				&CreateCBMSequencer };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "OutputerStreamPostNotify",	&CreateOutputerStreamPostNotify };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "OutputerStreamFluxPostNotify",&CreateOutputerStreamFluxPostNotify };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMSpinupSequencer",			&CreateCBMSpinupSequencer };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMBuildLandUnitModule",		&CreateCBMBuildLandUnitModule };
				outModuleRegistrations[index++] = moja::flint::ModuleRegistration{ "CBMSpinupDisturbanceModule",  &CreateCBMSpinupDisturbanceModule };
				return index;
			}

			MOJA_LIB_API int getTransformRegistrations(moja::flint::TransformRegistration* outTransformRegistrations) {
				int index = 0;
				outTransformRegistrations[index++] = TransformRegistration{ "CBMLandUnitDataTransform", &CreateCBMLandUnitDataTransform };
				return index;
			}
		}
	}
}