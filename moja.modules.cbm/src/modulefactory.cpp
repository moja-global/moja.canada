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
#include "moja/flint/recordaccumulatortbb.h"
#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"
#include "moja/modules/cbm/mossgrowthmodule.h"
#include "moja/modules/cbm/mossturnovermodule.h"
#include "moja/modules/cbm/mossdecaymodule.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/peatlanddisturbancemodule.h"
#include "moja/modules/cbm/mossdisturbancemodule.h"
#include "moja/modules/cbm/peatlandpreparemodule.h"
#include "moja/modules/cbm/peatlandgrowthmodule.h"
#include "moja/modules/cbm/peatlandturnovermodule.h"
#include "moja/modules/cbm/peatlanddecaymodule.h"
#include "moja/modules/cbm/cbmtransitionrulesmodule.h"

#include <unordered_set>
#include <atomic>

namespace moja {
namespace modules {

    struct CBMObjectHolder {
        CBMObjectHolder() {
            dateDimension			= std::make_shared<flint::RecordAccumulatorTBB<cbm::DateRow>>();
            poolInfoDimension		= std::make_shared<flint::RecordAccumulatorTBB<cbm::PoolInfoRow>>();
            classifierSetDimension	= std::make_shared<flint::RecordAccumulatorTBB<cbm::ClassifierSetRow>>();
            classifierNames         = std::make_shared<std::unordered_set<std::string>>();
            locationDimension		= std::make_shared<flint::RecordAccumulatorTBB<cbm::LocationRow>>();
			poolDimension			= std::make_shared<flint::RecordAccumulatorTBB<cbm::PoolRow>>();
			fluxDimension			= std::make_shared<flint::RecordAccumulatorTBB<cbm::FluxRow>>();
			moduleInfoDimension		= std::make_shared<flint::RecordAccumulatorTBB<cbm::ModuleInfoRow>>();
			gcFactory				= std::make_shared<cbm::StandGrowthCurveFactory>();
            fluxAggregatorId        = 1;
        }

        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::DateRow>> dateDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::PoolInfoRow>> poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::ClassifierSetRow>> classifierSetDimension;
        std::shared_ptr<std::unordered_set<std::string>> classifierNames;
        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::LocationRow>> locationDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::PoolRow>> poolDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::FluxRow>> fluxDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<cbm::ModuleInfoRow>> moduleInfoDimension;
		std::shared_ptr<cbm::StandGrowthCurveFactory> gcFactory;
        std::atomic<int> fluxAggregatorId;
    };

    static CBMObjectHolder cbmObjectHolder;

