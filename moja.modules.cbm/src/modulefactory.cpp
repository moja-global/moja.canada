#include "moja/modules/cbm/modulefactory.h"
#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/modules/cbm/yieldtablegrowthmodule.h"
#include "moja/modules/cbm/cbmsequencer.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/cbmaggregatorfluxsqlite.h"
#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"
#include "moja/modules/cbm/outputerstreampostnotify.h"
#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/modules/cbm/cbmspinupdisturbancemodule.h"
#include "moja/modules/cbm/cbmaggregatorpoolsqlite.h"
#include "moja/modules/cbm/cbmlandunitdatatransform.h"
#include "moja/modules/cbm/growthcurvetransform.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/recordaccumulator.h"
#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"

namespace moja {
namespace modules {

    struct CBMObjectHolder {
        CBMObjectHolder() {
            dateDimension = std::make_shared<flint::RecordAccumulator<cbm::DateRow>>();
            poolInfoDimension = std::make_shared<flint::RecordAccumulator<cbm::PoolInfoRow>>();
            classifierSetDimension = std::make_shared<flint::RecordAccumulator<cbm::ClassifierSetRow>>();
            locationDimension = std::make_shared<flint::RecordAccumulator<cbm::LocationRow>>();
        }

        std::shared_ptr<flint::RecordAccumulator<cbm::DateRow>> dateDimension;
        std::shared_ptr<flint::RecordAccumulator<cbm::PoolInfoRow>> poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulator<cbm::ClassifierSetRow>> classifierSetDimension;
        std::shared_ptr<flint::RecordAccumulator<cbm::LocationRow>> locationDimension;
    };

    CBMObjectHolder cbmObjectHolder;

    extern "C" {

        MOJA_LIB_API flint::IModule* CreateCBMAggregatorPoolSQLite() {
            return new cbm::CBMAggregatorPoolSQLite(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.locationDimension);
        }

        MOJA_LIB_API flint::IModule* CreateCBMAggregatorFluxSQLite() {
            return new cbm::CBMAggregatorFluxSQLite(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.locationDimension);
        }

        MOJA_LIB_API flint::IModule* CreateCBMDecayModule					() { return new cbm::CBMDecayModule				 (); }
        MOJA_LIB_API flint::IModule* CreateCBMDisturbanceEventModule		() { return new cbm::CBMDisturbanceEventModule	 (); }
        MOJA_LIB_API flint::IModule* CreateCBMGrowthModule					() { return new cbm::YieldTableGrowthModule		 (); }
        MOJA_LIB_API flint::IModule* CreateCBMSequencer						() { return new cbm::CBMSequencer				 (); }
        MOJA_LIB_API flint::IModule* CreateOutputerStreamPostNotify			() { return new cbm::OutputerStreamPostNotify	 (); }
        MOJA_LIB_API flint::IModule* CreateOutputerStreamFluxPostNotify		() { return new cbm::OutputerStreamFluxPostNotify(); }
        MOJA_LIB_API flint::IModule* CreateCBMSpinupSequencer				() { return new cbm::CBMSpinupSequencer			 (); }
        MOJA_LIB_API flint::IModule* CreateCBMBuildLandUnitModule			() { return new cbm::CBMBuildLandUnitModule		 (); }
        MOJA_LIB_API flint::IModule* CreateCBMSpinupDisturbanceModule		() { return new cbm::CBMSpinupDisturbanceModule  (); }
        MOJA_LIB_API flint::IModule* CreateCBMLandClassTransitionModule     () { return new cbm::CBMLandClassTransitionModule(); }

        MOJA_LIB_API flint::ITransform* CreateCBMLandUnitDataTransform		() { return new cbm::CBMLandUnitDataTransform	 (); }
        MOJA_LIB_API flint::ITransform* CreateGrowthCurveTransform			() { return new cbm::GrowthCurveTransform		 (); }

        MOJA_LIB_API int getModuleRegistrations(moja::flint::ModuleRegistration* outModuleRegistrations) {
            int index = 0;
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorPoolSQLite",		 &CreateCBMAggregatorPoolSQLite };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorFluxSQLite",      &CreateCBMAggregatorFluxSQLite };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDecayModule",               &CreateCBMDecayModule };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDisturbanceEventModule",	 &CreateCBMDisturbanceEventModule };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMGrowthModule",              &CreateCBMGrowthModule };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSequencer",				 &CreateCBMSequencer };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "OutputerStreamPostNotify",	 &CreateOutputerStreamPostNotify };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "OutputerStreamFluxPostNotify", &CreateOutputerStreamFluxPostNotify };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSpinupSequencer",			 &CreateCBMSpinupSequencer };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMBuildLandUnitModule",		 &CreateCBMBuildLandUnitModule };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSpinupDisturbanceModule",   &CreateCBMSpinupDisturbanceModule };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMLandClassTransitionModule", &CreateCBMLandClassTransitionModule };
            return index;
        }

        MOJA_LIB_API int getTransformRegistrations(flint::TransformRegistration* outTransformRegistrations) {
            int index = 0;
            outTransformRegistrations[index++] = flint::TransformRegistration{ "CBMLandUnitDataTransform", &CreateCBMLandUnitDataTransform };
            outTransformRegistrations[index++] = flint::TransformRegistration{ "GrowthCurveTransform",     &CreateGrowthCurveTransform };
            return index;
        }

    }

}}