    extern "C" {
        MOJA_LIB_API flint::IModule* CreateCBMAggregatorPoolSQLite() {
            return new cbm::CBMAggregatorPoolSQLite(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.locationDimension,
                cbmObjectHolder.poolDimension);
        }

        MOJA_LIB_API flint::IModule* CreateCBMAggregatorFluxSQLite() {
            bool isPrimaryAggregator = cbmObjectHolder.fluxAggregatorId++ == 1;
            return new cbm::CBMAggregatorFluxSQLite(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.locationDimension,
                cbmObjectHolder.fluxDimension,
				cbmObjectHolder.moduleInfoDimension,
                cbmObjectHolder.classifierNames,
                isPrimaryAggregator);
		}
		
        MOJA_LIB_API flint::IModule* CreateCBMGrowthModule () {
			return new cbm::YieldTableGrowthModule(cbmObjectHolder.gcFactory);
		}

		MOJA_LIB_API flint::IModule* CreateCBMMossGrowthModule() {
			return new cbm::MossGrowthModule(cbmObjectHolder.gcFactory);
		}

		MOJA_LIB_API flint::IModule* CreateCBMMossDecayModule() { 
			return new cbm::MossDecayModule(cbmObjectHolder.gcFactory);
		}

        MOJA_LIB_API flint::IModule* CreateCBMDecayModule					() { return new cbm::CBMDecayModule				 (); }
        MOJA_LIB_API flint::IModule* CreateCBMDisturbanceEventModule		() { return new cbm::CBMDisturbanceEventModule	 (); }
        MOJA_LIB_API flint::IModule* CreateCBMSequencer						() { return new cbm::CBMSequencer				 (); }
        MOJA_LIB_API flint::IModule* CreateOutputerStreamPostNotify			() { return new cbm::OutputerStreamPostNotify	 (); }
        MOJA_LIB_API flint::IModule* CreateOutputerStreamFluxPostNotify		() { return new cbm::OutputerStreamFluxPostNotify(); }
        MOJA_LIB_API flint::IModule* CreateCBMSpinupSequencer				() { return new cbm::CBMSpinupSequencer			 (); }
        MOJA_LIB_API flint::IModule* CreateCBMBuildLandUnitModule			() { return new cbm::CBMBuildLandUnitModule		 (); }
        MOJA_LIB_API flint::IModule* CreateCBMSpinupDisturbanceModule		() { return new cbm::CBMSpinupDisturbanceModule  (); }
        MOJA_LIB_API flint::IModule* CreateCBMLandClassTransitionModule     () { return new cbm::CBMLandClassTransitionModule(); }
        MOJA_LIB_API flint::ITransform* CreateCBMLandUnitDataTransform		() { return new cbm::CBMLandUnitDataTransform	 (); }
        MOJA_LIB_API flint::ITransform* CreateGrowthCurveTransform			() { return new cbm::GrowthCurveTransform		 (); }
		MOJA_LIB_API flint::IModule* CreateCBMMossTurnoverModule			() { return new cbm::MossTurnoverModule			 (); }	
		MOJA_LIB_API flint::IModule* CreatePeatlandDisturbanceModule		() { return new cbm::PeatlandDisturbanceModule   (); }
		MOJA_LIB_API flint::IModule* CreateMossDisturbanceModule			() { return new cbm::MossDisturbanceModule		 (); }
		MOJA_LIB_API flint::IModule* CreatePeatlandPrepareModule			() { return new cbm::PeatlandPrepareModule		 (); }
		MOJA_LIB_API flint::IModule* CreatePeatlandGrowthModule				() { return new cbm::PeatlandGrowthModule		 (); }
		MOJA_LIB_API flint::IModule* CreatePeatlandTurnoverModule			() { return new cbm::PeatlandTurnoverModule		 (); }
		MOJA_LIB_API flint::IModule* CreatePeatlandDecayModule				() { return new cbm::PeatlandDecayModule		 (); }
        MOJA_LIB_API flint::IModule* CreateTransitionRulesModule            () { return new cbm::CBMTransitionRulesModule    (); }

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
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossTurnoverModule",		 &CreateCBMMossTurnoverModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossDecayModule",			 &CreateCBMMossDecayModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossGrowthModule",			 &CreateCBMMossGrowthModule };	
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlanDisturbanceModule",     &CreatePeatlandDisturbanceModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "MossDisturbanceModule",		 &CreateMossDisturbanceModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandPrepareModule",		 &CreatePeatlandPrepareModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandGrowthModule",		 &CreatePeatlandGrowthModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandTurnoverModule",		 &CreatePeatlandTurnoverModule };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandDecayModule",			 &CreatePeatlandDecayModule };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMTransitionRulesModule",     &CreateTransitionRulesModule };

            return index;
        }

        MOJA_LIB_API int getTransformRegistrations(flint::TransformRegistration* outTransformRegistrations) {
            int index = 0;
            outTransformRegistrations[index++] = flint::TransformRegistration{ "CBMLandUnitDataTransform", &CreateCBMLandUnitDataTransform };
            outTransformRegistrations[index++] = flint::TransformRegistration{ "GrowthCurveTransform",     &CreateGrowthCurveTransform };
            return index;
        }

		MOJA_LIB_API int getFlintDataRegistrations(moja::flint::FlintDataRegistration* outFlintDataRegistrations) {
			auto index = 0;
			return index;
		}
	}

}}